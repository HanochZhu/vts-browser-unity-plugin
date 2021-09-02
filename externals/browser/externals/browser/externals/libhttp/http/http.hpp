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
#ifndef http_http_hpp_included_
#define http_http_hpp_included_

#include <memory>
#include <string>

#include "utility/tcpendpoint.hpp"

#include "contentgenerator.hpp"
#include "contentfetcher.hpp"

namespace http {

class Http {
public:
    /** Simple server-side interface: listen at given endpoint and start
     *  machinery right away.
     *
     *  NB: contentGenerator must survive this object
     */
    Http(const utility::TcpEndpoint &listen
         , unsigned int threadCount
         , ContentGenerator &contentGenerator);

    /** Create HTTP machiner and do nothing.
     */
    Http();

    /** Listen at given endpoint and as content generator to generate replies to
     *  received requests.
     *  Returned value can be different if port was 0 (i.e. listen at any
     *  available port)
     *
     * \param listen address where to listen
     * \param contentGenerator request handler
     * \return real listening endpoint
     */
    utility::TcpEndpoint
    listen(const utility::TcpEndpoint &listen
           , const ContentGenerator::pointer &contentGenerator);

    /** Listen at given endpoint and as content generator to generate replies to
     *  received requests.
     *
     *  NB: contentGenerator must survive this object
     *
     *  Returned value can be different if port was 0 (i.e. listen at any
     *  available port)
     *
     * \param listen address where to listen
     * \param contentGenerator request handler
     * \return real listening endpoint
     */
    utility::TcpEndpoint listen(const utility::TcpEndpoint &listen
                                , ContentGenerator &contentGenerator);

    /** Start server-side processing machinery.
     */
    void startServer(unsigned int threadCount);

    /** Start client-side processing machinery.
     */
    void startClient(unsigned int threadCount,
                     const ContentFetcher::Options *options = nullptr);

    /** Stop processing machinery.
     */
    void stop();

    /** Set server header.
     */
    void serverHeader(const std::string &value);

    /** Returns content fetcher interface.
     */
    ContentFetcher& fetcher();

    void stat(std::ostream &os) const;

    class Detail;
    friend class Detail;

private:
    std::shared_ptr<Detail> detail_;
    Detail& detail() { return *detail_; }
    const Detail& detail() const { return *detail_; }
};

} // namespace http

#endif // http_http_hpp_included_
