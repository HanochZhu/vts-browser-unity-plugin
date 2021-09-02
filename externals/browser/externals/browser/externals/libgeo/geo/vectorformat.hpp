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
#ifndef geo_vectorformat_hpp_included_
#define geo_vectorformat_hpp_included_

#include <boost/variant.hpp>

#include "utility/enum-io.hpp"

namespace geo {

UTILITY_GENERATE_ENUM(VectorFormat,
                      ((geodataJson))
                      )

/** Returns content type string for given file format
 *  Always returns a valid string
 */
const char* contentType(VectorFormat format);

namespace vectorformat {

struct GeodataConfig {
    // "resolution" (quantization of all 3 coordinartes of geodata bounding box)
    unsigned int resolution;

    GeodataConfig()
        : resolution(4096)
    {}
};

typedef boost::variant<GeodataConfig> Config;

inline GeodataConfig& createGeodataConfig(Config &config)
{
    return boost::get<GeodataConfig>(config = GeodataConfig());
}

// add more helpers when needed

} // namespace vectorformat

} // namespace geo

#endif // geo_vectorformat_hpp_included_
