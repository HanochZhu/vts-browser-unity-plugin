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
#ifndef http_error_hpp_included_
#define http_error_hpp_included_

#include <ctime>
#include <string>
#include <memory>
#include <stdexcept>

#include "utility/httpcode.hpp"

namespace http {

struct Error : std::runtime_error {
    Error(const std::string &message) : std::runtime_error(message) {}
};

typedef utility::HttpError HttpError;

#define HTTP_DEFINE_ERROR(NAME)                                         \
    struct NAME : HttpError {                                           \
        NAME(const std::string &message)                                \
            : HttpError(make_error_code(utility::HttpCode::NAME)        \
                                 , message) {}                          \
    }

HTTP_DEFINE_ERROR(NotAllowed);
HTTP_DEFINE_ERROR(NotFound);
HTTP_DEFINE_ERROR(NotAuthorized);
HTTP_DEFINE_ERROR(Forbidden);
HTTP_DEFINE_ERROR(BadRequest);
HTTP_DEFINE_ERROR(ServiceUnavailable);
HTTP_DEFINE_ERROR(InternalServerError);
HTTP_DEFINE_ERROR(NotModified);
HTTP_DEFINE_ERROR(RequestAborted);

#undef HTTP_DEFINE_ERROR

} // namespace http

#endif // http_error_hpp_included_
