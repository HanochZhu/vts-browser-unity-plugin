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
 * \file storage/support.hpp
 * \author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Support files
 */

#ifndef vtslibs_storage_support_hpp_included_
#define vtslibs_storage_support_hpp_included_

#include <cstddef>
#include <ctime>
#include <string>
#include <map>

namespace vtslibs { namespace storage {

struct SupportFile {
    typedef std::map<std::string, SupportFile> Files;
    typedef std::map<std::string, std::string> Vars;

    const unsigned char *data;
    std::size_t size;
    std::time_t lastModified;
    const char *contentType;
    bool isTemplate;

    SupportFile(const unsigned char *data, std::size_t size
                , std::time_t lastModified, const char *contentType
                , bool isTemplate = false)
        : data(data), size(size), lastModified(lastModified)
        , contentType(contentType), isTemplate(isTemplate)
    {}

    std::string expand(const Vars *vars, const Vars *defaults) const;
};

} } // namespace vtslibs::storage

#endif // vtslibs_storage_support_hpp_included_
