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
/**
 * @file histogram.hpp
 * @author Ondrej Prochazka <ondrej.prochazka@citationtech.net>
 *
 * Image histogram
 */

#ifndef IMGPROC_HISTOGRAM_HPP
#define IMGPROC_HISTOGRAM_HPP

#include <cmath>
#include <limits>
#include <algorithm>

#include "math/boost_gil_all.hpp"

namespace imgproc {


namespace gil = boost::gil;

namespace detail {
    template<typename T> struct numeric_limits;

    template<> struct numeric_limits<signed char> {
        static const signed char max = CHAR_MAX;
    };

    template<> struct numeric_limits<unsigned char> {
        static const unsigned char max = UCHAR_MAX;
    };

    template<> struct numeric_limits<short> {
        static const short max = SHRT_MAX;
    };

    template<> struct numeric_limits<unsigned short> {
        static const unsigned short max = USHRT_MAX;
    };
} // namespace detail

/* Obtain image histogram from a single channel view (gil based) */

template <typename View>
class Histogram {
public:
    typedef typename gil::channel_type<View>::type channel_type;
    static const channel_type max = detail::numeric_limits<channel_type>::max;

    Histogram(const View &view, channel_type lowerBound = 0
              , channel_type upperBound = max)
        : values{0}, total(0)
    {
        // TODO: work with Y channel, using Y or G so far
        const int channel((view.num_channels() == 3) ? 1 : 0);
        for (const auto &pixel : view) {
            auto value(pixel[channel]);
            if ((value >= lowerBound) && (value <= upperBound)) {
                values[value]++;
                ++total;
            }
        }
    }

    /**
     * Return the least threshold value such that given share of pixels is less
     * or equal to it
     */
    channel_type threshold( const float ratio ) const {
        const float thresholdCount = ratio * total;
        uint count = 0;
        for ( uint i = 0; i <= std::numeric_limits<channel_type>::max(); i++ )
        {
            count += values[ i ];
            if ( count >= thresholdCount ) return i;
        }
        return max;
    }

    channel_type prevalentValue() const {
        channel_type index(0);
        uint value(0);
        for (unsigned int i(0); i < (max + 1ul); ++i) {
            if (values[i] > value) {
                value = values[i];
                index = i;
            }
        }

        return index;
    }

private:
    uint values[max + 1ul];
    uint total;
};

template <typename View>
Histogram<View>
histogram(const View &v
          , typename Histogram<View>::channel_type lowerBound = 0
          , typename Histogram<View>::channel_type upperBound
          = Histogram<View>::max)
{
    return Histogram<View>(v, lowerBound, upperBound);
}

template <typename SrcView>
void stretchValues(const SrcView &src
                   , const typename gil::channel_type<SrcView>::type &lb
                   , const typename gil::channel_type<SrcView>::type &ub)
{
    typedef typename gil::channel_type<SrcView>::type channel_type;

    const float max(std::numeric_limits<channel_type>::max());
    const float fmax(max);

    // TODO: work with YUV
    for ( int i = 0; i < src.height(); i++ ) {

        typename SrcView::x_iterator sit = src.row_begin( i );

        for ( int j = 0; j < src.width(); j++ ) {

            for ( int k = 0; k < gil::num_channels<SrcView>::value; k++ ) {

                if ( (*sit)[k] < lb ) {
                    (*sit)[k] = 0;
                } else if ( (*sit)[k] > ub ) {
                    (*sit)[k] = max;
                } else {
                    (*sit)[k] = channel_type((fmax * ((*sit)[k] - lb))
                                             / ( ub - lb ));
                }
            }

            ++sit;
        }
    }
}


} // namespace imgproc

#endif // IMGPROC_HISTOGRAM_HPP

