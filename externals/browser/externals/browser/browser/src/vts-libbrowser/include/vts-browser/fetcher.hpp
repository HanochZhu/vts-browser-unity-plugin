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

#ifndef FETCHER_HPP_wjehfduj
#define FETCHER_HPP_wjehfduj

#include <string>
#include <memory>
#include <map>

#include "foundation.hpp"
#include "buffer.hpp"

namespace vts
{

class VTS_API FetchTask : private Immovable
{
public:
    enum class ResourceType
    {
        Undefined,
        Mapconfig,
        AuthConfig,
        BoundLayerConfig,
        FreeLayerConfig,
        TilesetMappingConfig,
        BoundMetaTile,
        MetaTile,
        Mesh,
        Texture,
        NavTile,
        Search,
        SriIndex,
        GeodataFeatures,
        GeodataStylesheet,
        Font,
    };

    struct VTS_API ExtraCodes
    {
        enum
        {
            // Timed out while waiting for data.
            Timeout = 10504,
            // Internal fetcher error.
            InternalError = 10500,
            // Content is not to be shown to the end user.
            ProhibitedContent = 10403,
            // Content is rejected to simulate errors for testing purposes.
            SimulatedError = 10000,
        };
    };

    struct VTS_API Query
    {
        std::string url;
        std::map<std::string, std::string> headers;
        ResourceType resourceType;

        explicit Query(const std::string &url, ResourceType resourceType);
    };

    struct VTS_API Reply
    {
        Buffer content;
        std::string contentType;
        std::string redirectUrl;

        // absolute time in seconds, comparable to std::time
        //   -1 = invalid value
        //   -2 = always revalidate
        sint64 expires = -1;

        // http status code, or one of the ExtraCodes
        uint32 code = 0;
    };

    Query query;
    Reply reply;

    explicit FetchTask(const Query &query);
    explicit FetchTask(const std::string &url, ResourceType resourceType);
    virtual ~FetchTask();
    virtual void fetchDone() = 0;
};

class VTS_API FetcherOptions
{
public:
    FetcherOptions();
    explicit FetcherOptions(const std::string &json);
    void applyJson(const std::string &json);
    std::string toJson() const;

    // the curl options are applied to each thread individually
    uint32 threads = 1;

    // timeout for each download, in milliseconds
    sint32 timeout = 30000;

    // create extra file with log entry for each download
    // the output is meant to be computer readable
    bool extraFileLog = false;

    // curl options
    uint32 maxHostConnections = 0;
    uint32 maxTotalConnections = 0;
    uint32 maxCacheConections = 0;

    // 0 = use http/1
    // 1 = use http/1.1
    // 2 = use http/2, fallback http/1
    // 3 = use http/2, fallback http/1.1
    sint32 pipelining = 2;
};

class VTS_API Fetcher : private Immovable
{
public:
    static std::shared_ptr<Fetcher> create(const FetcherOptions &options);

    virtual ~Fetcher();
    virtual void initialize();
    virtual void finalize();
    virtual void update();
    virtual void fetch(const std::shared_ptr<FetchTask> &) = 0;
};

} // namespace vts

#endif
