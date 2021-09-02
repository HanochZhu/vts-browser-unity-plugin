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

#include <boost/optional.hpp>

#include "ondemandclient.hpp"
#include "contentfetcher.hpp"
#include "detail/curl.hpp"

namespace http {

struct OnDemandClient::Detail : public ContentFetcher
{
    Detail(std::size_t threadCount)
        : threadCount(threadCount ? threadCount : 1)
        , fetcher(*this) {}

    std::mutex mutex_;
    std::size_t threadCount;
    detail::CurlClient::list clients;
    detail::CurlClient::list::iterator currentClient;
    ResourceFetcher fetcher;

    virtual void fetch_impl(const std::string &location
                            , const ClientSink::pointer &sink
                            , const ContentFetcher::RequestOptions &options);
};

void OnDemandClient::Detail
::fetch_impl(const std::string &location
             , const ClientSink::pointer &sink
             , const ContentFetcher::RequestOptions &options)
{
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        if (clients.empty()) {
            for (int i(0); i < int(threadCount); ++i) {
                clients.push_back(std::make_shared<detail::CurlClient>(i));
            }
            currentClient = clients.begin();
        }

        (*currentClient)->fetch(location, sink, options);
        if (++currentClient == clients.end()) {
            currentClient = clients.begin();
        }
    } catch (...) {
        sink->error();
    }
}

OnDemandClient::OnDemandClient(std::size_t threads)
    : detail_(std::make_shared<Detail>(threads))
{}

utility::ResourceFetcher& OnDemandClient::fetcher() const
{
    return detail_->fetcher;
}

} // namespace http
