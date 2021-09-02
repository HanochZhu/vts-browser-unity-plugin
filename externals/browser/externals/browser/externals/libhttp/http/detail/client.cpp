/**
 * Copyright (c) 2017-2019 Melown Technologies SE
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

#include <chrono>

#include <boost/format.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/chrono/duration.hpp>
#include <boost/asio/steady_timer.hpp>

#include "dbglog/dbglog.hpp"

#include "utility/streams.hpp"
#include "utility/raise.hpp"

#include "../error.hpp"

#include "detail.hpp"

#include "curl.hpp"
#include "types.hpp"

namespace ba = boost::algorithm;

namespace http { namespace detail {

#define CHECK_CURL_STATUS(op, what)                                     \
    do {                                                                \
        auto res(op);                                                   \
        if (res != CURLE_OK) {                                          \
            LOGTHROW(err2, Error)                                       \
                << "Failed to perform easy CURL operation <" << what    \
                << ">:" << res << ", "                                  \
                << ::curl_easy_strerror(res)                            \
                << ">.";                                                \
        }                                                               \
    } while (0)

#define LOG_CURL_STATUS(op, what)                                       \
    do {                                                                \
        auto res(op);                                                   \
        if (res != CURLE_OK) {                                          \
            LOG(err2)                                                   \
                << "Failed to perform easy CURL operation <" << what    \
                << ">:" << res << ", "                                  \
                << ::curl_easy_strerror(res)                            \
                << ">.";                                                \
        }                                                               \
    } while (0)

#define SETOPT(name, value)                                     \
    CHECK_CURL_STATUS(::curl_easy_setopt(easy_, name, value)    \
                      , "curl_easy_setopt")

#define CHECK_CURLM_STATUS(op, what)                                    \
    do {                                                                \
        auto res(op);                                                   \
        if (res != CURLM_OK) {                                          \
            LOGTHROW(err2, Error)                                       \
                << "Failed to perform multi CURL operation <" << what   \
                << ">:" << res << ", "                                  \
                << ::curl_multi_strerror(res)                           \
                << ">.";                                                \
        }                                                               \
    } while (0)

#define LOG_CURLM_STATUS(op, what)                                      \
    do {                                                                \
        auto res(op);                                                   \
        if (res != CURLM_OK) {                                          \
            LOG(err2)                                                   \
                << "Failed to perform multi CURL operation <" << what   \
                << ">:" << res << ", "                                  \
                << ::curl_multi_strerror(res)                           \
                << ">.";                                                \
        }                                                               \
    } while (0)

#define MSETOPT(name, value)                                       \
    CHECK_CURLM_STATUS(::curl_multi_setopt(multi_, name, value)    \
                       , "curl_multi_setopt")

ClientConnection* connFromEasy(CURL *easy)
{
    ClientConnection *conn(nullptr);
    LOG_CURL_STATUS(::curl_easy_getinfo(easy, CURLINFO_PRIVATE, &conn)
                    , "curl_easy_getinfo");
    return conn;
}

extern "C" {

std::size_t http_curlclient_write(void *ptr, std::size_t size
                                  , std::size_t nmemb, void *userp)
{
    auto count(size * nmemb);
    static_cast<ClientConnection*>(userp)
        ->store(static_cast<char*>(ptr), count);
    return count;
}

std::size_t http_curlclient_header(char *buf, std::size_t size
                                   , std::size_t nmemb, void *userp)
{
    auto count(size * nmemb);
    static_cast<ClientConnection*>(userp)->header(buf, count);
    return count;
}

::curl_socket_t http_curlclient_opensocket(void *clientp,
                                           ::curlsocktype purpose,
                                           ::curl_sockaddr *address)
{
    return static_cast<CurlClient*>(clientp)->open_cb(purpose, address);
}

int http_curlclient_closesocket(void *clientp, ::curl_socket_t item)
{
    return static_cast<CurlClient*>(clientp)->close_cb(item);
}

} // extern "C"

ClientConnection
::ClientConnection(CurlClient &owner
                   , const std::string &location
                   , const ClientSink::pointer &sink
                   , const ContentFetcher::RequestOptions &options)
    : easy_(::curl_easy_init()), headers_()
    , location_(location), sink_(sink)
    , maxAge_(constants::cacheUnspecified)
    , expires_(constants::cacheUnspecified)
{
    LOG(info2) << "Starting transfer from <" << location_ << ">.";

    if (!easy_) {
        LOGTHROW(err2, Error)
            << "Failed to create easy CURL handle.";
    }

    struct Guard {
        ::CURL *easy;
        ~Guard() { if (easy) {
                LOG(warn2) << "Destroying easy handle due to an error.";
                ::curl_easy_cleanup(easy);
            } }
    } guard{ easy_ };

    // set myself as a private data
    SETOPT(CURLOPT_PRIVATE, this);

    // switch off SIGALARM
    SETOPT(CURLOPT_NOSIGNAL, 1L);
    // retain last modified time
    SETOPT(CURLOPT_FILETIME, 1L);

#if LIBCURL_VERSION_NUM >= 0x072100 // 7.33.0
    // try to force HTTP/2.0
    if (::curl_easy_setopt(easy_, CURLOPT_HTTP_VERSION,
                           CURL_HTTP_VERSION_2_0) != CURLE_OK) {
        // fallback force HTTP/1.1
        SETOPT(CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    }
#else
    // force HTTP/1.1
    SETOPT(CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
#endif

    // use user agent
    if (!options.userAgent.empty()) {
        SETOPT(CURLOPT_USERAGENT, options.userAgent.c_str());
    }

    if (options.lastModified >= 0) {
        headers_ = ::curl_slist_append
            (headers_
             , ("If-Modified-Since: " + formatHttpDate(options.lastModified))
             .c_str());
    }

    // push custom headers
    {
        std::ostringstream os;
        for (const auto &header : options.headers) {
            os.str("");
            // TODO: sanitize header name/value
            os << header.first << ": " << header.second;
            headers_ = ::curl_slist_append(headers_, os.str().c_str());
        }
    }

    // follow redirects
    SETOPT(CURLOPT_FOLLOWLOCATION, long(options.followRedirects));

    // single shot
    if (!options.reuse) { SETOPT(CURLOPT_FORBID_REUSE, long(1)); }

    // set timeout
    if (options.timeout > 0) {
        SETOPT(CURLOPT_TIMEOUT_MS, long(options.timeout));
    }

    // set (optional) headers
    if (headers_) { SETOPT(CURLOPT_HTTPHEADER, headers_); }

    // calbacks

    // header op
    SETOPT(CURLOPT_HEADERFUNCTION, &http_curlclient_header);
    SETOPT(CURLOPT_HEADERDATA, this);

    // write op
    SETOPT(CURLOPT_WRITEFUNCTION, &http_curlclient_write);
    SETOPT(CURLOPT_WRITEDATA, this);

    // and finally set url
    SETOPT(CURLOPT_URL, location_.c_str());

    SETOPT(CURLOPT_OPENSOCKETFUNCTION, &http_curlclient_opensocket);
    SETOPT(CURLOPT_OPENSOCKETDATA, &owner);
    SETOPT(CURLOPT_CLOSESOCKETFUNCTION, &http_curlclient_closesocket);
    SETOPT(CURLOPT_CLOSESOCKETDATA, &owner);

    // release from guard
    guard.easy = nullptr;
}

ClientConnection::~ClientConnection()
{
    if (easy_) { ::curl_easy_cleanup(easy_); }
    if (headers_) { ::curl_slist_free_all(headers_); }
}

void ClientConnection::notify(::CURLcode result)
{
    if (result != CURLE_OK) {
        sink_->error(utility::makeError<Error>
                     ("Transfer of <%s> failed: <%d, %s>."
                      , location_, result, ::curl_easy_strerror(result)));
        return;
    }

    bool logged(false);
    try {
        long int httpCode(500);
        CHECK_CURL_STATUS(::curl_easy_getinfo
                          (easy_, CURLINFO_RESPONSE_CODE, &httpCode)
                          , "curl_easy_getinfo");

        LOG(info2)
            << "Transfer from <" << location_ << "> finished, status="
            << httpCode << ".";
        logged = true;

        switch (httpCode / 100) {
        case 2: {
            // cool, we have some content, fetch info and report it
            long int lastModified;
            CHECK_CURL_STATUS(::curl_easy_getinfo
                              (easy_, CURLINFO_FILETIME, &lastModified)
                              , "curl_easy_getinfo");
            char *contentType;
            CHECK_CURL_STATUS(::curl_easy_getinfo
                              (easy_, CURLINFO_CONTENT_TYPE, &contentType)
                              , "curl_easy_getinfo");

            // expires header
            std::time_t expires(maxAge_);
            if (maxAge_ == constants::cacheUnspecified) {
                expires = expires_;
            }

            sink_->content(content_
                           , http::SinkBase::FileInfo
                           ((contentType ? contentType
                             : "application/octet-stream")
                            , lastModified, long(expires)));
            break;
        }

        case 3: {
            switch (httpCode) {
            case 300:
                sink_->error(make_error_code
                             (utility::HttpCode::MultipleChoices));
                break;

            case 305:
                sink_->error(make_error_code
                             (utility::HttpCode::UseProxy));
                break;

            case 306:
                sink_->error(make_error_code
                             (utility::HttpCode::SwitchProxy));
                break;

            case 304:
                sink_->notModified();
                break;

            case 301:
            case 302:
            case 303:
            case 307:
            case 308: {
                char *url(nullptr);
                LOG_CURL_STATUS(::curl_easy_getinfo
                                (easy_, CURLINFO_EFFECTIVE_URL, &url)
                                , "curl_easy_getinfo");

                sink_->redirect(url, static_cast<utility::HttpCode>(httpCode));
                break; }

            default:
                sink_->error(utility::make_http_error_code(httpCode));
            }
            break;
        }

        case 4:
            switch (httpCode) {
            case 400:
                sink_->error(make_error_code(utility::HttpCode::BadRequest));
                break;

            case 401:
                sink_->error(make_error_code
                             (utility::HttpCode::NotAuthorized));
                break;

            case 404:
                sink_->error(make_error_code(utility::HttpCode::NotFound));
                break;

            case 405:
                sink_->error(make_error_code(utility::HttpCode::NotAllowed));
                break;

            default:
                sink_->error(utility::make_http_error_code(httpCode));
                break;
            }
            break;

        default:
            switch (httpCode) {
            case 500:
                sink_->error(make_error_code
                             (utility::HttpCode::InternalServerError));
                break;

            case 501:
                sink_->error(make_error_code
                             (utility::HttpCode::NotImplemented));
                break;

            case 502:
                sink_->error(make_error_code
                             (utility::HttpCode::BadGateway));
                break;

            case 503:
                sink_->error(make_error_code
                             (utility::HttpCode::ServiceUnavailable));
                break;

            case 504:
                sink_->error(make_error_code
                             (utility::HttpCode::GatewayTimeout));
                break;

            default:
                sink_->error(utility::make_http_error_code(httpCode));
                break;
            }
            break;
        }
    } catch (...) {
        if (!logged) {
            LOG(err2)
                << "Transfer from <" << location_ << "> finished, "
                "status unknown.";
        }
        sink_->error();
    }
}

void ClientConnection::store(const char *data, std::size_t size)
{
    content_.append(data, size);
}

void ClientConnection::header(const char *data, std::size_t size)
{
    // empty line?
    if (size <= 2) {
        if (!headerName_.empty()) {
            processHeader();
        }
        return;
    }

    // previous line continuation?
    if (std::isspace(*data)) {
        if (!headerName_.empty()) {
            // we have previous header, append line (minus terminator)
            headerValue_.append(data, size - 2);
        }
        return;
    }

    if (!headerName_.empty()) {
        processHeader();
    }

    const auto *colon
        (static_cast<const char*>(std::memchr(data, ':', size - 2)));
    if (!colon) { return; }

    // new header
    headerName_.assign(data, colon);
    headerValue_.assign(colon + 1, data + size - 2);
}

void ClientConnection::processHeader()
{
    if (ba::iequals(headerName_, "Cache-Control")) {
        // cache control, parse
        std::string token;
        std::time_t ma(constants::cacheUnspecified);
        std::time_t sma(constants::cacheUnspecified);
        bool noCache(false);
        bool private_(false);
        bool public_(false);
        bool mustRevalidate(false);

        // process value (no exception, relaxed parsing)
        std::istringstream is(headerValue_);
        while (is) {
            is >> token;
            if (ba::istarts_with(token, "private")) {
                private_ = true;
            } else if (ba::istarts_with(token, "public")) {
                public_ = true;
            } if (ba::istarts_with(token, "no-cache")) {
                noCache = true;
                break;
            } else if (ba::istarts_with(token, "s-maxage=")) {
				std::istringstream t(token);
                t.ignore(9);
                t >> sma;
            } else if (ba::istarts_with(token, "max-age=")) {
                std::istringstream t(token);
                t.ignore(8);
                t >> ma;
            } else if (ba::istarts_with(token, "must-revalidate")) {
                mustRevalidate = true;
            }
        }

        // what to do with public?
        (void) public_;

        if (private_) {
            // private -> we cannot cache it
            maxAge_ = 0;
        } else if (noCache) {
            // cache forbidden
            maxAge_ = 0;
        } else if (mustRevalidate) {
            maxAge_ = constants::mustRevalidate;
        } else if (sma >= 0) {
            maxAge_ = sma;
        } else if (ma >= 0) {
            maxAge_ = ma;
        } else {
            // have no idea
            maxAge_ = constants::cacheUnspecified;
        }
    }

    headerName_.clear();
}

extern "C" {

int http_curlclient_socket(::CURL *easy, ::curl_socket_t s, int what
                           , void *userp, void *socketp)
{
    return static_cast<CurlClient*>(userp)
        ->handle_cb(easy, s, what, socketp);
}

int http_curlclient_timer(CURLM*, long int timeout, void *userp)
{
    return static_cast<CurlClient*>(userp)->timeout_cb(timeout);
}

} // extern "C"

CurlClient::CurlClient(int id, const ContentFetcher::Options *options)
    : multi_(::curl_multi_init())
    , work_(std::ref(ios_))
    , timer_(ios_)
    , runningTransfers_()
{
    if (!multi_) {
        LOGTHROW(err2, Error)
            << "Failed to create mutli CURL handle.";
    }

    MSETOPT(CURLMOPT_SOCKETFUNCTION, &http_curlclient_socket);
    MSETOPT(CURLMOPT_SOCKETDATA, this);
    MSETOPT(CURLMOPT_TIMERFUNCTION, &http_curlclient_timer);
    MSETOPT(CURLMOPT_TIMERDATA, this);

    if (options) {
#if LIBCURL_VERSION_NUM >= 0x071E00 // 7.30.0
        if (options->maxHostConnections) {
            MSETOPT(CURLMOPT_MAX_HOST_CONNECTIONS,
                    options->maxHostConnections);
        }
        if (options->maxTotalConections) {
            MSETOPT(CURLMOPT_MAX_TOTAL_CONNECTIONS,
                    options->maxTotalConections);
        }
#endif

        if (options->maxCacheConections) {
            MSETOPT(CURLMOPT_MAXCONNECTS, options->maxCacheConections);
        }
        if (options->pipelining) {
            MSETOPT(CURLMOPT_PIPELINING, options->pipelining);
        }
    }

    start(id);
}

CurlClient::~CurlClient()
{
    if (!multi_) { return; }

    // stop the machinery: this gives us free hand to manipulate with
    // connections
    stop();

    // destroy sockets and connections
    while (!connections_.empty()) {
        auto *conn(*connections_.begin());
        remove(conn);
    }
    sockets_.clear();

    LOG_CURLM_STATUS(::curl_multi_cleanup(multi_)
                     , "curl_multi_cleanup");
}

void CurlClient::start(unsigned int id)
{
    std::thread worker(&CurlClient::run, this, id);
    worker_.swap(worker);
}

void CurlClient::stop()
{
    LOG(info2) << "Stopping";
    work_ = boost::none;
    worker_.join();
    ios_.stop();
}

void CurlClient::run(unsigned int id)
{
    dbglog::thread_id(str(boost::format("chttp:%u") % id));
    LOG(info2) << "Spawned HTTP client worker id:" << id << ".";

    for (;;) {
        try {
            ios_.run();
            LOG(info2) << "Terminated HTTP client worker id:" << id << ".";
            return;
        } catch (const std::exception &e) {
            LOG(err3)
                << "Uncaught exception in HTTP client worker: <" << e.what()
                << ">. Going on.";
        }
    }
}

void CurlClient::add(std::unique_ptr<ClientConnection> &&conn)
{
    auto *c(conn.get());
    connections_.insert(c);
    LOG(debug) << "Adding connection " << c->handle();
    CHECK_CURLM_STATUS(::curl_multi_add_handle
                       (multi_, c->handle())
                       , "curl_multi_add_handle");
    conn.release();
}

void CurlClient::remove(ClientConnection *conn)
{
    if (connections_.erase(conn)) {
        std::unique_ptr<ClientConnection> ptr(conn);
        LOG(debug) << "Removing connection " << conn->handle();
        CHECK_CURLM_STATUS(::curl_multi_remove_handle
                           (multi_, conn->handle())
                           , "curl_multi_remove_handle");
    }
}

void CurlClient::fetch(const std::string &location
                       , const ClientSink::pointer &sink
                       , const ContentFetcher::RequestOptions &options)
{
    if (!options.delay) {
        // immediate query
        ios_.post([=]() -> void
        {
            try {
                return add
                    (std::unique_ptr<ClientConnection>
                     (new ClientConnection(*this, location, sink, options)));
            } catch (...) {
                sink->error();
            }
        });
        return;
    }

    // delayed query
    auto timer(std::make_shared<asio::steady_timer>
               (ios_, std::chrono::milliseconds(options.delay)));

    // NB: we need to capture timer otherwise it would go out of scope
    timer->async_wait([timer, this, location, sink, options]
                      (const bs::error_code &ec) mutable -> void
    {
        if (ec != asio::error::operation_aborted) {
            try {
                add(std::unique_ptr<ClientConnection>
                    (new ClientConnection(*this, location, sink, options)));
            } catch (...) {
                sink->error();
            }
            return;
        }

        // forward error
        sink->error(ec);
    });
}

::curl_socket_t CurlClient::open_cb(::curlsocktype purpose,
                                    ::curl_sockaddr *address)
{
    if (purpose == CURLSOCKTYPE_IPCXN && address->family == AF_INET)
    {
        // remember socket
        auto socket(Socket::create(ios_));

        bs::error_code ec;
        socket->socket.open(ip::tcp::v4(), ec);

        if (ec) {
            LOG(warn2) << "Failed to open TCP socket: <" << ec << ">.";
            return CURL_SOCKET_BAD;
        }

        auto native(socket->handle());
        // LOG(info4)
        //     << "Opened socket " << socket.get() << ", " << native << ".";
        sockets_.insert(Socket::map::value_type(native, std::move(socket)));
        return native;
    }

    // unsupported
    return CURL_SOCKET_BAD;
}

int CurlClient::close_cb(::curl_socket_t s)
{
    // find socket
    auto fsockets(sockets_.find(s));
    if (fsockets == sockets_.end()) {
        // LOG(info4) << "Unknown socket " << s << ".";
        return -1;
    }

    // manipulate
    auto *socket(fsockets->second.get());

    // LOG(info4) << "Closing socket: " << socket->handle();
    if (socket->inProgress()) {
        // cancel pending operations now
        bs::error_code ec;
        socket->socket.cancel(ec);
    }

    // remove from map
    // LOG(info4) << "Erasing socket " << s << ".";
    sockets_.erase(s);
    return 0;
}

void CurlClient::prepareRead(Socket *socket)
{
    // LOG(info4) << "Prepare read: " << socket->handle();
    Socket::makeReading(socket, true);
    socket->socket.async_read_some
        (asio::null_buffers(), [=] (bs::error_code ec, std::size_t)
    {
        if (ec == asio::error::operation_aborted) { ec = {}; };

        auto handle(socket->handle());

        if (!ec) {
            // LOG(info4) << "Reading: " << socket->handle();
            action(handle, CURL_CSELECT_IN);

            if (Socket::makeReading(socket, false)) {
                // LOG(info4) << "Tying to make reading again";
                if (socket->canRead()) {
                    // LOG(info4)
                    //     << "Calling prepare read again: " << socket->handle();
                    prepareRead(socket);
                }
            }
            return;
        }

        // LOG(info4) << "Error: " << socket->handle();
        action(handle, CURL_CSELECT_ERR);
        Socket::makeReading(socket, false);
    });

    // LOG(info4) << "Out of read: " << socket->handle();
}

void CurlClient::prepareWrite(Socket *socket)
{
    // LOG(info4) << "Prepare write: " << socket->handle();
    Socket::makeWriting(socket, true);
    socket->socket.async_write_some
        (asio::null_buffers(), [=] (bs::error_code ec, std::size_t)
    {
        if (ec == asio::error::operation_aborted) { ec = {}; };

        auto handle(socket->handle());

        if (!ec) {
            // LOG(info4) << "Writing: " << socket->handle();
            action(handle, CURL_CSELECT_OUT);

            if (Socket::makeWriting(socket, false)) {
                if (socket->canWrite()) {
                    // LOG(info4)
                    //     << "Calling prepare write again: " << socket->handle();
                    prepareWrite(socket);
                }
            }
            return;
        }

        // let curl handle errors
        // LOG(info4) << "Error: " << socket->handle();
        action(handle, CURL_CSELECT_ERR);
        Socket::makeWriting(socket, false);
    });

    // LOG(info4) << "Out of write: " << socket->handle();
}

int CurlClient::handle_cb(::CURL*, ::curl_socket_t s, int what, void *socketp)
{
    if (what == CURL_POLL_REMOVE) {
        // LOG(info4) << "Asked to remove socket " << s << ".";
        if (!socketp) {
            // LOG(info4) << "No socket.";
            return 0;
        }

        auto *socket(static_cast<Socket*>(socketp));

        // LOG(info4) << "Removing socket: " << socket->handle()
        //            << " (" << socket << ")";
        socket->rw(false, false);

        if (socket->inProgress()) {
            LOG(debug) << "Cancelling socket io: " << socket->handle();
            bs::error_code ec;
            socket->socket.cancel(ec);
        }

        // nothing to do
        return 0;
    }

    Socket *socket;

    if (!socketp) {
        // no associated data (socket) provided -> find it via handle
        auto fsockets(sockets_.find(s));
        if (fsockets == sockets_.end()) {
            // OOPS, not found
            LOG(warn2) << "Unknown socket " << s << " in handle callback.";
            return -1;
        }

        // assign
        socket = fsockets->second.get();

        // assign connection to socket
        CHECK_CURLM_STATUS(::curl_multi_assign(multi_, s, socket)
                           , "curl_multi_assign");
    } else {
        socket = static_cast<Socket*>(socketp);
    }

    // bool oldRead(socket->read);
    // bool oldWrite(socket->write);

    // update read/write markers
    socket->rw(what & CURL_POLL_IN, what & CURL_POLL_OUT);

    // LOG(info4) << "Socket state change: " << socket
    //            << ": read " << oldRead << "->" << socket->read
    //            << ", write " << oldWrite << "->" << socket->write;

    if (socket->canRead()) { prepareRead(socket); }
    if (socket->canWrite()) { prepareWrite(socket); }

    return 0;
}

int CurlClient::timeout_cb(long int timeout)
{
    // LOG(info4) << "Handling timeout (" << timeout << ")";

    if (timeout > 0) {
        // positive: update timer and wait
        timer_.cancel();
        timer_.expires_from_now(boost::posix_time::millisec(timeout));
        timer_.async_wait([this](const bs::error_code &ec)
        {
            if (ec != asio::error::operation_aborted) {
                // regular timeout
                action();
            }
        });
    } else if (!timeout) {
        ios_.post([this]() { action(); });
    } else {
        // negative: cancel
        timer_.cancel();
    }
    return 0;
}

void CurlClient::action(::curl_socket_t socket, int what)
{
    CHECK_CURLM_STATUS(::curl_multi_socket_action
                       (multi_, socket, what, &runningTransfers_)
                       , "curl_multi_socket_action");

    // LOG(info4) << "action(" << socket << ", " << what << ")";

    // check message info
    int left(0);
    while (auto *msg = ::curl_multi_info_read(multi_, &left)) {
        if (msg->msg != CURLMSG_DONE) { continue; }

        auto *conn(connFromEasy(msg->easy_handle));
        if (!conn) {
            LOG(info2) << "No connection associated with CURL easy handle "
                       << msg->easy_handle << "; skipping.";
            continue;
        }

        // notify finished transfer
        conn->notify(msg->data.result);

        // get rid of connection
        remove(conn);
    }
}

} } // namespace http::detail

namespace http {

void Http::Detail::fetch_impl(const std::string &location
                              , const ClientSink::pointer &sink
                              , const ContentFetcher::RequestOptions &options)
{
    // must be under lock
    try {
        std::unique_lock<std::mutex> lock(clientMutex_);

        if (clients_.empty()) {
            LOGTHROW(err2, Error)
                << "Cannot perform fetch request: no client is running.";
        }

        (*currentClient_)->fetch(location, sink, options);
        if (++currentClient_ == clients_.end()) {
            currentClient_ = clients_.begin();
        }
    } catch (...) {
        sink->error();
    }
}

} // namespace http

/** CURL initialization
 */
#if defined(__linux__)
#  include <signal.h>
#endif

/** Special configuration on linux: ignore SIGPIPE.
 */
namespace http { namespace {
void linux() {
#if defined(__linux__) && CURL_AT_LEAST_VERSION(7, 34, 0)
        auto curl(::curl_easy_init());
        if (!curl) {
            std::cerr
                << "HTTP clinet: CURL: cannot create easy handler."
                << std::endl;
            return;
        }

        const ::curl_tlssessioninfo *ssl(nullptr);
#if CURL_AT_LEAST_VERSION(7, 48, 0)
        auto res(::curl_easy_getinfo(curl, CURLINFO_TLS_SSL_PTR, &ssl));
#else
        auto res(::curl_easy_getinfo(curl, CURLINFO_TLS_SESSION, &ssl));
#endif

        if (res != CURLE_OK) {
            std::cerr << "Error getting CURL SSL info: <"
                      << ::curl_easy_strerror(res) << ">."
                      << std::endl;
        } else if (ssl->backend == CURLSSLBACKEND_OPENSSL) {
            struct sigaction act{};
            act.sa_handler = SIG_IGN;
            act.sa_flags = SA_RESTART;
            if (::sigaction(SIGPIPE, &act, NULL) == -1) {
                std::cerr << "Unable to ignore SIGPIPE.";
            }
        }
        ::curl_easy_cleanup(curl);
#endif
}

static volatile struct Initializer {
    Initializer()
        : initialized(false)
    {
        auto res(::curl_global_init(CURL_GLOBAL_ALL));
        if (res != CURLE_OK) {
            std::cerr << "Error getting CURL SSL info: <"
                      << ::curl_easy_strerror(res) << ">."
                      << std::endl;
            return;
        }
        initialized = true;

        linux();
    }

    ~Initializer() {
        if (initialized) { ::curl_global_cleanup(); }
    }

    bool initialized;

} curlInitializer;

} } // namespace http::
