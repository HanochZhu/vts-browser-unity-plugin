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

using System;
using System.Runtime.InteropServices;

namespace vts
{
    [StructLayout(LayoutKind.Sequential)]
    public struct RenderOptions
    {
        [MarshalAs(UnmanagedType.R4)] public float textScale;
        [MarshalAs(UnmanagedType.U4)] public uint width;
        [MarshalAs(UnmanagedType.U4)] public uint height;
        [MarshalAs(UnmanagedType.U4)] public uint targetFrameBuffer;
        [MarshalAs(UnmanagedType.U4)] public uint targetViewportX;
        [MarshalAs(UnmanagedType.U4)] public uint targetViewportY;
        [MarshalAs(UnmanagedType.U4)] public uint targetViewportW;
        [MarshalAs(UnmanagedType.U4)] public uint targetViewportH;
        [MarshalAs(UnmanagedType.U4)] public uint antialiasingSamples;
        [MarshalAs(UnmanagedType.U4)] public uint renderGeodataDebug;
        [MarshalAs(UnmanagedType.I1)] public bool renderAtmosphere;
        [MarshalAs(UnmanagedType.I1)] public bool renderPolygonEdges;
        [MarshalAs(UnmanagedType.I1)] public bool flatShading;
        [MarshalAs(UnmanagedType.I1)] public bool geodataHysteresis;
        [MarshalAs(UnmanagedType.I1)] public bool debugDepthFeedback;
        [MarshalAs(UnmanagedType.I1)] public bool colorToTargetFrameBuffer;
        [MarshalAs(UnmanagedType.I1)] public bool colorToTexture;
    }
}
