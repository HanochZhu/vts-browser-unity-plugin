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
#ifndef http_contentfetcher_hpp_included_
#define http_contentfetcher_hpp_included_

#include <ctime>
#include <string>
#include <vector>
#include <memory>
#include <exception>

#include "constants.hpp"
#include "sink.hpp"

namespace http {

class ContentFetcher {
public:
    typedef std::shared_ptr<ContentFetcher> pointer;

    virtual ~ContentFetcher() {}

    struct Options {
        Options()
            : maxHostConnections(0),
              maxTotalConections(0),
              maxCacheConections(0),
              pipelining(0)
        {}

        unsigned long maxHostConnections;
        unsigned long maxTotalConections;
        unsigned long maxCacheConections;
        long pipelining;
    };

    struct RequestOptions {
        RequestOptions()
            : followRedirects(true), lastModified(-1), reuse(true)
            , timeout(-1), delay()
        {}

        bool followRedirects;
        std::string userAgent;

        /** Sends If-Modified-Since if non-negative
         */
        std::time_t lastModified;

        /** Can we reuse existing connection?
         */
        bool reuse;

        long timeout;

        /** Headers.
         */
        typedef std::vector<std::pair<std::string, std::string>> Headers;
        Headers headers;

        /** Delayed request execution. Delay in milliseconds before request is
         *  performed.
         *  Zero means immediate action.
         */
        unsigned long delay;
    };

    void fetch(const std::string &location
               , const ClientSink::pointer &sink
               , const RequestOptions &options = RequestOptions());

private:
    virtual void fetch_impl(const std::string &location
                            , const ClientSink::pointer &sink
                            , const RequestOptions &options) = 0;
};

// inlines

inline void ContentFetcher::fetch(const std::string &location
                                  , const ClientSink::pointer &sink
                                  , const RequestOptions &options)
{
    return fetch_impl(location, sink, options);
}

} // namespace http

#endif // http_contentfetcher_hpp_included_

