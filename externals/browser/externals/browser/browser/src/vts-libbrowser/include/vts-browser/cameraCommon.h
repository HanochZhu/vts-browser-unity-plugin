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

#ifndef CAMERA_COMMON_H_desgjjgf
#define CAMERA_COMMON_H_desgjjgf

#include "foundation.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vtsCDrawSurfaceBase
{
    float mv[16];
    float uvTrans[4]; // scale-x, scale-y, offset-x, offset-y
    float uvClip[4];
    float color[4]; // alpha is transparency
    float center[3];
    float blendingCoverage;
    bool externalUv;
} vtsCDrawSurfaceBase;

typedef struct vtsCDrawInfographicsBase
{
    float mv[16];
    float color[4];
    float data[4];
    float data2[4];
    int type; // 0 = original infographics; 1 = tile diagnostics text
} vtsCDrawInfographicsBase;

typedef struct vtsCDrawColliderBase
{
    float mv[16];
} vtsCDrawColliderBase;

typedef struct vtsCCameraBase
{
    double view[16];
    double proj[16];
    double eye[3]; // position in world (physical) space
    double targetDistance; // distance of the eye to center of orbit
    double viewExtent; // window height projected to world at the target distance
    double altitudeOverEllipsoid; // altitude of the eye over ellipsoid
    double altitudeOverSurface; // altitude of the eye over surface
} vtsCCameraBase;

#ifdef __cplusplus
} // extern C
#endif

#endif
