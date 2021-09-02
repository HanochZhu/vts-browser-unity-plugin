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

#include "../mapLayer.hpp"
#include "../map.hpp"
#include "../mapConfig.hpp"

#include <boost/algorithm/string.hpp>

namespace vts
{

void SurfaceStack::print()
{
    std::ostringstream ss;
    ss << "Surface stack: " << std::endl;
    for (auto &l : surfaces)
        ss << (l.alien ? "* " : "  ")
           << std::setw(100) << std::left << (std::string()
           + "[" + boost::algorithm::join(l.name, ",") + "]")
           << " " << l.color.transpose() << std::endl;
    LOG(info3) << ss.str();
}

void SurfaceStack::colorize()
{
    auto &ss = surfaces;
    for (auto it = ss.begin(),
         et = ss.end(); it != et; it++)
        it->color = convertHsvToRgb(vec3f((it - ss.begin())
                                          / (float)ss.size(), 1, 1));
}

void SurfaceStack::generateVirtual(MapImpl *map,
        const vtslibs::vts::VirtualSurfaceConfig *virtualSurface)
{
    assert(surfaces.empty());
    LOG(info2) << "Generating (virtual) surface stack for <"
               << boost::algorithm::join(virtualSurface->id, ",") << ">";
    surfaces.emplace_back(*virtualSurface, map->mapconfig->name);
}

void SurfaceStack::generateTileset(MapImpl *map,
        const std::vector<std::string> &vsId,
        const vtslibs::vts::TilesetReferencesList &dataRaw)
{
    auto *mapconfig = map->mapconfig.get();

    assert(surfaces.empty());
    surfaces.reserve(dataRaw.size() + 1);
    // the sourceReference in metanodes is one-based
    surfaces.push_back(SurfaceInfo());
    for (auto &it : dataRaw)
    {
        if (it.size() == 1)
        { // surface
            SurfaceInfo i(
                    *mapconfig->findSurface(vsId[it[0]]),
                    mapconfig->name);
            i.name.push_back(vsId[it[0]]);
            surfaces.push_back(i);
        }
        else
        { // glue
            std::vector<std::string> id;
            id.reserve(it.size());
            for (auto &it2 : it)
                id.push_back(vsId[it2]);
            SurfaceInfo i(
                    *mapconfig->findGlue(id),
                    mapconfig->name);
            i.name = id;
            surfaces.push_back(i);
        }
    }

    colorize();
}

void SurfaceStack::generateReal(MapImpl *map)
{
    LOG(info2) << "Generating (real) surface stack";

    // prepare initial surface stack
    vtslibs::vts::TileSetGlues::list lst;
    for (auto &s : map->mapconfig->view.surfaces)
    {
        vtslibs::vts::TileSetGlues ts(s.first);
        for (auto &g : map->mapconfig->glues)
        {
            bool active = g.id.back() == ts.tilesetId;
            for (auto &it : g.id)
                if (map->mapconfig->view.surfaces.find(it)
                        == map->mapconfig->view.surfaces.end())
                    active = false;
            if (active)
                ts.glues.push_back(vtslibs::vts::Glue(g.id));
        }
        lst.push_back(ts);
    }

    // sort surfaces by their order in mapconfig
    std::unordered_map<std::string, uint32> order;
    uint32 i = 0;
    for (auto &it : map->mapconfig->surfaces)
        order[it.id] = i++;
    std::sort(lst.begin(), lst.end(), [order](
              vtslibs::vts::TileSetGlues &a,
              vtslibs::vts::TileSetGlues &b) mutable {
        assert(order.find(a.tilesetId) != order.end());
        assert(order.find(b.tilesetId) != order.end());
        return order[a.tilesetId] < order[b.tilesetId];
    });

    // sort glues on surfaces
    lst = vtslibs::vts::glueOrder(lst);
    std::reverse(lst.begin(), lst.end());

    // generate proper surface stack
    assert(surfaces.empty());
    for (auto &ts : lst)
    {
        for (auto &g : ts.glues)
        {
            SurfaceInfo i(
                    *map->mapconfig->findGlue(g.id),
                    map->mapconfig->name);
            i.name = g.id;
            surfaces.push_back(i);
        }
        SurfaceInfo i(
                    *map->mapconfig->findSurface(ts.tilesetId),
                    map->mapconfig->name);
        i.name = { ts.tilesetId };
        surfaces.push_back(i);
    }

    // generate alien surface stack positions
    auto copy(surfaces);
    for (auto &it : copy)
    {
        if (it.name.size() > 1)
        {
            auto n2 = it.name;
            n2.pop_back();
            std::string n = boost::join(n2, "|");
            auto jt = surfaces.begin(), et = surfaces.end();
            while (jt != et && boost::join(jt->name, "|") != n)
                jt++;
            if (jt != et)
            {
                SurfaceInfo i(it);
                i.alien = true;
                surfaces.insert(jt, i);
            }
        }
    }

    colorize();
}

void SurfaceStack::generateFree(MapImpl *map,
        const FreeInfo &freeLayer)
{
    (void)map;
    switch (freeLayer.type)
    {
    case vtslibs::registry::FreeLayer::Type::external:
        LOGTHROW(fatal, std::logic_error)
                << "Trying to use external free layer directly";
        throw;
    case vtslibs::registry::FreeLayer::Type::meshTiles:
    {
        SurfaceInfo item(
            boost::get<vtslibs::registry::FreeLayer::MeshTiles>(
                        freeLayer.definition), freeLayer.url);
        surfaces.push_back(item);
    } break;
    case vtslibs::registry::FreeLayer::Type::geodataTiles:
    {
        SurfaceInfo item(
            boost::get<vtslibs::registry::FreeLayer::GeodataTiles>(
                        freeLayer.definition), freeLayer.url);
        surfaces.push_back(item);
    } break;
    case vtslibs::registry::FreeLayer::Type::geodata:
    {
        SurfaceInfo item(
            boost::get<vtslibs::registry::FreeLayer::Geodata>(
                        freeLayer.definition), freeLayer.url);
        surfaces.push_back(item);
    } break;
    default:
        LOGTHROW(fatal, std::logic_error)
                << "Not implemented free layer type";
        throw;
    }
}

SurfaceInfo::SurfaceInfo(const vtslibs::vts::SurfaceCommonConfig &surface,
                         const std::string &parentPath)
{
    urlMeta.parse(convertPath(surface.urls3d->meta, parentPath));
    urlMesh.parse(convertPath(surface.urls3d->mesh, parentPath));
    urlIntTex.parse(convertPath(surface.urls3d->texture, parentPath));
}

SurfaceInfo::SurfaceInfo(
        const vtslibs::registry::FreeLayer::MeshTiles &surface,
        const std::string &parentPath)
    : color(0,0,0), alien(false)
{
    urlMeta.parse(convertPath(surface.metaUrl, parentPath));
    urlMesh.parse(convertPath(surface.meshUrl, parentPath));
    urlIntTex.parse(convertPath(surface.textureUrl, parentPath));
}

SurfaceInfo::SurfaceInfo(
        const vtslibs::registry::FreeLayer::GeodataTiles &surface,
        const std::string &parentPath)
    : color(0,0,0), alien(false)
{
    urlMeta.parse(convertPath(surface.metaUrl, parentPath));
    urlGeodata.parse(convertPath(surface.geodataUrl, parentPath));
}

SurfaceInfo::SurfaceInfo(
        const vtslibs::registry::FreeLayer::Geodata &surface,
        const std::string &parentPath)
    : color(0,0,0), alien(false)
{
    urlGeodata.parse(convertPath(surface.geodata, parentPath));
}

} // namespace vts
