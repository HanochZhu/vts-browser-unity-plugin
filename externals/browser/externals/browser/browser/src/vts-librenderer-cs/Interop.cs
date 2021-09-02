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

// THIS FILE IS GENERATED BY SCRIPT
// DO NOT MODIFY

using System;
using System.Runtime.InteropServices;

namespace vts
{
	public static class RendererInterop
	{

#if ENABLE_IL2CPP && UNITY_IOS
	const string LibName = "__Internal";
#else
	const string LibName = "vts-renderer";
#endif

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate IntPtr  GLADloadproc([MarshalAs(UnmanagedType.LPStr)] string name);

[DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
public static extern void vtsCheckGl([MarshalAs(UnmanagedType.LPStr)] string name);

[DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
public static extern void vtsCheckGlFramebuffer(uint target);

[DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
public static extern void vtsLoadGlFunctions(GLADloadproc functionLoader);

[DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
public static extern IntPtr vtsRenderContextCreate();

[DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
public static extern void vtsRenderContextDestroy(IntPtr context);

[DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
public static extern void vtsRenderContextBindLoadFunctions(IntPtr context, IntPtr map);

[DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
public static extern IntPtr vtsRenderContextCreateView(IntPtr context, IntPtr camera);

[DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
public static extern void vtsRenderViewDestroy(IntPtr view);

[DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
public static extern IntPtr vtsRenderViewOptions(IntPtr view);

[DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
public static extern IntPtr vtsRenderViewVariables(IntPtr view);

[DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
public static extern void vtsRenderViewRender(IntPtr view);

[DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
public static extern void vtsRenderViewRenderCompas(IntPtr view, [In] double[] screenPosSize, [In] double[] mapRotation);

[DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
public static extern void vtsRenderViewGetWorldPosition(IntPtr view, [In] double[] screenPosIn, [Out] double[] worldPosOut);

	}
}