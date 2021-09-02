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
#include <cstdlib>
#include <utility>
#include <functional>
#include <future>
#include <map>

#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/thread.hpp>

#include "utility/streams.hpp"
#include "utility/tcpendpoint-io.hpp"
#include "utility/buildsys.hpp"
#include "utility/format.hpp"
#include "service/cmdline.hpp"

#include "http/http.hpp"
#include "http/resourcefetcher.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

class SelfTest
    : public service::Cmdline
    , public http::ContentGenerator
{
public:
    SelfTest()
        : service::Cmdline("selftest", BUILD_TARGET_VERSION)
        , httpThreadCount_(boost::thread::hardware_concurrency())
        , httpClientThreadCount_(1)
    {}

private:
    void configuration(po::options_description &cmdline
                       , po::options_description &config
                       , po::positional_options_description &pd);

    void configure(const po::variables_map &vars);

    int run();

    virtual void generate_impl(const std::string &location
                               , const http::ServerSink::pointer &sink);

    unsigned int httpThreadCount_;
    unsigned int httpClientThreadCount_;

    boost::optional<http::Http> http_;
};

void SelfTest::configuration(po::options_description &cmdline
                             , po::options_description &config
                             , po::positional_options_description &pd)
{
    config.add_options()
        ("http.threadCount", po::value(&httpThreadCount_)
         ->default_value(httpThreadCount_)->required()
         , "Number of server HTTP threads.")
        ("http.client.threadCount", po::value(&httpClientThreadCount_)
         ->default_value(httpClientThreadCount_)->required()
         , "Number of client HTTP threads.")
        ;

    (void) cmdline;
    (void) pd;
}


void SelfTest::configure(const po::variables_map &vars)
{
    LOG(info3, log_)
        << "Config:"
        << "\n\thttp.threadCount = " << httpThreadCount_
        << "\n\thttp.client.threadCount = " << httpClientThreadCount_
        ;

    (void) vars;
}

class DataSource : public http::ServerSink::DataSource {
public:
    DataSource(const std::vector<std::string> &data)
        : data_(data), idata_(data_.begin()), edata_(data_.end())
        , start_(0)
    {}

    virtual http::SinkBase::FileInfo stat() const {
        return { "text/plain", -1 };
    }

    virtual std::size_t read(char *buf, std::size_t size
                             , std::size_t off)
    {
        for (;;) {
            // skip empty strings
            for (; (idata_ != edata_) && idata_->empty();
                 start_ += idata_->size(), ++idata_);
            // termination condition
            if (idata_ == edata_) { return 0; }

            auto local(off - start_);
            auto left(idata_->size() - local);
            if (!left) {
                // try next
                start_ += idata_->size();
                ++idata_;
                continue;
            }

            if (size > left) { size = left; }
            std::copy(idata_->data() + local
                      , idata_->data() + local + size
                      , buf);

            return size;
        }
    }

    virtual std::string name() const { return "memory"; }
    virtual void close() const {}
    virtual long size() const { return -1; }

private:
    const std::vector<std::string> &data_;
    std::vector<std::string>::const_iterator idata_;
    std::vector<std::string>::const_iterator edata_;
    std::size_t start_;
};

std::vector<std::string> data = {
    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~ \t\n\r"
    , "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~ \t\n\r"
    , "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~ \t\n\r"
    , "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~ \t\n\r"
    , "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~ \t\n\r"
    , "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~ \t\n\r"
    , "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~ \t\n\r"
    , "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~ \t\n\r"
    , "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~ \t\n\r"
    , "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~ \t\n\r"
    , "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~ \t\n\r"
};

void SelfTest::generate_impl(const std::string&
                             , const http::ServerSink::pointer &sink)
{
    sink->content(std::make_shared<DataSource>(data));
}

int SelfTest::run()
{
    http_ = boost::in_place();
    http_->startClient(httpClientThreadCount_);

    auto local(http_->listen(utility::TcpEndpoint("127.0.0.1:0"), *this));
    http_->startServer(httpThreadCount_);

    http::ResourceFetcher rf(http_->fetcher());

    auto url(utility::format("http://%s/", local.value));

    typedef utility::ResourceFetcher::Query Query;
    typedef utility::ResourceFetcher::MultiQuery MultiQuery;

    // prefill query
    MultiQuery q;
    for (int i(8); i; --i) { q.add(url); }

    for (;;) {
        auto promise(std::make_shared<std::promise<void>>());

        rf.perform(q, [=, this](const MultiQuery &mq) mutable -> void
        {
            (void) mq;

            // ping
            promise->set_value();
        });

        // wait
        promise->get_future().get();
    }

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    return SelfTest()(argc, argv);
}
