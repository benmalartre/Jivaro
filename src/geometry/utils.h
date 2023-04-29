#ifndef JVR_GEOMETRY_UTILS_H
#define JVR_GEOMETRY_UTILS_H

#include <pxr/pxr.h>
#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3i.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>

#include "../common.h"
#include "../geometry/triangle.h"


JVR_NAMESPACE_OPEN_SCOPE

struct ManipXformVectors {
  pxr::GfVec3d translation;
  pxr::GfVec3f rotation;
  pxr::GfVec3f scale;
  pxr::GfVec3f pivot;
  pxr::UsdGeomXformCommonAPI::RotationOrder rotOrder;
};

struct ManipTargetDesc {
  pxr::SdfPath path;
  pxr::GfMatrix4f base;
  pxr::GfMatrix4f offset;
  pxr::GfMatrix4f parent;
  ManipXformVectors previous;
  ManipXformVectors current;
};

typedef std::vector<ManipTargetDesc> ManipTargetDescList;

void _GetManipTargetXformVectors(pxr::UsdGeomXformCommonAPI& xformApi,
  ManipXformVectors& vectors, pxr::UsdTimeCode& time);

const double GEOM_EPSILON = std::numeric_limits<double>::epsilon();
const std::size_t GEOM_INVALID_INDEX = std::numeric_limits<std::size_t>::max();

template<typename T>
static inline double ComputeDistanceSquared(const T& a, const T& b)
{
  return (b - a).GetLengthSq();
}

template<typename T>
static inline double ComputeDistance(const T& a, const T& b) 
{
  return (b - a).GetLength();
}

static inline double ComputeCircumRadius(const pxr::GfVec3d& a, 
  const pxr::GfVec3d& b, const pxr::GfVec3d& c) 
{
    const pxr::GfVec3d ab(b - a);
    const pxr::GfVec3d ac(c - a);
    const pxr::GfVec3d n(ab ^ ac);

    // this is the from a to circumsphere center
    pxr::GfVec3d delta = (
      (n ^ ab) * ac.GetLengthSq() + 
      (ac ^ n) * ab.GetLengthSq()
    ) / (2.f * n.GetLengthSq());
    return delta.GetLength();
}

static inline pxr::GfVec3d ComputeCircumCenter(const pxr::GfVec3d& a, 
  const pxr::GfVec3d& b, const pxr::GfVec3d& c)
{
    const pxr::GfVec3d ab(b - a);
    const pxr::GfVec3d ac(c - a);
    const pxr::GfVec3d n(ab ^ ac);

    // this is the from a to circumsphere center
    pxr::GfVec3d delta = (
      (n ^ ab) * ac.GetLengthSq() + 
      (ac ^ n) * ab.GetLengthSq()
    ) / (2.f * n.GetLengthSq());
    return a + delta;
}

static inline bool CheckPointInSphere(const pxr::GfVec3d& center, float radius, const pxr::GfVec3d& point)
{
  return (point - center).GetLengthSq() < (radius * radius);
}

template<typename T>
inline bool CheckPointsEquals(const T& a, const T& b) {
  return pxr::GfIsClose(a, b, GEOM_EPSILON);
};

/// Barycentric coordinates
void GetBarycenter(const pxr::GfVec3f& p, const pxr::GfVec3f& a, const pxr::GfVec3f& b, 
  const pxr::GfVec3f& c, float* u, float* v, float* w);

/// Make circle
void
MakeCircle(std::vector<pxr::GfVec3f>* points, float radius, const pxr::GfMatrix4f& m, size_t n);

/// Make arc (circle part)
void
MakeArc(std::vector<pxr::GfVec3f>* points, float radius, const pxr::GfMatrix4f& m, size_t n, float startAngle, float endAngle);

/// Triangulate a polygonal mesh
int 
TriangulateMesh(const pxr::VtArray<int>& counts, 
                const pxr::VtArray<int>& indices, 
                pxr::VtArray<Triangle>& triangles);

void
UpdateTriangles(pxr::VtArray<Triangle>& triangles, size_t removeVertexIdx);


/// Compute smooth vertex normals on a triangulated polymesh
void 
ComputeVertexNormals(const pxr::VtArray<pxr::GfVec3f>& positions,
                     const pxr::VtArray<int>& counts,
                     const pxr::VtArray<int>& indices,
                     const pxr::VtArray<Triangle>& triangles,
                     pxr::VtArray<pxr::GfVec3f>& normals);

/// Compute triangle normals
void 
ComputeTriangleNormals( const pxr::VtArray<pxr::GfVec3f>& positions,
                        const pxr::VtArray<Triangle>& triangles,
                        pxr::VtArray<pxr::GfVec3f>& normals);
                          
/// Triangulate data
/// No checks are made on data type or array bounds
/// 
template<typename DataType>
void
TriangulateDatas( const pxr::VtArray<Triangle>& triangles,
                  const pxr::VtArray<DataType>& datas,
                  pxr::VtArray<DataType>& result)
{
  size_t numTriangles = triangles.size();
  result.resize(numTriangles * 3);
  for(int i = 0; i < numTriangles; ++i) {
    const Triangle* T = &triangles[i];
    result[i * 3    ] = datas[T->vertices[0]];
    result[i * 3 + 1] = datas[T->vertices[1]];
    result[i * 3 + 2] = datas[T->vertices[2]];
  }
};

/// Compute line tangents
void
ComputeLineTangents(const pxr::VtArray<pxr::GfVec3f>& points,
                    const pxr::VtArray<pxr::GfVec3f>& ups,
                    pxr::VtArray<pxr::GfVec3f>& tangents);

/// Compute plane from points
pxr::GfPlane 
ComputePlaneFromPoints(const pxr::VtArray<pxr::GfVec3f>& points);

JVR_NAMESPACE_CLOSE_SCOPE

#endif