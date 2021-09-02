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

#include "tiff_io.hpp"
#include "tiff.hpp"
#include "error.hpp"
#include "cvmat.hpp"

namespace imgproc {

namespace gil = boost::gil;
namespace fs = boost::filesystem;

namespace detail {

typedef boost::shared_ptr<TIFF> Tiff;

Tiff openTiff(const fs::path &path)
{
    if (auto t = TIFFOpen(path.string().c_str(), "r")) {
        return Tiff(t, [](TIFF *t) { if (t) TIFFClose(t); });
    }

    LOGTHROW(err1, Error)
        << "Cannot open TIFF file " << path << ".";
    throw;
}

struct ImageParams {
    fs::path path;

    std::uint16_t bpp;
    std::uint16_t orientation;
    std::uint32_t width;
    std::uint32_t height;
    bool tiled;

    ImageParams(const fs::path &path)
        : path(path), bpp(8), orientation(1), width(0), height(0)
        , tiled(false)
    {}

    int cvType() const {
        switch (bpp) {
        case 8: return CV_8UC3;
        case 16: return CV_16UC3;
        }

        LOGTHROW(err1, Error)
            << "Unsupported bit field " << bpp
            << " in  TIFF file " << path << ".";
        throw;
    }

    math::Size2 dims() const {
        switch (orientation) {
        case ORIENTATION_TOPLEFT:
        case ORIENTATION_TOPRIGHT:
        case ORIENTATION_BOTRIGHT:
        case ORIENTATION_BOTLEFT:
            return { int(width), int(height) };
        }

        return { int(height), int(width) };
    }

};

ImageParams getParams(const fs::path &path)
{
    auto tiff(openTiff(path));
    ImageParams params(path);
    if (!TIFFGetField(tiff.get(), TIFFTAG_BITSPERSAMPLE, &params.bpp)) {
        params.bpp = 8;
    }

    if (!TIFFGetField(tiff.get(), TIFFTAG_ORIENTATION, &params.orientation)) {
        params.orientation = 1;
    } else {
        switch (params.orientation) {
        case ORIENTATION_TOPLEFT:
        case ORIENTATION_TOPRIGHT:
        case ORIENTATION_BOTRIGHT:
        case ORIENTATION_BOTLEFT:
        case ORIENTATION_LEFTTOP:
        case ORIENTATION_RIGHTTOP:
        case ORIENTATION_RIGHTBOT:
        case ORIENTATION_LEFTBOT:
            break;

        default:
            params.orientation = 1;
            break;
        }
    }

    if (!TIFFGetField(tiff.get(), TIFFTAG_IMAGEWIDTH, &params.width)) {
        LOGTHROW(err1, Error)
            << "Cannot get TIFF file " << path << " width.";
    }
    if (!TIFFGetField(tiff.get(), TIFFTAG_IMAGELENGTH, &params.height)) {
        LOGTHROW(err1, Error)
            << "Cannot get TIFF file " << path << " height.";
    }

    std::uint32_t tileWidth;
    params.tiled = TIFFGetField(tiff.get(), TIFFTAG_TILEWIDTH, &tileWidth);

    return params;
}

template <typename View>
void loadTiled(const ImageParams &params, View view)
{
    switch (params.orientation) {
    case ORIENTATION_TOPLEFT:
    case ORIENTATION_TOPRIGHT:
    case ORIENTATION_BOTRIGHT:
    case ORIENTATION_BOTLEFT:
        // correct
        gil::tiff_read_and_convert_view(params.path.c_str(), view);
        break;

    case ORIENTATION_LEFTTOP:
        // correct
        gil::tiff_read_and_convert_view
            (params.path.c_str()
             , gil::flipped_left_right_view(gil::rotated90cw_view(view)));
        break;

    case ORIENTATION_RIGHTTOP:
        // read does left-right flip
        gil::tiff_read_and_convert_view
            (params.path.c_str()
             , gil::flipped_left_right_view(gil::rotated90ccw_view(view)));
        break;

    case ORIENTATION_RIGHTBOT:
        // read does rot180
        gil::tiff_read_and_convert_view
            (params.path.c_str()
             , gil::flipped_left_right_view(gil::rotated90cw_view(view)));
        break;

    case ORIENTATION_LEFTBOT:
        // read does up-down flip
        gil::tiff_read_and_convert_view
            (params.path.c_str()
             , gil::flipped_up_down_view(gil::rotated90cw_view(view)));
        break;
    }
}

template <typename View>
void loadStriped(const ImageParams &params, View view)
{
    switch (params.orientation) {
    case ORIENTATION_TOPLEFT:
        gil::tiff_read_and_convert_view(params.path.c_str(), view);
        break;

    case ORIENTATION_TOPRIGHT:
        gil::tiff_read_and_convert_view
            (params.path.c_str()
             , gil::flipped_left_right_view(view));
        break;

    case ORIENTATION_BOTRIGHT:
        gil::tiff_read_and_convert_view
            (params.path.c_str(), rotated180_view(view));
        break;

    case ORIENTATION_BOTLEFT:
        gil::tiff_read_and_convert_view
            (params.path.c_str()
             , gil::flipped_up_down_view(view));
        break;

    case ORIENTATION_LEFTTOP:
        gil::tiff_read_and_convert_view
            (params.path.c_str()
             , gil::flipped_left_right_view(gil::rotated90cw_view(view)));
        break;

    case ORIENTATION_RIGHTTOP:
        gil::tiff_read_and_convert_view
            (params.path.c_str()
             , gil::rotated90ccw_view(view));
        break;

    case ORIENTATION_RIGHTBOT:
        gil::tiff_read_and_convert_view
            (params.path.c_str()
             , gil::flipped_left_right_view(gil::rotated90ccw_view(view)));
        break;

    case ORIENTATION_LEFTBOT:
        gil::tiff_read_and_convert_view
            (params.path.c_str()
             , gil::rotated90cw_view(view));
        break;
    }
}

template <typename View>
void load(const ImageParams &params, View view)
{
    if (params.tiled) {
        loadTiled(params, view);
    } else {
        loadStriped(params, view);
    }
}

} // namespace detail;

cv::Mat readTiff(const void *data, std::size_t size)
{
    (void) data;
    (void) size;
    LOGTHROW(err3, Error)
        << "Open TIFF from memory: not-implemented.";
    throw;
}

cv::Mat readTiff(const fs::path &path)
{
    const auto params(detail::getParams(path));
    const auto dims(params.dims());

    cv::Mat img(dims.height, dims.width, params.cvType());

    switch (params.bpp) {
    case 8:
        detail::load(params, imgproc::view<gil::bgr8_pixel_t>(img));
        break;

    case 16:
        detail::load(params, imgproc::view<gil::bgr16_pixel_t>(img));
        break;
    }

    return img;
}

math::Size2 tiffSize(const fs::path &path)
{
    return detail::getParams(path).dims();
}

} // namespace imgproc
