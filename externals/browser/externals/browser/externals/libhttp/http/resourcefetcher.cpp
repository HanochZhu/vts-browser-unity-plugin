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
#include "dbglog/dbglog.hpp"

#include "contentfetcher.hpp"
#include "resourcefetcher.hpp"

namespace asio = boost::asio;

namespace http {

namespace {
class QuerySink;

struct SingleQuerySink : public http::ClientSink {
    typedef std::shared_ptr<SingleQuerySink> pointer;

    SingleQuerySink(const std::shared_ptr<QuerySink> &owner
                    , ResourceFetcher::Query &query)
        : owner(owner), query(&query)
    {}

    virtual void content_impl(const void *data, std::size_t size
                              , const FileInfo &stat, bool
                              , const Header::list *headers);

    virtual void error_impl(const std::exception_ptr &exc);

    virtual void error_impl(const std::error_code &ec
                            , const std::string &message);

    virtual void redirect_impl(const std::string &url, utility::HttpCode code
                               , const CacheControl &cacheControl);

    std::shared_ptr<QuerySink> owner;
    ResourceFetcher::Query *query;
};

struct QuerySinkBase {
    std::shared_ptr<ResourceFetcher::MultiQuery> query_;
    ResourceFetcher::Done done_;

    QuerySinkBase(ResourceFetcher::MultiQuery &&query
                  , const ResourceFetcher::Done &done)
        : query_(std::make_shared<ResourceFetcher::MultiQuery>
                 (std::move(query)))
        , done_(done)
    {}
};

class QuerySink
    : private QuerySinkBase
    , public SingleQuerySink
{
public:
    typedef std::shared_ptr<QuerySink> pointer;

    QuerySink(ResourceFetcher::MultiQuery &&query
              , asio::io_service *ios
              , const ResourceFetcher::Done &done)
        : QuerySinkBase(std::move(query), done)
        , SingleQuerySink(((query_->size() == 1)
                           ? std::shared_ptr<QuerySink>(this, [](void*){})
                           : pointer())
                          , query_->front())
        , ios_(ios)
        , queriesLeft_(int(query_->size()))
    {}

    void ping() {
        // decrement
        if (!--queriesLeft_) {
            try {
                LOG(info1) << "All subqueries finished.";
                if (ios_) {
                    // need to copy
                    auto &done(done_);
                    auto &query(query_);
                    ios_->post([done, query]() {
                            done(std::move(*query));
                        });
                } else {
                    done_(std::move(*query_));
                }
            } catch (const std::exception &e) {
                LOG(err1)
                    << "Resource(s) fetch callback failed: <"
                    << e.what() << ">.";
            }
        }
    }

    /** Fetch entry point.
     */
    static void fetch(const pointer &sink
                      , http::ContentFetcher &contentFetcher)
    {
        if (sink->query_->empty()) {
            auto &q(sink->query_);
            // ehm...
            if (sink->ios_) {
                sink->ios_->post([=]() { sink->done_(std::move(*q)); });
            } else {
                sink->done_(std::move(*q));
            }
            return;
        }

        auto queryOptions([&](const ResourceFetcher::Query &query)
                          -> ContentFetcher::RequestOptions
        {
            ContentFetcher::RequestOptions options;
            options.reuse = query.reuse();
            options.timeout = query.timeout();
            options.delay = query.delay();
            const auto &headers(query.options());
            options.headers.assign(headers.begin(), headers.end());
            return options;
        });

        auto &q(*sink->query_);
        if (q.size() == 1) {
            // single query -> use sink
            const auto &query(q.front());
            contentFetcher.fetch(query.location(), sink, queryOptions(query));
            return;
        }

        // multiquery -> use multiple queries
        for (auto &query : q) {
            contentFetcher.fetch
                (query.location()
                 , std::make_shared<SingleQuerySink>(sink, query)
                 , queryOptions(query));
        }
    }

private:
    asio::io_service *ios_;
    std::atomic<int> queriesLeft_;
};

void SingleQuerySink::content_impl(const void *data, std::size_t size
                                   , const FileInfo &stat, bool
                                   , const Header::list*)
{
    // TODO: make better (use response Date field)
    std::time_t expires(-1);
    if (stat.cacheControl.maxAge && (*stat.cacheControl.maxAge >= 0)) {
        expires = std::time(nullptr) + *stat.cacheControl.maxAge;
    }
    query->set(stat.lastModified, expires, data, size, stat.contentType);
    owner->ping();
}

void SingleQuerySink::error_impl(const std::exception_ptr &exc)
{
    query->error(exc);
    owner->ping();
}

void SingleQuerySink::error_impl(const std::error_code &ec
                                 , const std::string&)
{
    query->error(ec);
    owner->ping();
}

void SingleQuerySink::redirect_impl(const std::string &url
                                    , utility::HttpCode code
                                    , const CacheControl&)
{
    // TODO: use max age
    query->redirect(url, make_error_code(code));
    owner->ping();
}

} // namespace

void ResourceFetcher::perform_impl(MultiQuery query, const Done &done)
    const
{
    QuerySink::fetch
        (std::make_shared<QuerySink>(std::move(query), queryIos_, done)
         , contentFetcher_);
}

} // namespace http
