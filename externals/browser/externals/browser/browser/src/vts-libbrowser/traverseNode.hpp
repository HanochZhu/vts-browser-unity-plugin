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

#ifndef TRAVERSENODE_HPP_sgh44f
#define TRAVERSENODE_HPP_sgh44f

#include "utilities/array.hpp"
#include "renderTasks.hpp"
#include "metaTile.hpp"

#include <boost/container/small_vector.hpp>

namespace vts
{

class MapLayer;
class SurfaceInfo;
class Resource;
class RenderSurfaceTask;
class RenderColliderTask;
class MeshAggregate;
class GeodataTile;

struct TraverseChildsContainer
{
    std::unique_ptr<struct TraverseChildsArray> ptr;

    TraverseNode *begin();
    TraverseNode *end();
    bool empty() const;
    uint32 size() const;
};

class TraverseNode : private Immovable
{
public:
    // traversal
    TraverseChildsContainer childs;
    MapLayer *const layer = nullptr;
    TraverseNode *const parent = nullptr;
    const TileId id;
    const uint32 hash = 0;

    // metadata
    boost::container::small_vector<vtslibs::registry::CreditId, 8> credits;
    boost::container::small_vector<std::shared_ptr<MetaTile>, 1> metaTiles;
    std::shared_ptr<const MetaNode> meta;
    const SurfaceInfo *surface = nullptr;

    uint32 lastAccessTime = 0;
    uint32 lastRenderTime = 0;
    float priority = nan1();

    // renders
    bool determined = false; // draws are fully loaded (draws may be empty)
    std::shared_ptr<MeshAggregate> meshAgg;
    std::shared_ptr<GeodataTile> geodataAgg;
    boost::container::small_vector<RenderSurfaceTask, 1> opaque;
    boost::container::small_vector<RenderSurfaceTask, 1> transparent;
    boost::container::small_vector<RenderColliderTask, 1> colliders;

    TraverseNode();
    TraverseNode(MapLayer *layer, TraverseNode *parent, const TileId &id);
    ~TraverseNode();
    void clearAll();
    void clearRenders();
    bool rendersReady() const;
    bool rendersEmpty() const;
};

struct TraverseChildsArray
{
    Array<TraverseNode, 4> arr;
};

inline TraverseNode *TraverseChildsContainer::begin()
{
    if (ptr)
        return &ptr->arr[0];
    return nullptr;
}

inline TraverseNode *TraverseChildsContainer::end()
{
    if (ptr)
        return &ptr->arr[0] + ptr->arr.size();
    return nullptr;
}

inline bool TraverseChildsContainer::empty() const
{
    if (ptr)
        return ptr->arr.empty();
    return true;
}

inline uint32 TraverseChildsContainer::size() const
{
    if (ptr)
        return ptr->arr.size();
    return 0;
}

TraverseNode *findTravById(TraverseNode *trav, const TileId &what);

} // namespace vts

#endif
