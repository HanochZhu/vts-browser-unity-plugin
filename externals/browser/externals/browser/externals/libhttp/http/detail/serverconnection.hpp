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
#ifndef http_detail_serverconnection_hpp_included_
#define http_detail_serverconnection_hpp_included_

#include <memory>
#include <string>
#include <vector>

#include "utility/enum-io.hpp"

#include "detail.hpp"

namespace http { namespace detail {

class ServerConnection
    : boost::noncopyable
    , public std::enable_shared_from_this<ServerConnection>
{
public:
    typedef std::shared_ptr<ServerConnection> pointer;
    typedef std::set<pointer> set;

    ServerConnection(Http::Detail &owner, asio::io_service &ios
               , const ContentGenerator::pointer &contentGenerator)
        : id_(++idGenerator_)
        , lm_(dbglog::make_module(str(boost::format("conn:%s") % id_)))
        , owner_(owner), ios_(ios), strand_(ios), socket_(ios_)
        , requestData_(1 << 13) // max line size; TODO: make configurable
        , state_(State::ready)
        , contentGenerator_(contentGenerator)
    {}

    tcp::socket& socket() {return  socket_; }

    void sendResponse(const Request &request, const Response &response
                      , const std::string &data, bool persistent = false)
    {
        sendResponse(request, response, data.data(), data.size(), persistent);
    }

    void sendResponse(const Request &request, const Response &response
                      , const void *data = nullptr, const size_t size = 0
                      , bool persistent = false);

    void sendResponse(const Request &request, const Response &response
                      , const SinkBase::DataSource::pointer &source);

    void start();

    bool valid() const;

    void closeConnection();

    dbglog::module& lm() { return lm_; }

    bool finished() const;

    void setAborter(const ServerSink::AbortedCallback &ac);

    ContentGenerator::pointer contentGenerator() { return contentGenerator_; }

    void countRequest() { owner_.request(); }

private:
    void startRequest();
    void readRequest();
    void readHeader(const pointer &self);

    Request pop() {
        auto r(requests_.front());
        requests_.erase(requests_.begin());
        return r;
    }

    void process();
    void badRequest();

    void close();
    void close(const bs::error_code &ec);

    void makeReady();

    void aborted();

    static std::atomic<std::size_t> idGenerator_;

    std::atomic<std::size_t> id_;
    dbglog::module lm_;

    Http::Detail &owner_;

    asio::io_service &ios_;
    asio::io_service::strand strand_;
    tcp::socket socket_;
    asio::streambuf requestData_;
    asio::streambuf responseData_;

    Request::list requests_;

    enum class State { ready, busy, busyClose, closed };
    State state_;

    std::mutex acMutex_;
    ServerSink::AbortedCallback ac_;
    ContentGenerator::pointer contentGenerator_;
};

} } // namespace http::detail

#endif // http_detail_serverconnection_hpp_included_
