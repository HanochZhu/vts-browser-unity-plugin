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

#include "math/boost_gil_all.hpp"

#include "dbglog/dbglog.hpp"

#include "qtree.hpp"

namespace vtslibs { namespace vts {

template <typename View, typename Covert>
void rasterize(const QTree &tree, View &view, const Covert &convert);

template <typename View>
void rasterize(const QTree &tree, View &view);

template <typename View, typename Covert>
void rasterize(const QTree &tree
               , unsigned int depth, unsigned int x, unsigned int y
               , View &view, const Covert &convert);

template <typename View, typename Covert>
void rasterize(const QTree &tree
               , unsigned int depth, unsigned int x, unsigned int y
               , View &view);

// inlines

template <typename View, typename Covert>
inline void rasterize(const QTree &tree, View &view, const Covert &convert)
{
    if (tree.size() != math::Size2(view.width(), view.height())) {
        LOGTHROW(err1, std::runtime_error)
            << "Tree and view have incompatible sizes.";
    }

    tree.forEachNode([&](unsigned int x, unsigned int y, unsigned int size
                         , QTree::value_type value)
    {
        boost::gil::fill_pixels
            (boost::gil::subimage_view(view, x, y, size, size)
             , convert(value));
    }, QTree::Filter::white);
}

template <typename View>
inline void rasterize(const QTree &tree, View &view)
{
    return rasterize(tree, view
                     , [](QTree::value_type value)
                     {
                         return value;
                     });
}

template <typename View, typename Covert>
inline void rasterize(const QTree &tree
               , unsigned int depth, unsigned int x, unsigned int y
               , View &view, const Covert &convert)
{
    // if (tree.size() != math::Size2(view.width(), view.height())) {
    //     LOGTHROW(err1, std::runtime_error)
    //         << "Tree and view have incompatible sizes.";
    // }

    tree.forEachNode(depth, x, y
                     , [&](unsigned int x, unsigned int y, unsigned int size
                           , QTree::value_type value)
    {
        boost::gil::fill_pixels
            (boost::gil::subimage_view(view, x, y, size, size)
             , convert(value));
    }, QTree::Filter::white);
}

template <typename View, typename Covert>
inline void rasterize(const QTree &tree
                      , unsigned int depth, unsigned int x, unsigned int y
                      , View &view)
{
    return rasterize(tree, depth, x, y, view
                     , [](QTree::value_type value)
                     {
                         return value;
                     });
}

} } // namespace vtslibs::vts
