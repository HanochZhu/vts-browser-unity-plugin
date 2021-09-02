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
#ifndef http_sink_hpp_included_
#define http_sink_hpp_included_

#include <ctime>
#include <string>
#include <vector>
#include <memory>
#include <exception>
#include <iosfwd>

#include <boost/optional.hpp>

#include "error.hpp"
#include "request.hpp"

namespace http {

/** Sink for sending/receiving data to/from the client.
 */
class SinkBase {
public:
    typedef std::shared_ptr<SinkBase> pointer;

    SinkBase() {}
    virtual ~SinkBase() {}

    struct CacheControl  {
        /** File max age. <0: no-cache, >=0 max-age=value
         */
        boost::optional<long> maxAge;

        /** File stale-while-revalidated value. Used only when
         *  >0 and max-age >= 0.
         *
         *  More info: https://tools.ietf.org/html/rfc5861#section-3
         */
        long staleWhileRevalidate;

        CacheControl(const boost::optional<long> &maxAge = boost::none
                     , long staleWhileRevalidate = 0)
            : maxAge(maxAge), staleWhileRevalidate(staleWhileRevalidate)
        {}

        operator bool() const { return bool(maxAge); }
    };

    struct FileInfo {
        /** File content type.
         */
        std::string contentType;

        /** Timestamp of last modification. -1 means now.
         */
        std::time_t lastModified;

        /** Cache control header data.
         */
        CacheControl cacheControl;

        FileInfo(const std::string &contentType = "application/octet-stream"
                 , std::time_t lastModified = -1
                 , const CacheControl &cacheControl = CacheControl())
            : contentType(contentType), lastModified(lastModified)
            , cacheControl(cacheControl)
        {}

        FileInfo(const std::string &contentType
                 , std::time_t lastModified, long maxAge)
            : contentType(contentType), lastModified(lastModified)
            , cacheControl(maxAge)
        {}
    };

    class DataSource {
    public:
        typedef std::shared_ptr<DataSource> pointer;

        DataSource(bool hasContentLength = true)
            : hasContentLength_(hasContentLength)
        {}

        virtual ~DataSource() {}

        virtual FileInfo stat() const = 0;

        virtual std::size_t read(char *buf, std::size_t size
                                 , std::size_t off) = 0;

        virtual std::string name() const { return "unknown"; }

        virtual void close() const {}

        /** Returns size of response.
         *  >=0: exact length, use content-length
         *  <0: unknown, use content-transfer-encoding chunked
         */
        virtual long size() const = 0;

        /** Additional headers sent to output.
         */
        virtual const Header::list *headers() const { return nullptr; }

        bool hasContentLength() const { return hasContentLength_; }

    private:
        bool hasContentLength_;
    };

    /** Sends content to client.
     * \param data data top send
     * \param stat file info (size is ignored)
     * \param headers additional (optional) headers
     */
    void content(const std::string &data, const FileInfo &stat
                 , const Header::list *headers = nullptr);

    /** Sends content to client.
     * \param data data top send
     * \param stat file info (size is ignored)
     * \param headers additional (optional) headers
     */
    template <typename T>
    void content(const std::vector<T> &data, const FileInfo &stat
                 , const Header::list *headers = nullptr);

    /** Sends content to client.
     * \param data data top send
     * \param size size of data
     * \param stat file info (size is ignored)
     * \param needCopy data are copied if set to true
     * \param headers additional (optional) headers
     */
    void content(const void *data, std::size_t size
                 , const FileInfo &stat, bool needCopy
                 , const Header::list *headers = nullptr);

    /** Sends current exception to the client.
     */
    void error();

    /** Sends given error to the client.
     */
    void error(const std::exception_ptr &exc);

    /** Sends given error to the client.
     */
    void error(const std::error_code &ec, const std::string &message = "");

    /** Shortcut for http errors.
     */
    void error(const HttpError &exc);

    /** Tell client to look somewhere else.
     *
     * \param url location where to redirect
     * \param code HTTP code (302, 303, ...)
     */
    void redirect(const std::string &url, utility::HttpCode code);

    /** Tell client to look somewhere else with caching information.
     *
     * \param url location where to redirect
     * \param code HTTP code (302, 303, ...)
     * \param cacheControl cache-control header
     */
    void redirect(const std::string &url, utility::HttpCode code
                  , const CacheControl &cacheControl);

    /** Sends given error to the client.
     */
    template <typename T> void error(const T &exc);

protected: // needed do to -Wvirtual-override
    virtual void content_impl(const void *data, std::size_t size
                              , const FileInfo &stat, bool needCopy
                              , const Header::list *headers) = 0;
    virtual void error_impl(const std::exception_ptr &exc) = 0;
    virtual void error_impl(const std::error_code &ec
                            , const std::string &message) = 0;
    virtual void redirect_impl(const std::string &url, utility::HttpCode
                               , const CacheControl &cacheControl) = 0;
};

/** Sink for sending/receiving data to/from the client.
 */
class ServerSink : public SinkBase {
public:
    typedef std::shared_ptr<ServerSink> pointer;
    typedef std::function<void()> AbortedCallback;

    struct ListingItem {
        enum class Type { dir, file };

        std::string name;
        Type type;

        ListingItem(const std::string &name = "", Type type = Type::file)
            : name(name), type(type)
        {}

        bool operator<(const ListingItem &o) const;
    };

    typedef std::vector<ListingItem> Listing;

    ServerSink() {}

    /** Generates listing.
     */
    void listing(const Listing &list, const std::string &header = ""
                 , const std::string &footer = "");

    /** Checks wheter client aborted request.
     *  Throws RequestAborted exception when true.
     */
    void checkAborted() const;

    /** Sets aborted callback.
     */
    void setAborter(const AbortedCallback &ac);

    /** Pull in base class content(...) functions
     */
    using SinkBase::content;

    /** Sends content to client.
     * \param stream stream to send
     */
    void content(const DataSource::pointer &source);

private:
    using SinkBase::content_impl; // needed do to -Wvirtual-override
    virtual void content_impl(const DataSource::pointer &source) = 0;
    virtual void listing_impl(const Listing &list, const std::string &header
                              , const std::string &footer) = 0;
    virtual bool checkAborted_impl() const = 0;
    virtual void setAborter_impl(const AbortedCallback &ac) = 0;
};

class ClientSink : public SinkBase {
public:
    typedef std::shared_ptr<ClientSink> pointer;

    ClientSink() {}

    /** Content has not been modified. Called only when asked.
     *  Default implementation calls error(NotModified());
     */
    void notModified();

private:
    virtual void notModified_impl();
};

// inlines

inline void SinkBase::content(const std::string &data, const FileInfo &stat
                              , const Header::list *headers)
{
    content_impl(data.data(), data.size(), stat, true, headers);
}

inline void SinkBase::content(const void *data, std::size_t size
                              , const FileInfo &stat, bool needCopy
                              , const Header::list *headers)
{
    content_impl(data, size, stat, needCopy, headers);
}

template <typename T>
inline void SinkBase::content(const std::vector<T> &data, const FileInfo &stat
                              , const Header::list *headers)
{
    content_impl(data.data(), data.size() * sizeof(T), stat, true, headers);
}

inline void ServerSink::content(const DataSource::pointer &source)
{
    content_impl(source);
}

inline void SinkBase::redirect(const std::string &url, utility::HttpCode code)
{
    redirect_impl(url, code, CacheControl());
}

inline void SinkBase::redirect(const std::string &url, utility::HttpCode code
                               , const CacheControl &cacheControl)
{
    redirect_impl(url, code, cacheControl);
}

inline void SinkBase::error(const std::exception_ptr &exc)
{
    error_impl(exc);
}

inline void SinkBase::error(const std::error_code &ec
                            , const std::string &message)
{
    error_impl(ec, message);
}

inline void SinkBase::error(const HttpError &exc)
{
    error_impl(exc.code(), exc.what());
}

template <typename T> inline void SinkBase::error(const T &exc) {
    error_impl(std::make_exception_ptr(exc));
}

inline void ServerSink::listing(const Listing &list
                                , const std::string &header
                                , const std::string &footer)
{
    listing_impl(list, header, footer);
}

inline void ServerSink::setAborter(const AbortedCallback &ac) {
    setAborter_impl(ac);
}

inline bool ServerSink::ListingItem::operator<(const ListingItem &o) const
{
    if (type < o.type) { return true; }
    if (o.type < type) { return false; }
    return name < o.name;
}

inline void ClientSink::notModified() { notModified_impl(); }

inline void ClientSink::notModified_impl() {
    error(make_error_code(utility::HttpCode::NotModified));
}

} // namespace http

#endif // http_sink_hpp_included_
