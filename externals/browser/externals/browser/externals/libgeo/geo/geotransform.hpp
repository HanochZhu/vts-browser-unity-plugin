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
 * @file geotransform.hpp
 * @author Ondrej Prochazka <ondrej.prochazka@citationtech.net>
 * 
 * Georeferencing transformation, converter classes
 */

#ifndef geo_geotransform_hpp_included
#define geo_geotransform_hpp_included

#include <array>

#include "math/geometry_core.hpp"

#include "csconvertor.hpp"

namespace geo {
  
    
/**
 * @brief A transformation defining georeferencing for a dataset
 * @details For a georeferenced dataset, geotransformation is the linear
 * transformation defining the relation between world coordinates (geographical
 * or projected) and local coordinates in the dataset (typically pixel 
 * coordinates in a raster dataset). 
 * 
 * GDAL data model defines the transformation as 6 numbers (other equivalent 
 * definitions are ESRI world files, etc). We use the same definition.
 * 
 * Note that GeoTransform has nothing to do with transformation betwen 
 * spatial reference systems, it merely georeferences a dataset.
 */    

class GeoTransform :  public std::array<double, 6> {

    friend class GeoDataset;
    
public :

    GeoTransform() {};

    /**
     * @brief The classic left-handed north up raster transformation.
     */
    static GeoTransform northUpFromExtents(
        const math::Extents2 & extents, const math::Size2 & size );
    
    /**
     * @brief Right handed, with zero at a given spot.
     */
    static GeoTransform localFromOrigin( const math::Point2 & origin ); 
    
    math::Point3 rowcol2geo( double row, double col, double value ) const;

    void geo2rowcol( const math::Point3 & gp, double & row, double & col ) const;

    math::Point3 raster2geo( math::Point2 p, double value ) const;

    template <typename T>
    T geo2raster( const math::Point3 & gp) const {
        double x, y; geo2rowcol(gp, y, x); return T(x, y);
    }

    template <typename T>
    T geo2raster(double gx, double gy, double gz = .0) const {
        double x, y; geo2rowcol({gx, gy, gz}, y, x); return T(x, y);
    }

    template <typename T>
    T geo2raster(const math::Point2 &gp) const {
        double x, y; geo2rowcol({gp(0), gp(1), .0}, y, x); return T(x, y);
    }
    
    template <typename T1 = double, typename T2>
    math::Point2_<T1> convert( const math::Point2_<T2> & p ) const {
        auto ret( rwocol2geo( p(1),p(0),.0 ) );
        return math::Point2_<T1>( ret(0), ret(1) ); 
    }
    
    template <typename T1 = double, typename T2>
    math::Point2_<T1> iconvert( const math::Point2_<T2> & gp ) const {
        double row, col;
        geo2rowcol( {gp(0),gp(1),.0}, row, col ); 
        return math::Point2_<T1>( col, row );
    }

    template <typename T1 = double, typename T2>
    math::Point3_<T1> convert( const math::Point3_<T2> & p ) const {
        auto ret( rowcol2geo( p(1), p(0), p(2) ) );
        return math::Point3_<T1>( ret(0), ret(1), ret(2) );
    }
    
    template <typename T1 = double, typename T2>
    math::Point3_<T1> iconvert( const math::Point3_<T2> & gp ) const {
        double row, col;
        geo2rowcol( {gp(0),gp(1),.0}, row, col ); 
        return math::Point3_<T1>( col, row, gp[2] );
    }

    static GeoTransform identity() {
        GeoTransform gt;
        gt[1] = gt[5] = 1.0;
        gt[0] = gt[2] = gt[3] = gt[4] = gt[2] = 0.0;
        return gt;
    }

    bool isUpright() const;

    /** Generates 4x4 matrix from undelying geo transformation. Converts raster
     *  coordinates to world coodinates.
     *
     *  Returns matrix converting to pixel registration if pixel is true,
     *  i.e. (0,0) is in the center of left-top pixel.
     *
     *  Otherwise, keeps original transformation that has (0,0) in
     *  left-top pixel's corner.
     */
    math::Matrix4 raster2geo(bool pixel = true) const;

    /** Inverse to raster2geo.
     */
    math::Matrix4 geo2raster(bool pixel = true) const;

private :
    math::Point2 applyGeoTransform( double col, double row ) const;
    void applyInvGeoTransform( 
        const math::Point2 & gp, double & col, double & row ) const;
};


/**
 * @brief Convert between local coordinates and geo coordinates in a given SRS.
 * @details Useful if you want to convert from a global reference frame to 
 *  pixel coordinates and vice versa.
 */

class GeoConverter2 {
public:
    GeoConverter2( 
        const GeoTransform & srcGeo = GeoTransform(), 
        const SrsDefinition & srcSrs = SrsDefinition::longlat(),
        const SrsDefinition & dstSrs = SrsDefinition::longlat() ) :
            srcGeo_( srcGeo ), src2dst_( srcSrs, dstSrs ),
            dst2src_( dstSrs, srcSrs ) {};
    
    GeoConverter2(
        const GeoTransform & srcGeo,
        const CsConvertor & src2dst ) :
            srcGeo_( srcGeo ), src2dst_( src2dst ), 
            dst2src_( src2dst.inverse() ) {}
            
    template <typename T1 = double, typename T2>
    math::Point2_<T1> convert( const math::Point2_<T2> & p ) const {
        auto ret( src2dst_(srcGeo_.convert<double>(p) ) );
        return math::Point2_<T1>(ret(0),ret(1));
    }
    
    template <typename T1 = double, typename T2>
    math::Point2_<T1> iconvert( const math::Point2_<T2> & gp ) const {
        return srcGeo_.iconvert<T1>( dst2src_( math::Point2(gp(0),gp(1)) ) );
    }

    template <typename T1 = double, typename T2>
    math::Point3_<T1> convert( const math::Point3_<T2> & p ) const {
        auto ret( src2dst_(srcGeo_.convert<double>(p)) );
        return math::Point3_<T1>(ret(0),ret(1),ret(2));
    }
    
    template <typename T1 = double, typename T2>
    math::Point3_<T1> iconvert( const math::Point3_<T2> & gp ) const {
        return srcGeo_.iconvert<T1>( 
            dst2src_( math::Point3(gp(0),gp(1),gp(2))) );
    }
    
private:
    GeoTransform srcGeo_;
    CsConvertor src2dst_;
    CsConvertor dst2src_;
};


class GeoConverter3 {
public:
    GeoConverter3(
        const GeoTransform & srcGeo,
        const SrsDefinition & srcSrs,
        const SrsDefinition & dstSrs,
        const GeoTransform & dstGeo );
    
    GeoConverter3(
        const GeoTransform & srcGeo,
        const CsConvertor & src2dst,
        const GeoTransform & dstGeo );
    
    template <typename T1, typename T2>
    math::Point2_<T1> convert( const math::Point2_<T2> & p ) const;
    
    template <typename T1, typename T2>
    math::Point2_<T1> iconvert( const math::Point2_<T2> & p ) const;

    template <typename T1, typename T2>
    math::Point3_<T1> convert( const math::Point3_<T2> & p ) const;
    
    template <typename T1, typename T2>
    math::Point3_<T1> iconvert( const math::Point3_<T2> & p ) const;
    
private:
    GeoTransform srcGeo_, dstGeo_;
    CsConvertor src2dst_;
};


} // namespace geo

#endif // geo_geotransform_hpp_included
