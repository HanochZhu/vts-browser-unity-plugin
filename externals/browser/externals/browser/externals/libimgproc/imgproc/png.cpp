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

#include <cstdio>
#include <memory>
#include <algorithm>
#include <system_error>

#include <png.h>

#include "dbglog/dbglog.hpp"

#include "utility/binaryio.hpp"

#include "png.hpp"
#include "error.hpp"

namespace fs = boost::filesystem;
namespace bin = utility::binaryio;

namespace imgproc { namespace png {

namespace {

extern "C" {

void imgproc_PngWrite(::png_structp png, ::png_bytep data, ::png_size_t length)
{
    auto &out(*static_cast<SerializedPng*>(::png_get_io_ptr(png)));
    out.insert(out.end(), data, data + length);
}

void imgproc_PngFlush(::png_structp) {}

} // extern "C"

class PngWriter {
public:
    PngWriter(SerializedPng &out)
        : png_(::png_create_write_struct
               (PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr))
        , info_()
    {
        if (!png_) {
            LOGTHROW(err1, Error)
                << "Unable to initialize PNG writer.";
        }
        info_ = ::png_create_info_struct(png_);

        ::png_set_write_fn(png_
                           , &out
                           , &imgproc_PngWrite
                           , imgproc_PngFlush);
    }

    PngWriter(FILE *out)
        : png_(::png_create_write_struct
               (PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr))
        , info_()
    {
        if (!png_) {
            LOGTHROW(err1, Error)
                << "Unable to initialize PNG writer.";
        }
        info_ = ::png_create_info_struct(png_);

        ::png_init_io(png_, out);
    }

    ~PngWriter() {
        png_destroy_write_struct(&png_, &info_);
    }

    ::png_structp png() { return png_; }
    ::png_infop info() { return info_; }

private:
    ::png_structp png_;
    ::png_infop info_;
};

template <typename PixelType, typename ConstView>
void writeView(PngWriter &writer, const ConstView &view
               , int compressionLevel, int type)
{
    ::png_set_IHDR(writer.png(), writer.info()
                   , png_uint_32(view.width()), png_uint_32(view.height())
                   , 8, type, PNG_INTERLACE_NONE
                   , PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    if ((compressionLevel >= 0) && (compressionLevel <= 9)) {
        ::png_set_compression_level(writer.png(), compressionLevel);
    }

    ::png_write_info(writer.png(), writer.info());

    std::vector<PixelType> row(view.width());

    // copy row by row
    for(int y(0), ey(int(view.height())); y != ey; ++y) {
        std::copy(view.row_begin(y), view.row_end(y), row.begin());
        ::png_write_row(writer.png()
                        , reinterpret_cast< ::png_bytep>(&row.front()));
    }

    ::png_write_end(writer.png(), writer.info());
}

template <typename PixelType, typename ConstView>
SerializedPng serializeView(const ConstView &view, int compressionLevel
                            , int type)
{
    SerializedPng out;
    PngWriter writer(out);
    writeView<PixelType>(writer, view, compressionLevel, type);
    return out;
}

template <typename PixelType, typename ConstView>
void writeViewToFile(const fs::path &path, const ConstView &view
                     , int compressionLevel, int type)
{
    struct FileHolder {
        FileHolder(const fs::path &path)
            : path(path), f(std::fopen(path.string().c_str(), "w"))
        {}

        ~FileHolder() {
            if (!f) { return; }
            if (std::fclose(f)) {
                std::system_error e(errno, std::system_category());
                LOG(warn3) << "Cannot close PNG file " << path << ": <"
                           << e.code() << ", " << e.what() << ">.";
            }
        }

        const fs::path path;
        std::FILE *f;
    };

    FileHolder fh(path);
    if (!fh.f) {
        std::system_error e(errno, std::system_category());
        LOG(err3) << "Cannot create PNG file " << path << ": <"
                   << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    PngWriter writer(fh.f);
    writeView<PixelType>(writer, view, compressionLevel, type);
}

} // namespace

SerializedPng serialize(const boost::gil::gray8_image_t &image
                        , int compressionLevel)
{
    return serializeView<boost::gil::gray8_pixel_t>
        (boost::gil::const_view(image), compressionLevel, PNG_COLOR_TYPE_GRAY);
}

void write(const fs::path &file
           , const boost::gil::gray8_image_t &image
           , int compressionLevel)
{
    return writeViewToFile<boost::gil::gray8_pixel_t>
        (file, boost::gil::const_view(image)
         , compressionLevel, PNG_COLOR_TYPE_GRAY);
}

SerializedPng serialize(const boost::gil::rgb8_image_t &image
                        , int compressionLevel)
{
    return serializeView<boost::gil::rgb8_pixel_t>
        (boost::gil::const_view(image), compressionLevel, PNG_COLOR_TYPE_RGB);
}

void write(const fs::path &file
           , const boost::gil::rgb8_image_t &image
           , int compressionLevel)
{
    return writeViewToFile<boost::gil::rgb8_pixel_t>
        (file, boost::gil::const_view(image)
         , compressionLevel, PNG_COLOR_TYPE_RGB);
}

SerializedPng serialize(const boost::gil::rgba8_image_t &image
                        , int compressionLevel)
{
    return serializeView<boost::gil::rgba8_pixel_t>
        (boost::gil::const_view(image), compressionLevel, PNG_COLOR_TYPE_RGBA);
}

SerializedPng serialize(const void *data, std::size_t length
                        , const math::Size2 &size, RawFormat format
                        , int compressionLevel)
{
    auto checkLength([&](int components)
    {
        if ((math::area(size) * components) != int(length)) {
            LOGTHROW(err1, Error)
                << "Cannot serialize raw data to PNG: wrong lenght "
                << length << "; should be " << (math::area(size) * components)
                << ".";
        }
    });
    switch (format) {
    case RawFormat::gray:
        checkLength(1);
        return serializeView<boost::gil::gray8_pixel_t>
            (boost::gil::interleaved_view
             (size.width, size.height
              , static_cast<const boost::gil::gray8_pixel_t*>(data)
              , size.width)
             , compressionLevel, PNG_COLOR_TYPE_GRAY);

    case RawFormat::rgb:
        checkLength(3);
        return serializeView<boost::gil::rgb8_pixel_t>
            (boost::gil::interleaved_view
             (size.width, size.height
              , static_cast<const boost::gil::rgb8_pixel_t*>(data)
              , 3 * size.width)
             , compressionLevel, PNG_COLOR_TYPE_RGB);

    case RawFormat::rgba:
        checkLength(4);
        return serializeView<boost::gil::rgba8_pixel_t>
            (boost::gil::interleaved_view
             (size.width, size.height
              , static_cast<const boost::gil::rgba8_pixel_t*>(data)
              , 4 * size.width)
             , compressionLevel, PNG_COLOR_TYPE_RGBA);
    }

    LOGTHROW(err1, Error)
        << "Unsupported raw format <" << static_cast<int>(format)
        << "> for PNG serialization.";
    throw;
}

void write(const fs::path &file
           , const boost::gil::rgba8_image_t &image
           , int compressionLevel)
{
    return writeViewToFile<boost::gil::rgba8_pixel_t>
        (file, boost::gil::const_view(image)
         , compressionLevel, PNG_COLOR_TYPE_RGBA);
}

} } // namespace imgproc::png
