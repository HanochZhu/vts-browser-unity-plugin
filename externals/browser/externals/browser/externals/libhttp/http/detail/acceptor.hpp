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
#ifndef http_detail_acceptor_hpp_included_
#define http_detail_acceptor_hpp_included_

#include <memory>
#include <string>
#include <vector>

#include "utility/enum-io.hpp"

#include "detail.hpp"

namespace http { namespace detail {

class Acceptor : public std::enable_shared_from_this<Acceptor> {
public:
    typedef std::shared_ptr<Acceptor> pointer;
    typedef std::vector<pointer> list;

    Acceptor(Http::Detail &owner, asio::io_service &ios
             , const utility::TcpEndpoint &listen
             , const ContentGenerator::pointer &contentGenerator)
        : owner_(owner), ios_(ios), strand_(ios)
        , acceptor_(ios_, listen.value, true)
        , contentGenerator_(contentGenerator)
    {}

    void start();

    typedef std::function<void(const pointer&)> StoppedHandler;

    void stop(const StoppedHandler &done);

    utility::TcpEndpoint localEndpoint() const {
        return utility::TcpEndpoint(acceptor_.local_endpoint());
    }

private:
    Http::Detail &owner_;
    asio::io_service &ios_;
    asio::io_service::strand strand_;
    tcp::acceptor acceptor_;
    ContentGenerator::pointer contentGenerator_;
};

} } // namespace http::detail

#endif // http_detail_acceptor_hpp_included_
