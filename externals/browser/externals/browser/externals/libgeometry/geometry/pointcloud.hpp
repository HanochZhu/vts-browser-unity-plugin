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
 * @file pointcloud.hpp
 * @author Ondrej Prochazka <ondrej.prochazka@citationtech.net>
 *
 * Point clouds, or set of 3D points (usually surface boundary samples).
 */

#ifndef GEOMETRY_POINTCLOUD_HPP
#define GEOMETRY_POINTCLOUD_HPP

#include <math/geometry.hpp>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>

#include <set>

namespace ublas = boost::numeric::ublas;

namespace geometry {

/**
 * PointCloud is essentially a std::vector of 3D points, with extents
 * maintenance and sampling density computation.
 */

class PointCloud: public std::vector<math::Point3> {

public :

    /** Initialize */
    PointCloud() {};

    PointCloud( const std::vector<math::Point3> & in ) {
        assign(in.begin(), in.end());
    }

    void assign( const std::vector<math::Point3> & in ) {
        assign(in.begin(), in.end());
    }

    template <class InputIterator>
    void assign(InputIterator first, InputIterator last);

    /** push_back */
    void push_back ( const math::Point3 & x );

    /** insert */
    iterator insert ( iterator position, const math::Point3 & x );

    /** insert */
    void insert ( iterator position, size_type n,
                  const math::Point3 & x );

    /** insert */
    template <class InputIterator>
    void insert ( iterator position, InputIterator first, InputIterator last );

    /** clear */
    void clear();

    /**
     * Save to a file. The format is simplistic, with one line per point,
     * three whitespace separated values per line.
     */
    void dump( const std::string & path );

    /**
     * Load a file saved with "dump".
     */
    void load( const std::string & path );

    /**
     * Return a measure of euclidian distance to a nearest point.
     * Sampling delta * 100 % point are at most as far from a nearest point as
     * the return value.
     */
    double samplingDelta ( float bulkThreshold = 0.5 ) const;

    /** Upper bound of all points */
    math::Point3 upper() const { assert( ! empty() ); return extents_.ur; }

    /** Upper bound of all points. */
    math::Point3 lower() const { assert( ! empty() ); return extents_.ll; }

    /** Extents of all points. */
    math::Extents3 extents() const { assert( ! empty() ); return extents_; }

    /* trasnform pointcloud via matrix4 */
    template <class Matrix>
    PointCloud transform( const Matrix & trafo ) const;

    void swap(PointCloud &other);

    void swap(std::vector<math::Point3> &other);

    const std::vector<math::Point3>& asVector() const {
        return *this;
    }

private :

    /* forbidden modifiers */
    void pop_back ( ) { assert( false); }
    iterator erase ( iterator position ) {
        (void) position; assert( false ); return position; }
    iterator erase ( iterator first, iterator last ) {
        (void) first; (void) last;
        assert( false ); return first;
    }
    void swap( std::vector<ublas::vector< double> >& vec ) {
        (void) vec;
        assert( false ); }

    void updateExtents( const math::Point3 & x );

    class ThreeDistance {

    public :

        ThreeDistance( double value = 0.0 )
            : distX( value ), distY( value ), distZ( value ) {}

        void update( const math::Point3 diff );

        double value() const;

        bool operator < ( const ThreeDistance & op ) const {

            return value() < op.value();
        }
    private :

        double distX, distY, distZ;
    };

    math::Extents3 extents_;
};


template <typename E, typename T>
std::basic_ostream<E, T> & operator << (
    std::basic_ostream<E,T> & os,
    const PointCloud & cloud );


/* template method implementation */

template <class InputIterator>
void PointCloud::assign(InputIterator first, InputIterator last)
{
    std::vector<math::Point3>::assign(first, last);

    extents_ = math::computeExtents(first, last);
}

template <class InputIterator>
inline void PointCloud::insert ( iterator position, InputIterator first,
                            InputIterator last ) {

    for ( InputIterator it = first; it < last; it++ )
        updateExtents( *it );

    std::vector<math::Point3>::insert( position, first, last );
}


template <class Matrix>
inline PointCloud PointCloud::transform( const Matrix & trafo ) const {

    PointCloud retval;

    for(const auto &point : *this) {
        retval.push_back( math::euclidian(
            prod( trafo, math::homogeneous( point ) ) ) );
    }

    return retval;
}

inline void PointCloud::swap(PointCloud &other)
{
    std::swap(static_cast<std::vector<math::Point3>&>(*this)
              , static_cast<std::vector<math::Point3>&>(other));
    std::swap(extents_, other.extents_);
}

inline void PointCloud::swap(std::vector<math::Point3> &other)
{
    std::swap(static_cast<std::vector<math::Point3>&>(*this), other);

    extents_ = math::computeExtents(begin(), end());
}


template <typename E, typename T>
std::basic_ostream<E, T> & operator << (
    std::basic_ostream<E,T> & os, const PointCloud & cloud ) {

    BOOST_FOREACH( math::Point3 point, cloud )
        os << point(0) << "\t" << point(1) << "\t" << point(2) << "\n";
        
    return os;
}


} // namespace geometry

#endif
