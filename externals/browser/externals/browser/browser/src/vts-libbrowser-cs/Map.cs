﻿/**
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
using System.Diagnostics;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace vts
{
    public enum Srs : uint
    {
        Physical,
        Navigation,
        Public,
        Search,
        Custom1,
        Custom2,
    }

    public partial class Map : IDisposable
    {
        public Map(string createOptions)
        {
            handle = BrowserInterop.vtsMapCreate(createOptions, IntPtr.Zero);
            Util.CheckInterop();
#if ENABLE_IL2CPP
            GlobalMaps.Add(handle, this);
#endif
            AssignInternalDelegates();
            AssignInternalCallbacks();
        }

#if ENABLE_IL2CPP
        private static Dictionary<IntPtr, Map> GlobalMaps = new Dictionary<IntPtr, Map>();
        private static Map GetMap(IntPtr h)
        {
            Debug.Assert(GlobalMaps[h] != null);
            return GlobalMaps[h];
        }
#else
        private Map GetMap(IntPtr h)
        {
            Debug.Assert(Handle == h);
            return this;
        }
#endif

        public void SetMapconfigPath(string mapconfigPath, string authPath = "")
        {
            BrowserInterop.vtsMapSetConfigPaths(Handle, mapconfigPath, authPath);
            Util.CheckInterop();
        }

        public string GetMapconfigPath()
        {
            return Util.CheckString(BrowserInterop.vtsMapGetConfigPath(Handle));
        }

        public bool GetMapconfigAvailable()
        {
            bool res = BrowserInterop.vtsMapGetConfigAvailable(Handle);
            Util.CheckInterop();
            return res;
        }

        public bool GetMapconfigReady()
        {
            bool res = BrowserInterop.vtsMapGetConfigReady(Handle);
            Util.CheckInterop();
            return res;
        }

        public bool GetMapRenderComplete()
        {
            bool res = BrowserInterop.vtsMapGetRenderComplete(Handle);
            Util.CheckInterop();
            return res;
        }

        public double GetMapRenderProgress()
        {
            double res = BrowserInterop.vtsMapGetRenderProgress(Handle);
            Util.CheckInterop();
            return res;
        }

        public bool GetProjected()
        {
            bool res = BrowserInterop.vtsMapGetProjected(Handle);
            Util.CheckInterop();
            return res;
        }

        public Position GetDefaultPosition()
        {
            Position p = new Position();
            IntPtr i = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(PositionBase)));
            BrowserInterop.vtsMapGetDefaultPosition(Handle, i);
            p.data = (PositionBase)Marshal.PtrToStructure(i, typeof(PositionBase));
            Marshal.FreeHGlobal(i);
            Util.CheckInterop();
            return p;
        }

        public void DataUpdate()
        {
            BrowserInterop.vtsMapDataUpdate(Handle);
            Util.CheckInterop();
        }

        public void DataFinalize()
        {
            BrowserInterop.vtsMapDataFinalize(Handle);
            Util.CheckInterop();
        }

        public void DataAllRun()
        {
            BrowserInterop.vtsMapDataAllRun(Handle);
            Util.CheckInterop();
        }

        public void RenderUpdate(double elapsedTime)
        {
            BrowserInterop.vtsMapRenderUpdate(Handle, elapsedTime);
            Util.CheckInterop();
        }

        public void RenderFinalize()
        {
            BrowserInterop.vtsMapRenderFinalize(Handle);
            Util.CheckInterop();
        }

        public string GetOptions()
        {
            return Util.CheckString(BrowserInterop.vtsMapGetOptions(Handle));
        }

        public string GetStatistics()
        {
            return Util.CheckString(BrowserInterop.vtsMapGetStatistics(Handle));
        }

        public void SetOptions(string json)
        {
            BrowserInterop.vtsMapSetOptions(Handle, json);
            Util.CheckInterop();
        }

        public double[] Convert(double[] point, Srs srsFrom, Srs srsTo)
        {
            Util.CheckArray(point, 3);
            double[] res = new double[3];
            BrowserInterop.vtsMapConvert(Handle, point, res, (uint)srsFrom, (uint)srsTo);
            Util.CheckInterop();
            return res;
        }

        //public bool GetAltitude(ref double result, double longtitude, double latitude)
        //{
        //    double[] nav = new double[3] { longtitude, latitude, 0.0 };

        //    // BrowserInterop.vtsMapConvert(Handle, nav, nav, (uint)Srs.Physical, (uint)Srs.Navigation);

        //    return BrowserInterop.vtsCameraGetSurfaceOverEllipsoid(Handle, ref result, nav, -1, false);
        //}
        public bool GetSearchable()
        {
            bool res = BrowserInterop.vtsMapGetSearchable(Handle);
            Util.CheckInterop();
            return res;
        }

        public SearchTask Search(string query)
        {
            IntPtr ptr = BrowserInterop.vtsMapSearch(Handle, query);
            Util.CheckInterop();
            return new SearchTask(ptr);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        ~Map()
        {
            Dispose(false);
        }

        protected void Dispose(bool disposing)
        {
            if (Handle != IntPtr.Zero)
            {
#if ENABLE_IL2CPP
                GlobalMaps.Remove(Handle);
#endif
                BrowserInterop.vtsMapDestroy(Handle);
                Util.CheckInterop();
                handle = IntPtr.Zero;
            }
        }

        protected IntPtr handle;
        public IntPtr Handle { get { return handle; } }
    }
}
