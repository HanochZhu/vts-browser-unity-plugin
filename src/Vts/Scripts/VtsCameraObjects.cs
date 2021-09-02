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

using System.Collections.Generic;
using UnityEngine;
using vts;

namespace vts
{
    public static partial class Extensions
    {
        internal static TValue GetOrCreate<TKey, TValue>(this IDictionary<TKey, TValue> dict, TKey key) where TValue : new()
        {
            TValue val;
            if (!dict.TryGetValue(key, out val))
            {
                val = new TValue();
                dict.Add(key, val);
            }
            return val;
        }
    }
}

public class VtsCameraObjects : VtsCameraBase
{

    public void Awake()
    {
        Shader.DisableKeyword("VTS_ATMOSPHERE");

    }
    protected override void Start()
    {
        base.Start();
        partsGroup = new GameObject(name + " - parts").transform;
    }

    protected void OnDestroy()
    {
        Destroy(partsGroup);
    }

    protected override void CameraDraw()
    {
        shiftingOriginMap = mapObject.GetComponent<VtsMapShiftingOrigin>();
        conv = Math.Mul44x44(Math.Mul44x44(VtsUtil.U2V44(mapTrans.localToWorldMatrix), VtsUtil.U2V44(VtsUtil.SwapYZ)), Math.Inverse44(draws.camera.view));
        UpdateSurfacesCache(opaquePrefab, draws.opaque, opaquePartsCache);
        UpdateSurfacesCache(transparentPrefab, draws.transparent, transparentPartsCache);
        originHasShifted = false;
    }

    public override void OriginShifted()
    {
        originHasShifted = true;
    }

    private void UpdateSurfacesCache(GameObject prefab, List<DrawSurfaceTask> tasks, Dictionary<VtsMesh, List<Part>> partsCache)
    {
        //tempMeshDic.Clear();
        // clear draw queue
        //  注意 DrawSurfaceTask 里面 mesh 是共享的
        //for (int i = 0; i < tasks.Count; i++)
        //{
        //    DrawSurfaceTask dst = tasks[i];
        //    {
        //        VtsMesh mesh = dst.mesh as VtsMesh;
        //        if (tempMeshDic.ContainsKey(mesh.meshLevel.levelString))
        //        {
        //            if (!dst.isInit)
        //            {
        //                dst.RenderQueueBias = tempMeshDic[mesh.meshLevel.levelString].RenderQueueBias + 1;
        //            }
        //            tempMeshDic[mesh.meshLevel.levelString] = dst;
        //        }
        //        else
        //        {
        //            tempMeshDic.Add(mesh.meshLevel.levelString, dst);
        //        }
        //        dst.isInit = true;
        //    }
        //    tasks[i] = dst;
        //}

        // organize tasks by meshes
        Dictionary<VtsMesh, List<DrawSurfaceTask>> tasksByMesh = new Dictionary<VtsMesh, List<DrawSurfaceTask>>();
        foreach (var t in tasks)
        {
            //List<DrawSurfaceTask> ts = tasksByMesh.GetOrCreate(t.mesh as VtsMesh);
            //ts.Add(t); 
            // 如果有共用的情况
            tasksByMesh.GetOrCreate(t.mesh as VtsMesh).Add(t);
        }

        // remove obsolete cache entries
        {
            tmpKeys.UnionWith(partsCache.Keys);
            tmpKeys.ExceptWith(tasksByMesh.Keys);
            foreach (var k in tmpKeys)
            {
                foreach (var j in partsCache[k])
                    Destroy(j.go);
                partsCache.Remove(k);
            }
            tmpKeys.Clear();
        }

        // update remaining cache entries
        // update the sampe mesh 
        foreach (var k in tasksByMesh)
        {
            // the tile with same mesh 
            List<DrawSurfaceTask> ts = k.Value;
            List<Part> ps = partsCache.GetOrCreate(k.Key);

            // update current objects
            int updatable = ts.Count < ps.Count ? ts.Count : ps.Count;
            for (int i = 0; i < updatable; i++)
                UpdatePart(ps[i], ts[i]);

            // update the rest of the cache
            // ps the tile already draw
            // ts the tile will be draw
            int changeCount = ts.Count - ps.Count;
            // only draw changed tile
            while (changeCount > 0)
            {
                DrawSurfaceTask surfacetask = ts[updatable];
                surfacetask.RenderQueueBias = updatable;
                // inflate
                ps.Add(InitPart(prefab, surfacetask));
                updatable ++;
                changeCount--;
            }
            // change the load queue
            // from front to back change to back to front
            //int tempChangeCount = 0;
            //while (changeCount > tempChangeCount)
            //{
            //    // inflate
            //    ps.Add(InitPart(prefab, ts[tempChangeCount++]));
            //    tempChangeCount++;
            //}
            if (changeCount < 0)
            {
                // deflate
                foreach (var p in ps.GetRange(updatable, -changeCount))
                    Destroy(p.go);
                ps.RemoveRange(updatable, -changeCount);
            }
            Debug.Assert(ts.Count == ps.Count);
        }
    }

    private void UpdateTransform(Part part, DrawSurfaceTask task)
    {
        VtsUtil.Matrix2Transform(part.go.transform, VtsUtil.V2U44(Math.Mul44x44(conv, System.Array.ConvertAll(task.data.mv, System.Convert.ToDouble))));
    }

    private Part InitPart(GameObject prefab, DrawSurfaceTask task)
    {

        Part part = new Part();
        part.go = Instantiate(prefab, partsGroup);
        UpdateTransform(part, task);
        part.mf = part.go.GetOrAddComponent<MeshFilter>();
        part.mf.mesh = (task.mesh as VtsMesh).Get();
        
        InitMaterial(propertyBlock, task);
        part.mr = part.go.GetOrAddComponent<MeshRenderer>();

        VtsMesh tempmesh = task.mesh as VtsMesh;

        // 修改他的渲染队列
        part.mr.material.renderQueue += (int)tempmesh.level * 10 + task.RenderQueueBias;

        part.mr.SetPropertyBlock(propertyBlock);
        if (shiftingOriginMap)
            part.go.GetOrAddComponent<VtsObjectShiftingOrigin>().map = shiftingOriginMap;
        return part;
    }

    private void UpdatePart(Part part, DrawSurfaceTask task)
    {
        if (originHasShifted)
            UpdateTransform(part, task);
        part.mr.GetPropertyBlock(propertyBlock);
        UpdateMaterial(propertyBlock, task);
        part.mr.SetPropertyBlock(propertyBlock);
    }

    public GameObject opaquePrefab;
    public GameObject transparentPrefab;

    private readonly HashSet<VtsMesh> tmpKeys = new HashSet<VtsMesh>();

    // 每一次绘制，只保证一个块中只有最后加载的会被绘制
    private Dictionary<string, DrawSurfaceTask> tempMeshDic = new Dictionary<string, DrawSurfaceTask>();

    private readonly Dictionary<VtsMesh, List<Part>> opaquePartsCache = new Dictionary<VtsMesh, List<Part>>();
    private readonly Dictionary<VtsMesh, List<Part>> transparentPartsCache = new Dictionary<VtsMesh, List<Part>>();
    private Transform partsGroup;

    private double[] conv;
    private VtsMapShiftingOrigin shiftingOriginMap;
    private bool originHasShifted = false;
}

internal class Part
{
    public GameObject go;
    public MeshFilter mf;
    public MeshRenderer mr;
}
