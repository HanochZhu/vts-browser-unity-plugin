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

#ifndef jsoncpp_io_hpp_included_
#define jsoncpp_io_hpp_included_

#include <iostream>

#include <boost/filesystem/path.hpp>

#include "dbglog/dbglog.hpp"

#include "json.hpp"

namespace Json {

template <typename ExceptionType = RuntimeError>
Value read(std::istream &is, const boost::filesystem::path &path = "UNKNOWN"
           , const std::string &what = "");

bool read(std::istream &is, Value &value);

void write(std::ostream &os, const Value &value
           , bool humanReadable = true);

// Logging helper
namespace detail {
struct JsonLogger;
std::ostream& operator<<(std::ostream &os, const JsonLogger &log);
} // namespace detail

detail::JsonLogger log(const Json::Value &value, bool humanReadable = true);

// inlines

template <typename ExceptionType>
Value read(std::istream &is, const boost::filesystem::path &path
           , const std::string &what)
{
    std::string err;
    Value root;
    if (!parseFromStream(CharReaderBuilder(), is, &root, &err)) {
        if (what.empty()) {
            LOGTHROW(err2, ExceptionType)
                << "Unable to read JSON from file " << path
                << ": <" << err << ">.";
        } else {
            LOGTHROW(err2, ExceptionType)
                << "Unable to read " << what << " from file " << path
                << ": <" << err << ">.";
        }
    }
    return root;
}

inline bool read(std::istream &is, Value &value)
{
    std::string err;
    return parseFromStream(CharReaderBuilder(), is, &value, &err);
}

namespace detail {
struct JsonLogger {
    const Json::Value &value;
    bool humanReadable;
};

inline std::ostream&
operator<<(std::ostream &os, const detail::JsonLogger &log)
{
    write(os, log.value, log.humanReadable);
    return os;
}

} // namespace detail

inline detail::JsonLogger log(const Json::Value &value, bool humanReadable) {
    return { value, humanReadable };
}

} // namespace Json

#endif // jsoncpp_io_hpp_included_
