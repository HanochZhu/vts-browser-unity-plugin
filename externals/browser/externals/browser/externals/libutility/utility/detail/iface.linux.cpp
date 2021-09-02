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
#include <sys/types.h>
#include <ifaddrs.h>

#include <iostream>

#include <memory>
#include <stdexcept>
#include <system_error>

#include "iface.hpp"

namespace utility { namespace detail {

namespace ip = boost::asio::ip;

namespace {

template <typename Endpoint, typename Protocol>
Endpoint
endpointForIface(const Protocol &protocol
                 , const std::string &iface
                 , unsigned short portNum)
{
    int family;
    switch (protocol.family()) {
    case PF_INET: family = AF_INET; break;
    case PF_INET6: family = AF_INET6; break;
    default:
        throw std::runtime_error("Unsupported protocol family.");
    }

    struct ::ifaddrs *ifa;
    if (-1 == ::getifaddrs(&ifa)) {
        std::system_error e(errno, std::system_category(), "getifaddrs");
        throw e;
    }
    std::shared_ptr<struct ::ifaddrs> guard(ifa, [](struct ::ifaddrs *ifa) {
            freeifaddrs(ifa);
        });

    for (; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_name != iface) { continue; }
        if (ifa->ifa_addr->sa_family == family) {
            break;
        }
    }

    if (!ifa) {
        throw std::runtime_error("Interface <" + iface + "> not found.");
    }

    switch (protocol.family()) {
    case PF_INET:
        return { ip::address_v4
                (ntohl(reinterpret_cast<struct sockaddr_in*>
                       (ifa->ifa_addr)->sin_addr.s_addr))
                , portNum };

    case PF_INET6: {
        auto s6(reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_addr));
        ip::address_v6::bytes_type a;
        for (int i = 0; i < 16; ++i) { a[i] = s6->sin6_addr.s6_addr[i]; }

        return { ip::address_v6(a, s6->sin6_scope_id), portNum };
    } }

    throw std::runtime_error("Unsupported protocol family.");
}

} // namespace

ip::tcp::endpoint
tcpEndpointForIface(const ip::tcp &protocol
                    , const std::string &iface
                    , unsigned short portNum)
{
    return endpointForIface<ip::tcp::endpoint>(protocol, iface, portNum);
}

ip::udp::endpoint
udpEndpointForIface(const ip::udp &protocol
                    , const std::string &iface
                    , unsigned short portNum)
{
    return endpointForIface<ip::udp::endpoint>(protocol, iface, portNum);
}

} } // namespace utility::detail

