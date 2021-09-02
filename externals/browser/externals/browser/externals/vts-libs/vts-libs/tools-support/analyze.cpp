/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "../vts/meshop.hpp"

#include "analyze.hpp"

namespace vtslibs { namespace vts { namespace tools {

void computeNavtileInfo(const vts::NodeInfo &node
                        , const LodParams lodParams
                        , const LodInfo &lodInfo
                        , vts::NtGenerator &ntg
                        , const boost::optional<vts::LodTileRange> &tileExtents
                        , double ntLodPixelSize)
{
    // find nt lod by nt lod pixelsize
    const auto nodeId(node.nodeId());

    // build LOD range
    vts::LodRange lr(0, nodeId.lod + lodParams.lod);

    auto lodDiff(lodInfo.levelDiff());
    if (lodDiff > lodParams.lod) {
        lr.min = nodeId.lod;
    } else {
        lr.min = nodeId.lod + lodParams.lod - lodDiff;
    }

    // fix limit for tile extents
    if (tileExtents && (tileExtents->lod >= node.rootLod())) {
        lr.min = tileExtents->lod;
    }

    // nt lod, start with maximum lod
    vts::Lod ntLod(lr.max);

    // sample one tile at bottom lod
    const vts::NodeInfo n(node.child
                          (vts::lowestChild(nodeId, lodParams.lod)));

    // tile size at bottom lod
    const auto ts(math::size(n.extents()));

    // take center of extents
    const auto ntCenter(math::center(lodParams.meshExtents));

    // navtile size (in pixels)
    auto ntSize(vts::NavTile::size());
    ntSize.width -= 1;
    ntSize.height -= 1;

    // SRS factors at mesh center
    const auto f(geo::SrsFactors(node.srsDef())(ntCenter));

    // calculate pixel size from ration between tile area (in "beans") and
    // navtile size in pixels; ration is down scaled by area of srs factors
    // scales
    auto pixelSize(std::sqrt
                   (math::area(ts)
                    / (math::area(ntSize)
                       * f.meridionalScale * f.parallelScale)));

    // find best matching lod
    // FIXME: probably needs to be fixed
    while ((ntLod > lr.min) && (pixelSize < ntLodPixelSize)) {
        pixelSize *= 2.0;
        --ntLod;
    }

    ntg.addAccumulator(node.srs(), vts::LodRange(lr.min, ntLod), pixelSize);
}

vts::TileRange computeTileRange(const math::Extents2 &nodeExtents
                                , vts::Lod localLod
                                , const math::Extents2 &meshExtents)
{
    vts::TileRange r(math::InvalidExtents{});
    const auto ts(vts::tileSize(nodeExtents, localLod));
    const auto origin(math::ul(nodeExtents));

    for (const auto &p : vertices(meshExtents)) {
        update(r, vts::TileRange::point_type
               ((p(0) - origin(0)) / ts.width
                , (origin(1) - p(1)) / ts.height));
    }

    return r;
}

MeshInfo measureMesh(const vts::NodeInfo &rfNode
                     , const vts::CsConvertor conv
                     , const vts::Mesh &mesh
                     , const std::vector<math::Size2> &sizes)
{
    MeshInfo mi;

    auto isizes(sizes.begin());
    for (const auto &sm : mesh) {
        const auto &size(*isizes++);

        // make all faces valid by default
        vts::VertexMask valid(sm.vertices.size(), true);
        math::Points3 projected;
        projected.reserve(sm.vertices.size());

        auto ivalid(valid.begin());
        for (const auto &v : sm.vertices) {
            try {
                projected.push_back(conv(v));
                ++ivalid;
            } catch (const std::exception&) {
                // failed to convert vertex, mask it and skip
                projected.emplace_back();
                *ivalid++ = false;
            }
        }

        // clip mesh to node's extents
        // FIXME: implement mask application in clipping!
        auto osm(vts::clip(sm, projected, rfNode.extents(), valid));
        if (osm.faces.empty()) { continue; }

        // at least one face survived remember
        mi.update(osm, size);
    }

    return mi;
}

namespace {

void remapTcToRegion(vts::SubMesh &sm, const vts::FaceOriginList &faceOrigin
                     , const TextureRegionInfo &ri)
{
    if (ri.regions.empty()) {
        // nothing to inflate
        return;
    }

    // remap texture coordinates from region coordinates to texture coordinates

    std::vector<char> seen(sm.tc.size(), false);
    const auto &remap([&](int index, const TextureRegionInfo::Region &region)
                      -> void
    {
        auto &iseen(seen[index]);
        if (iseen) { return; }

        // remap from region space to texture space
        auto &tc(sm.tc[index]);
        tc(0) *= region.size.width;
        tc(1) *= region.size.height;

        iseen = true;
    });

    auto ifaceOrigin(faceOrigin.begin());
    for (const auto &face : sm.facesTc) {
        // grab index to regions
        const auto regionIndex(ri.faces[*ifaceOrigin++]);
        const auto &region(ri.regions[regionIndex]);

        for (const auto &tc : face) { remap(tc, region); }
    }
}

} // namespace

MeshInfo measureMesh(const vts::NodeInfo &rfNode
                     , const vts::CsConvertor conv
                     , const vts::Mesh &mesh
                     , const TextureRegionInfo::list &regions
                     , const std::vector<math::Size2> &sizes)
{
    MeshInfo mi;

    auto isizes(sizes.begin());
    auto iregions(regions.begin());
    for (const auto &sm : mesh) {
        const auto &size(*isizes++);
        const auto &ri(*iregions++);

        // make all faces valid by default
        vts::VertexMask valid(sm.vertices.size(), true);
        math::Points3 projected;
        projected.reserve(sm.vertices.size());

        auto ivalid(valid.begin());
        for (const auto &v : sm.vertices) {
            try {
                projected.push_back(conv(v));
                ++ivalid;
            } catch (const std::exception&) {
                // failed to convert vertex, mask it and skip
                projected.emplace_back();
                *ivalid++ = false;
            }
        }

        // clip mesh to node's extents
        // FIXME: implement mask application in clipping!
        vts::FaceOriginList faceOrigin;
        auto osm(vts::clip(sm, projected, rfNode.extents(), valid
                           , &faceOrigin));
        if (osm.faces.empty()) { continue; }

        remapTcToRegion(osm, faceOrigin, ri);

        // at least one face survived remember
        mi.update(osm, size);
    }

    return mi;
}

} } } // namespace vtslibs::vts::tools
