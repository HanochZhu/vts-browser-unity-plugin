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
 * Temporary tileset. Used during encoding.
 * \file tmptileset.hpp
 * \author Vaclav Blazek <vaclav.blazek@melown.com>
 */

#ifndef vts_tools_tmptileset_hpp_included
#define vts_tools_tmptileset_hpp_included

#include <mutex>

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>

#include "../vts/types.hpp"
#include "../vts/mesh.hpp"
#include "../vts/opencv/atlas.hpp"
#include "../vts/tileindex.hpp"

namespace vtslibs { namespace vts { namespace tools {

class TmpTileset : boost::noncopyable {
public:
    TmpTileset(const boost::filesystem::path &root
               , bool create = true);
    ~TmpTileset();

    /** Keep tmp tileset when destructor kicks in if true.
     *  Default behaviour is to drop tileset.
     */
    void keep(bool value) { keep_ = value; };

    /** Store tile. Extra flags are stored into tile index along mesh/atlas
     *  existence.
     *
     * Usage note: TileIndex::Flag::watertight flag is used by accompanying
     * TmpTsEncoder to identify tiles that do not need patching from parent.
     */
    void store(const TileId &tileId, const Mesh &mesh
               , const Atlas &atlas
               , const TileIndex::Flag::value_type extraFlags
               = TileIndex::Flag::watertight);

    /** Loading function return type.
     */
    typedef std::tuple<Mesh::pointer, opencv::HybridAtlas::pointer
                       , TileIndex::Flag::value_type> Tile;

    /** Load tile from temporary storage.
     *
     * \param tileId tile ID
     * \param quality texture quality of loaded atlas
     * \return tile's mesh and hybrid atlas and tile flags as a tupple
     */
    Tile load(const TileId &tileId, int quality) const;

    /** Flushes data to disk.
     */
    void flush();

    /** Returns dataset tileindex (union of all tileindices).
     */
    TileIndex tileIndex() const;

    /** Root directory.
     */
    const boost::filesystem::path root() const { return root_; }

private:
    class Slice;

    boost::filesystem::path root_;
    bool keep_;

    mutable std::mutex mutex_;
    std::vector<std::shared_ptr<Slice>> slices_;
};

} } } // namespace vtslibs::vts::tools

#endif // vts_tools_tmptileset_hpp_included


