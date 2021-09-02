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
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace vts
{
    [StructLayout(LayoutKind.Sequential)]
    public struct CameraBase
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)] public double[] view;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)] public double[] proj;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)] public double[] eye;
        [MarshalAs(UnmanagedType.R8)] public double targetDistance;
        [MarshalAs(UnmanagedType.R8)] public double viewExtent;
        [MarshalAs(UnmanagedType.R8)] public double altitudeOverEllipsoid;
        [MarshalAs(UnmanagedType.R8)] public double altitudeOverSurface;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct DrawSurfaceBase
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)] public float[] mv;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)] public float[] uvTrans;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)] public float[] uvClip;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)] public float[] color;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)] public float[] center;
        [MarshalAs(UnmanagedType.R4)] public float blendingCoverage;
        [MarshalAs(UnmanagedType.I1)] public bool externalUv;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct DrawColliderBase
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)] public float[] mv;
    }

    public struct DrawSurfaceTask
    {
        public DrawSurfaceBase data;
        public Object mesh;
        public Object texColor;
        public Object texMask;
        public int RenderQueueBias;
        public bool isInit;
    }

    public struct DrawColliderTask
    {
        public DrawColliderBase data;
        public Object mesh;
    }

    public struct Atmosphere
    {
        public Object densityTexture;
        public double boundaryThickness;
        public double horizontalExponent;
        public double verticalExponent;
        public double colorGradientExponent;
        public float[] colorHorizon;
        public float[] colorZenith;

        public void Load(Map map)
        {
            if (colorHorizon == null)
            {
                colorHorizon = new float[4];
                colorZenith = new float[4];
            }
            float[] colors = new float[8];
            double[] parameters = new double[5];
            BrowserInterop.vtsCelestialAtmosphere(map.Handle, colors, parameters);
            Util.CheckInterop();
            for (int i = 0; i < 4; i++)
            {
                colorHorizon[i] = colors[i];
                colorZenith[i] = colors[i + 4];
            }
            colorGradientExponent = parameters[0];
            BrowserInterop.vtsCelestialAtmosphereDerivedAttributes(map.Handle, ref boundaryThickness, ref horizontalExponent, ref verticalExponent);
            Util.CheckInterop();
            {
                IntPtr ptr = BrowserInterop.vtsDrawsAtmosphereDensityTexture(map.Handle);
                Util.CheckInterop();
                if (ptr == IntPtr.Zero)
                    densityTexture = null;
                else
                {
                    GCHandle hnd = GCHandle.FromIntPtr(ptr);
                    densityTexture = hnd.Target;
                }
            }
        }
    }

    public struct Celestial
    {
        public Atmosphere atmosphere;
        public string name;
        public double majorRadius;
        public double minorRadius;

        public void Load(Map map)
        {
            atmosphere.Load(map);
            name = Util.CheckString(BrowserInterop.vtsCelestialName(map.Handle));
            majorRadius = BrowserInterop.vtsCelestialMajorRadius(map.Handle);
            Util.CheckInterop();
            minorRadius = BrowserInterop.vtsCelestialMinorRadius(map.Handle);
            Util.CheckInterop();
        }
    }

    public class Draws
    {
        public CameraBase camera;
        public Celestial celestial;
        public List<DrawSurfaceTask> opaque;
        public List<DrawSurfaceTask> transparent;
        public List<DrawColliderTask> colliders;

        private Object Load(IntPtr ptr)
        {
            if (ptr == IntPtr.Zero)
                return null;
            GCHandle hnd = GCHandle.FromIntPtr(ptr);
            return hnd.Target;
        }

        private void LoadSurfaces(ref List<DrawSurfaceTask> tasks, IntPtr group, uint cnt)
        {
            Util.CheckInterop();
            if (tasks == null)
                tasks = new List<DrawSurfaceTask>((int)cnt);
            else
                tasks.Clear();
            for (uint i = 0; i < cnt; i++)
            {
                DrawSurfaceTask t;
                IntPtr pm = IntPtr.Zero, ptc = IntPtr.Zero, ptm = IntPtr.Zero, pbs = IntPtr.Zero;
                BrowserInterop.vtsDrawsSurfaceTask(group, i, ref pm, ref ptc, ref ptm, ref pbs);
                Util.CheckInterop();
                if (pm == IntPtr.Zero)
                    continue;
                t.data = (DrawSurfaceBase)Marshal.PtrToStructure(pbs, typeof(DrawSurfaceBase));
                t.mesh = Load(pm);
                t.texColor = Load(ptc);
                t.texMask = Load(ptm);
                t.RenderQueueBias = 0;
                t.isInit = false;
                tasks.Add(t);
            }
        }

        private void LoadColliders(ref List<DrawColliderTask> tasks, IntPtr group, uint cnt)
        {
            Util.CheckInterop();
            if (tasks == null)
                tasks = new List<DrawColliderTask>((int)cnt);
            else
                tasks.Clear();
            for (uint i = 0; i < cnt; i++)
            {
                DrawColliderTask t;
                IntPtr pm = IntPtr.Zero, pbs = IntPtr.Zero;
                BrowserInterop.vtsDrawsColliderTask(group, i, ref pm, ref pbs);
                Util.CheckInterop();
                if (pm == IntPtr.Zero)
                    continue;
                t.data = (DrawColliderBase)Marshal.PtrToStructure(pbs, typeof(DrawColliderBase));
                t.mesh = Load(pm);
                tasks.Add(t);
            }
        }

        public void Load(Map map, Camera cam)
        {
            IntPtr camPtr = BrowserInterop.vtsDrawsCamera(cam.Handle);
            Util.CheckInterop();
            camera = (CameraBase)Marshal.PtrToStructure(camPtr, typeof(CameraBase));
            celestial.Load(map);
            IntPtr group = IntPtr.Zero;
            uint cnt = 0;
            BrowserInterop.vtsDrawsOpaqueGroup(cam.Handle, ref group, ref cnt);
            LoadSurfaces(ref opaque, group, cnt);
            BrowserInterop.vtsDrawsTransparentGroup(cam.Handle, ref group, ref cnt);
            LoadSurfaces(ref transparent, group, cnt);
            BrowserInterop.vtsDrawsCollidersGroup(cam.Handle, ref group, ref cnt);
            LoadColliders(ref colliders, group, cnt);
        }
    }
}
