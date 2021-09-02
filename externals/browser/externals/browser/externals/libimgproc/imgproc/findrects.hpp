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
#ifndef imgproc_findrects_hpp_included_
#define imgproc_findrects_hpp_included_

#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "math/geometry_core.hpp"

namespace imgproc {

/** Finds all rectangles defined as rectangular area of the same color.
 */
template <typename PixelType>
std::vector<math::Extents2i>
findRectangles(const cv::Mat &img);

/** Finds all rectangles defined as rectangular area of the same color.
 *  Ignores areas for which filter returns false.
 */
template <typename PixelType, typename Filter>
std::vector<math::Extents2i>
findRectangles(const cv::Mat &img, Filter filter);

/** Finds all squares defined as rectangular area of the same color having the
 *  same with and height.
 */
template <typename PixelType, typename Filter>
std::vector<math::Extents2i>
findSquares(const cv::Mat &img, Filter filter);

/** Finds all squares defined as rectangular area of the same color having the
 *  same with and height.
 *  Ignores areas for which filter returns false.
 */
template <typename PixelType>
std::vector<math::Extents2i>
findSquares(const cv::Mat &img);

} // namespace imgproc

#include "detail/findrects.impl.hpp"

#endif // imgproc_findrects_hpp_included_
