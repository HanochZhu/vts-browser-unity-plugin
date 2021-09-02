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
/**
 * @file rastermask/quadtree.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Raster mask (quadtree): inline functions.
 */

#ifndef imgproc_rastermask_inline_quadtree_hpp_included_
#define imgproc_rastermask_inline_quadtree_hpp_included_

#include <boost/logic/tribool.hpp>

namespace imgproc { namespace quadtree {

template <typename Op>
inline void RasterMask::forEachQuad(const Op &op, Filter filter) const
{
    root_.descend(0, 0, quadSize_, op, filter);
}

template <typename Op>
inline void RasterMask::forEach(const Op &op, Filter filter) const
{
    this->forEachQuad([&](std::size_t x, std::size_t y,
                      std::size_t xsize, std::size_t ysize, bool white)
    {
        // rasterize quad
        std::size_t ex(x + xsize);
        std::size_t ey(y + ysize);

        for (std::size_t j(y); j < ey; ++j) {
            for (std::size_t i(x); i < ex; ++i) {
                op(i, j, white);
            }
        }
    }, filter);
}

template <typename Op>
inline void RasterMask::Node::descend(unsigned int x, unsigned int y, unsigned int size
                                      , const Op &op, Filter filter)
    const
{
    switch (type) {
    case GRAY: {
        // descend down
        unsigned int split = size / 2;
        children->ul.descend(x, y, split, op, filter);
        children->ll.descend(x, y + split, split, op, filter);
        children->ur.descend(x + split, y, split, op, filter);
        children->lr.descend(x + split, y + split, split, op, filter);
        return;
    }

    case BLACK:
        // filter out white quads
        if (filter == Filter::white) { return; }
        break;

    case WHITE:
        // filter out black quads
        if (filter == Filter::black) { return; }
        break;
    }

    // call operation for black/white node
    op(x, y, ((x + size) > mask.sizeX_) ? (mask.sizeX_ - x) : size
       , ((y + size) > mask.sizeY_) ? (mask.sizeY_ - y) : size
       , (type == WHITE));
}

template <typename Op>
inline void RasterMask::forEachQuad(unsigned int depth, const Op &op) const
{
    root_.descend(depth, 0, 0, quadSize_, op);
}

template <typename Op>
inline void RasterMask::Node::descend(unsigned int depth, unsigned int x
                                      , unsigned int y, unsigned int size
                                      , const Op &op)
    const
{
    boost::tribool value(boost::indeterminate);
    switch (type) {
    case NodeType::GRAY:
        if (depth) {
            // descend down
            unsigned int split = size / 2;
            children->ul.descend(depth - 1, x, y, split, op);
            children->ll.descend(depth - 1, x, y + split, split, op);
            children->ur.descend(depth - 1, x + split, y, split, op);
            children->lr.descend(depth - 1, x + split, y + split, split, op);
            return;
        }
        break;

    default:
        value = (type == NodeType::WHITE);
    }

    // call operation for black/white node
    op(x, y, ((x + size) > mask.sizeX_) ? (mask.sizeX_ - x) : size
       , ((y + size) > mask.sizeY_) ? (mask.sizeY_ - y) : size
       , value);
}

} } // namespace imgproc::quadtree

#endif // imgproc_rastermask_inline_quadtree_hpp_included_
