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



JVR_NAMESPACE_OPEN_SCOPE

#define EDGE_LEVEL 256.0f
#define ENABLE_SMOOTH_NORMALS 1

// conversion pxr/embree - embree/pxr  
inline embree::Vec3fa 
pxr2embree(const GfVec3f& v){return embree::Vec3fa(v[0], v[1], v[2]);};
inline embree::Vec3fa 
pxr2embree(const GfVec3d& v){return embree::Vec3fa((float)v[0], (float)v[1], (float)v[2]);};
inline embree::Vec2fa
pxr2embree(const GfVec2f& v){return embree::Vec2fa(v[0], v[1]);};
inline embree::Vec2fa 
pxr2embree(const GfVec2d& v){return embree::Vec2fa((float)v[0], (float)v[1]);};

inline GfVec3f
embree2pxr(const embree::Vec3fa & v){return GfVec3f(v.x, v.y, v.z);};
inline GfVec2f
embree2pxr(const embree::Vec2fa & v){return GfVec2f(v.x, v.y);};

class UsdEmbreeContext;
void 
ComputeVertexNormals(const VtArray<GfVec3f>& positions,
                    const VtArray<int>& counts,
                    const VtArray<int>& indices,
                    const VtArray<int>& triangles,
                    VtArray<GfVec3f>& normals);
/*
void 
ComputeVertexColors(const VtArray<GfVec3f>& positions,
                    const VtArray<int>& counts,
                    const VtArray<int>& indices,
                    const VtArray<int>& triangles,
                    GfVec3f color,
                    VtArray<GfVec3f>& normals);

void 
ComputeVertexColors(const VtArray<GfVec3f>& positions,
                    const VtArray<int>& counts,
                    const VtArray<int>& indices,
                    GfVec3f color,
                    VtArray<GfVec3f>& normals);
*/
int 
TriangulateMesh(const VtArray<int>& counts, 
                const VtArray<int>& indices, 
                VtArray<int>& triangles,
                VtArray<int>& samples);

template<typename T>
void 
TriangulateData(const VtArray<int>& indices, 
                const VtArray<T>& datas,
                VtArray<T>& result)
{
  result.resize(indices.size());
  for(int i=0;i<indices.size();++i)
  {
    result[i] = datas[indices[i]];
  }
};

void
GetProperties(const UsdPrim& prim, UsdEmbreeContext* ctxt);

JVR_NAMESPACE_CLOSE_SCOPE