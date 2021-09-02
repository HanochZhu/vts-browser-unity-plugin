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
// *****************************************************************************
/*
 * ANSI C code from the article
 * "Contrast Limited Adaptive Histogram Equalization"
 * by Karel Zuiderveld, karel@cv.ruu.nl
 * in "Graphics Gems IV", Academic Press, 1994
 *
 *
 *  These functions implement Contrast Limited Adaptive Histogram Equalization.
 *  The main routine (CLAHE) expects an input image that is stored contiguously in
 *  memory;  the CLAHE output image overwrites the original input image and has the
 *  same minimum and maximum values (which must be provided by the user).
 *  This implementation assumes that the X- and Y image resolutions are an integer
 *  multiple of the X- and Y sizes of the contextual regions. A check on various other
 *  error conditions is performed.
 *
 *  #define the symbol BYTE_IMAGE to make this implementation suitable for
 *  8-bit images. The maximum number of contextual regions can be redefined
 *  by changing uiMAX_REG_X and/or uiMAX_REG_Y; the use of more than 256
 *  contextual regions is not recommended.
 *
 *  The code is ANSI-C and is also C++ compliant.
 *
 *  Author: Karel Zuiderveld, Computer Vision Research Group,
 *           Utrecht, The Netherlands (karel@cv.ruu.nl)
 */

/*

EULA: The Graphics Gems code is copyright-protected. In other words, you cannot
claim the text of the code as your own and resell it. Using the code is permitted
in any program, product, or library, non-commercial or commercial. Giving credit
is not required, though is a nice gesture. The code comes as-is, and if there are
any flaws or problems with any Gems code, nobody involved with Gems - authors,
editors, publishers, or webmasters - are to be held responsible. Basically,
don't be a jerk, and remember that anything free comes with no guarantee.

- http://tog.acm.org/resources/GraphicsGems/ (August 2009)

*/

#include <cstring>
#include <limits>
#include <iostream>

namespace imgproc {

namespace detail {

/*********************** Local prototypes ************************/
static void ClipHistogram (unsigned long*, unsigned int, unsigned long);

template <class kz_pixel_t>
static void MakeHistogram (kz_pixel_t*, unsigned int, unsigned int, unsigned int,
                unsigned long*, unsigned int, kz_pixel_t*);

template <class kz_pixel_t>
static void MapHistogram (unsigned long*, kz_pixel_t, kz_pixel_t,
               unsigned int, unsigned long);

template <class kz_pixel_t>
static void MakeLut (kz_pixel_t*, kz_pixel_t, kz_pixel_t, unsigned int);

template <class kz_pixel_t>
static void Interpolate (kz_pixel_t*, int, unsigned long*, unsigned long*,
        unsigned long*, unsigned long*, unsigned int, unsigned int, kz_pixel_t*);

// *****************************************************************************

/**************  Start of actual code **************/
#include <stdlib.h>                      /* To get prototypes of malloc() and free() */

const static unsigned int uiMAX_REG_X = 1024;      /* max. # contextual regions in x-direction */
const static unsigned int uiMAX_REG_Y = 1024;      /* max. # contextual regions in y-direction */

/************************** main function CLAHE ******************/


template <class kz_pixel_t>
static int CLAHE (kz_pixel_t* pImage, unsigned int uiXRes, unsigned int uiYRes,
         kz_pixel_t Min, kz_pixel_t Max, unsigned int uiNrX, unsigned int uiNrY,
              unsigned int uiNrBins, float fCliplimit)
/*   pImage - Pointer to the input/output image
 *   uiXRes - Image resolution in the X direction
 *   uiYRes - Image resolution in the Y direction
 *   Min - Minimum greyvalue of input image (also becomes minimum of output image)
 *   Max - Maximum greyvalue of input image (also becomes maximum of output image)
 *   uiNrX - Number of contextial regions in the X direction (min 2, max uiMAX_REG_X)
 *   uiNrY - Number of contextial regions in the Y direction (min 2, max uiMAX_REG_Y)
 *   uiNrBins - Number of greybins for histogram ("dynamic range")
 *   float fCliplimit - Normalized cliplimit (higher values give more contrast)
 * The number of "effective" greylevels in the output image is set by uiNrBins; selecting
 * a small value (eg. 128) speeds up processing and still produce an output image of
 * good quality. The output image will have the same minimum and maximum value as the input
 * image. A clip limit smaller than 1 results in standard (non-contrast limited) AHE.
 */
{
    unsigned int uiX, uiY;                /* counters */
    unsigned int uiXSize, uiYSize, uiSubX, uiSubY; /* size of context. reg. and subimages */
    unsigned int uiXL, uiXR, uiYU, uiYB;  /* auxiliary variables interpolation routine */
    unsigned long ulClipLimit, ulNrPixels;/* clip limit and region pixel count */
    kz_pixel_t* pImPointer;                /* pointer to image */
    kz_pixel_t aLUT[ std::numeric_limits<kz_pixel_t>::max() ];
                                          /* lookup table used for scaling of input image */
    unsigned long* pulHist, *pulMapArray; /* pointer to histogram and mappings*/
    unsigned long* pulLU, *pulLB, *pulRU, *pulRB; /* auxiliary pointers interpolation */

    if (uiNrX > uiMAX_REG_X) return -1;    /* # of regions x-direction too large */
    if (uiNrY > uiMAX_REG_Y) return -2;    /* # of regions y-direction too large */
    if (uiXRes % uiNrX) return -3;        /* x-resolution no multiple of uiNrX */
    if (uiYRes % uiNrY) return -4;        /* y-resolution no multiple of uiNrY #TPB FIX */
    if (Min >= Max) return -6;            /* minimum equal or larger than maximum */
    if (uiNrX < 2 || uiNrY < 2) return -7;/* at least 4 contextual regions required */
    if (fCliplimit == 1.0) return 0;      /* is OK, immediately returns original image. */
    if (uiNrBins == 0) uiNrBins = 128;    /* default value when not specified */

    pulMapArray=(unsigned long *)malloc(sizeof(unsigned long)*uiNrX*uiNrY*uiNrBins);
    if (pulMapArray == 0) return -8;      /* Not enough memory! (try reducing uiNrBins) */

    uiXSize = uiXRes/uiNrX; uiYSize = uiYRes/uiNrY;  /* Actual size of contextual regions */
    ulNrPixels = (unsigned long)uiXSize * (unsigned long)uiYSize;

    if(fCliplimit > 0.0) {                /* Calculate actual cliplimit  */
       ulClipLimit = (unsigned long) (fCliplimit * (uiXSize * uiYSize) / uiNrBins);
       ulClipLimit = (ulClipLimit < 1UL) ? 1UL : ulClipLimit;
    }
    else ulClipLimit = 1UL<<14;           /* Large value, do not clip (AHE) */
    MakeLut<kz_pixel_t>(aLUT, Min, Max, uiNrBins);    /* Make lookup table for mapping of greyvalues */
    /* Calculate greylevel mappings for each contextual region */
    for (uiY = 0, pImPointer = pImage; uiY < uiNrY; uiY++) {
        for (uiX = 0; uiX < uiNrX; uiX++, pImPointer += uiXSize) {
            pulHist = &pulMapArray[uiNrBins * (uiY * uiNrX + uiX)];
            MakeHistogram<kz_pixel_t>(pImPointer,uiXRes,uiXSize,uiYSize,pulHist,uiNrBins,aLUT);
            ClipHistogram(pulHist, uiNrBins, ulClipLimit);
            MapHistogram<kz_pixel_t>(pulHist, Min, Max, uiNrBins, ulNrPixels);
        }
        pImPointer += (uiYSize - 1) * uiXRes;             /* skip lines, set pointer */
    }

    /* Interpolate greylevel mappings to get CLAHE image */
    for (pImPointer = pImage, uiY = 0; uiY <= uiNrY; uiY++) {
        if (uiY == 0) {                                   /* special case: top row */
            uiSubY = uiYSize >> 1;  uiYU = 0; uiYB = 0;
        }
        else {
            if (uiY == uiNrY) {                           /* special case: bottom row */
                uiSubY = uiYSize >> 1;  uiYU = uiNrY-1;  uiYB = uiYU;
            }
            else {                                        /* default values */
                uiSubY = uiYSize; uiYU = uiY - 1; uiYB = uiYU + 1;
            }
        }
        for (uiX = 0; uiX <= uiNrX; uiX++) {
            if (uiX == 0) {                               /* special case: left column */
                uiSubX = uiXSize >> 1; uiXL = 0; uiXR = 0;
            }
            else {
                if (uiX == uiNrX) {                       /* special case: right column */
                    uiSubX = uiXSize >> 1;  uiXL = uiNrX - 1; uiXR = uiXL;
                }
                else {                                    /* default values */
                    uiSubX = uiXSize; uiXL = uiX - 1; uiXR = uiXL + 1;
                }
            }

            pulLU = &pulMapArray[uiNrBins * (uiYU * uiNrX + uiXL)];
            pulRU = &pulMapArray[uiNrBins * (uiYU * uiNrX + uiXR)];
            pulLB = &pulMapArray[uiNrBins * (uiYB * uiNrX + uiXL)];
            pulRB = &pulMapArray[uiNrBins * (uiYB * uiNrX + uiXR)];
            Interpolate<kz_pixel_t>(pImPointer,uiXRes,pulLU,pulRU,pulLB,pulRB,uiSubX,uiSubY,aLUT);
            pImPointer += uiSubX;                         /* set pointer on next matrix */
        }
        pImPointer += (uiSubY - 1) * uiXRes;
    }
    free(pulMapArray);                                    /* free space for histograms */
    return 0;                                             /* return status OK */
}

inline void ClipHistogram (unsigned long* pulHistogram, unsigned int
                     uiNrGreylevels, unsigned long ulClipLimit)
/* This function performs clipping of the histogram and redistribution of bins.
 * The histogram is clipped and the number of excess pixels is counted. Afterwards
 * the excess pixels are equally redistributed across the whole histogram (providing
 * the bin count is smaller than the cliplimit).
 */
{
    unsigned long* pulBinPointer, *pulEndPointer, *pulHisto;
    unsigned long ulNrExcess, ulUpper, ulBinIncr, ulStepSize, i;
    unsigned long ulOldNrExcess;  // #IAC Modification

    long lBinExcess;

    ulNrExcess = 0;  pulBinPointer = pulHistogram;
    for (i = 0; i < uiNrGreylevels; i++) { /* calculate total number of excess pixels */
        lBinExcess = (long) pulBinPointer[i] - (long) ulClipLimit;
        if (lBinExcess > 0) ulNrExcess += lBinExcess;     /* excess in current bin */
    };

    /* Second part: clip histogram and redistribute excess pixels in each bin */
    ulBinIncr = ulNrExcess / uiNrGreylevels;              /* average binincrement */
    ulUpper =  ulClipLimit - ulBinIncr;  /* Bins larger than ulUpper set to cliplimit */

    for (i = 0; i < uiNrGreylevels; i++) {
      if (pulHistogram[i] > ulClipLimit) pulHistogram[i] = ulClipLimit; /* clip bin */
      else {
          if (pulHistogram[i] > ulUpper) {              /* high bin count */
              ulNrExcess -= pulHistogram[i] - ulUpper; pulHistogram[i]=ulClipLimit;
          }
          else {                                        /* low bin count */
              ulNrExcess -= ulBinIncr; pulHistogram[i] += ulBinIncr;
          }
       }
    }

    // while (ulNrExcess) {   /* Redistribute remaining excess  */
    //    pulEndPointer = &pulHistogram[uiNrGreylevels]; pulHisto = pulHistogram;
    //
    //    while (ulNrExcess && pulHisto < pulEndPointer) {
    //        ulStepSize = uiNrGreylevels / ulNrExcess;
    //        if (ulStepSize < 1) ulStepSize = 1;           /* stepsize at least 1 */
    //        for (pulBinPointer=pulHisto; pulBinPointer < pulEndPointer && ulNrExcess;
    //             pulBinPointer += ulStepSize) {
    //            if (*pulBinPointer < ulClipLimit) {
    //                (*pulBinPointer)++;  ulNrExcess--;    /* reduce excess */
    //            }
    //        }
    //        pulHisto++;           /* restart redistributing on other bin location */
    //    }
    //}

/* ####

       IAC Modification:
       In the original version of the loop below (commented out above) it was possible for an infinite loop to get
       created.  If there was more pixels to be redistributed than available space then the
       while loop would never end.  This problem has been fixed by stopping the loop when all
       pixels have been redistributed OR when no pixels where redistributed in the previous iteration.
       This change allows very low clipping levels to be used.

*/

     do {   /* Redistribute remaining excess  */
         pulEndPointer = &pulHistogram[uiNrGreylevels]; pulHisto = pulHistogram;

         ulOldNrExcess = ulNrExcess;     /* Store number of excess pixels for test later. */

         while (ulNrExcess && pulHisto < pulEndPointer)
         {
             ulStepSize = uiNrGreylevels / ulNrExcess;
             if (ulStepSize < 1)
                 ulStepSize = 1;       /* stepsize at least 1 */
             for (pulBinPointer=pulHisto; pulBinPointer < pulEndPointer && ulNrExcess;
             pulBinPointer += ulStepSize)
             {
                 if (*pulBinPointer < ulClipLimit)
                 {
                     (*pulBinPointer)++;  ulNrExcess--;    /* reduce excess */
                 }
            }
             pulHisto++;       /* restart redistributing on other bin location */
         }
     } while ((ulNrExcess) && (ulNrExcess < ulOldNrExcess));
     /* Finish loop when we have no more pixels or we can't redistribute any more pixels */


}

template <class kz_pixel_t>
void MakeHistogram (kz_pixel_t* pImage, unsigned int uiXRes,
     unsigned int uiSizeX, unsigned int uiSizeY,
     unsigned long* pulHistogram,
     unsigned int uiNrGreylevels, kz_pixel_t* pLookupTable)
    /* This function classifies the greylevels present in the array image into
     * a greylevel histogram. The pLookupTable specifies the relationship
     * between the greyvalue of the pixel (typically between 0 and 4095) and
     * the corresponding bin in the histogram (usually containing only 128 bins).
     */
    {
     unsigned int i,j=0;

    for (i = 0; i < uiNrGreylevels; i++) pulHistogram[i] = 0L; /* clear histogram */

    for (i = 0; i < uiSizeY; i++) {
     j = 0;
     while (j < uiSizeX) { pulHistogram[pLookupTable[pImage[j]]]++; ++j; }
     pImage = &pImage[uiXRes];
     }
}

template <class kz_pixel_t>
void MapHistogram (unsigned long* pulHistogram, kz_pixel_t Min, kz_pixel_t Max,
               unsigned int uiNrGreylevels, unsigned long ulNrOfPixels)
/* This function calculates the equalized lookup table (mapping) by
 * cumulating the input histogram. Note: lookup table is rescaled in range [Min..Max].
 */
{
    unsigned int i;  unsigned long ulSum = 0;
    const float fScale = ((float)(Max - Min)) / ulNrOfPixels;
    const unsigned long ulMin = (unsigned long) Min;

    for (i = 0; i < uiNrGreylevels; i++) {
        ulSum += pulHistogram[i]; pulHistogram[i]=(unsigned long)(ulMin+ulSum*fScale);
        if (pulHistogram[i] > Max) pulHistogram[i] = Max;
        //std::cout << i << "->" << pulHistogram[i] << std::endl;
    }
}

template <class kz_pixel_t>
void MakeLut (kz_pixel_t * pLUT, kz_pixel_t Min, kz_pixel_t Max, unsigned int uiNrBins)
/* To speed up histogram clipping, the input image [Min,Max] is scaled down to
 * [0,uiNrBins-1]. This function calculates the LUT.
 */
{
    int i;
    const kz_pixel_t BinSize = (kz_pixel_t) (1 + (Max - Min) / uiNrBins);

    //std::cout << "BinSize: " << (int) BinSize << std::endl;
    
    for (i = Min; i <= Max; i++)  pLUT[i] = (i - Min) / BinSize;
}

template <class kz_pixel_t>
void Interpolate (kz_pixel_t * pImage, int uiXRes, unsigned long * pulMapLU,
     unsigned long * pulMapRU, unsigned long * pulMapLB,  unsigned long * pulMapRB,
     unsigned int uiXSize, unsigned int uiYSize, kz_pixel_t * pLUT)
/* pImage      - pointer to input/output image
 * uiXRes      - resolution of image in x-direction
 * pulMap*     - mappings of greylevels from histograms
 * uiXSize     - uiXSize of image submatrix
 * uiYSize     - uiYSize of image submatrix
 * pLUT        - lookup table containing mapping greyvalues to bins
 * This function calculates the new greylevel assignments of pixels within a submatrix
 * of the image with size uiXSize and uiYSize. This is done by a bilinear interpolation
 * between four different mappings in order to eliminate boundary artifacts.
 * It uses a division; since division is often an expensive operation, I added code to
 * perform a logical shift instead when feasible.
 */
{
    const unsigned int uiIncr = uiXRes-uiXSize; /* Pointer increment after processing row */
    kz_pixel_t GreyValue; unsigned int uiNum = uiXSize*uiYSize; /* Normalization factor */

    unsigned int uiXCoef, uiYCoef, uiXInvCoef, uiYInvCoef, uiShift = 0;

    if (uiNum & (uiNum - 1))   /* If uiNum is not a power of two, use division */
    for (uiYCoef = 0, uiYInvCoef = uiYSize; uiYCoef < uiYSize;
         uiYCoef++, uiYInvCoef--,pImage+=uiIncr) {
        for (uiXCoef = 0, uiXInvCoef = uiXSize; uiXCoef < uiXSize;
             uiXCoef++, uiXInvCoef--) {
            GreyValue = pLUT[*pImage];             /* get histogram bin value */
            *pImage++ = (kz_pixel_t ) ((uiYInvCoef * (uiXInvCoef*pulMapLU[GreyValue]
                                      + uiXCoef * pulMapRU[GreyValue])
                                + uiYCoef * (uiXInvCoef * pulMapLB[GreyValue]
                                      + uiXCoef * pulMapRB[GreyValue])) / uiNum);
        }
    }
    else {                         /* avoid the division and use a right shift instead */
        while (uiNum >>= 1) uiShift++;             /* Calculate 2log of uiNum */
        for (uiYCoef = 0, uiYInvCoef = uiYSize; uiYCoef < uiYSize;
             uiYCoef++, uiYInvCoef--,pImage+=uiIncr) {
             for (uiXCoef = 0, uiXInvCoef = uiXSize; uiXCoef < uiXSize;
               uiXCoef++, uiXInvCoef--) {
               GreyValue = pLUT[*pImage];         /* get histogram bin value */
               *pImage++ = (kz_pixel_t)((uiYInvCoef* (uiXInvCoef * pulMapLU[GreyValue]
                                      + uiXCoef * pulMapRU[GreyValue])
                                + uiYCoef * (uiXInvCoef * pulMapLB[GreyValue]
                                      + uiXCoef * pulMapRB[GreyValue])) >> uiShift);
            }
        }
    }
}
// *****************************************************************************

} // namespace detail
 
} // namespace imgproc

