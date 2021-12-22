#pragma once

#include "../common.h"

// embree
#include <embree3/rtcore.h>
#include <sys/platform.h>
#include <sys/sysinfo.h>
#include <sys/ref.h>
#include <sys/vector.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <math/bbox.h>
#include <math/lbbox.h>
#include <math/affinespace.h>
#include <sys/filename.h>
#include <sys/string.h>
#include <lexers/tokenstream.h>
#include <lexers/streamfilters.h>
#include <lexers/parsestream.h>

#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec4f.h>
#include <pxr/base/gf/vec4d.h>
#include <pxr/base/gf/quaternion.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/matrix4d.h>

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/imageable.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usdGeom/bboxCache.h>



PXR_NAMESPACE_OPEN_SCOPE

#define EDGE_LEVEL 256.0f
#define ENABLE_SMOOTH_NORMALS 1

// conversion pxr/embree - embree/pxr  
inline embree::Vec3fa 
pxr2embree(const pxr::GfVec3f& v){return embree::Vec3fa(v[0], v[1], v[2]);};
inline embree::Vec3fa 
pxr2embree(const pxr::GfVec3d& v){return embree::Vec3fa((float)v[0], (float)v[1], (float)v[2]);};
inline embree::Vec2fa
pxr2embree(const pxr::GfVec2f& v){return embree::Vec2fa(v[0], v[1]);};
inline embree::Vec2fa 
pxr2embree(const pxr::GfVec2d& v){return embree::Vec2fa((float)v[0], (float)v[1]);};

inline pxr::GfVec3f
embree2pxr(const embree::Vec3fa & v){return pxr::GfVec3f(v.x, v.y, v.z);};
inline pxr::GfVec2f
embree2pxr(const embree::Vec2fa & v){return pxr::GfVec2f(v.x, v.y);};

class UsdEmbreeContext;
void 
ComputeVertexNormals(const pxr::VtArray<pxr::GfVec3f>& positions,
                    const pxr::VtArray<int>& counts,
                    const pxr::VtArray<int>& indices,
                    const pxr::VtArray<int>& triangles,
                    pxr::VtArray<pxr::GfVec3f>& normals);
/*
void 
ComputeVertexColors(const pxr::VtArray<pxr::GfVec3f>& positions,
                    const pxr::VtArray<int>& counts,
                    const pxr::VtArray<int>& indices,
                    const pxr::VtArray<int>& triangles,
                    pxr::GfVec3f color,
                    pxr::VtArray<pxr::GfVec3f>& normals);

void 
ComputeVertexColors(const pxr::VtArray<pxr::GfVec3f>& positions,
                    const pxr::VtArray<int>& counts,
                    const pxr::VtArray<int>& indices,
                    pxr::GfVec3f color,
                    pxr::VtArray<pxr::GfVec3f>& normals);
*/
int 
TriangulateMesh(const pxr::VtArray<int>& counts, 
                const pxr::VtArray<int>& indices, 
                pxr::VtArray<int>& triangles,
                pxr::VtArray<int>& samples);

template<typename T>
void 
TriangulateData(const pxr::VtArray<int>& indices, 
                const pxr::VtArray<T>& datas,
                pxr::VtArray<T>& result)
{
  result.resize(indices.size());
  for(int i=0;i<indices.size();++i)
  {
    result[i] = datas[indices[i]];
  }
};

void
GetProperties(const pxr::UsdPrim& prim, UsdEmbreeContext* ctxt);

PXR_NAMESPACE_CLOSE_SCOPE