#ifndef JVR_GEOMETRY_UTILS_H
#define JVR_GEOMETRY_UTILS_H

#include <pxr/pxr.h>
#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3i.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/plane.h>
#include <pxr/base/gf/line.h>
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
inline bool CheckPointsEquals(const T& a, const T& b, float eps=std::numeric_limits<float>::epsilon()) {
  return pxr::GfIsClose(a, b, eps);
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
template<typename T>
void
TriangulateDatas( const pxr::VtArray<Triangle>& triangles,
                  const pxr::VtArray<T>& datas,
                  pxr::VtArray<T>& result)
{
  size_t numTriangles = triangles.size();
  result.resize(numTriangles * 3);
  for(int i = 0; i < numTriangles; ++i) {
    const Triangle* triangle = &triangles[i];
    result[i * 3    ] = datas[triangle->vertices[0]];
    result[i * 3 + 1] = datas[triangle->vertices[1]];
    result[i * 3 + 2] = datas[triangle->vertices[2]];
  }
};

/// Compute line tangents
void
ComputeLineTangents(const pxr::VtArray<pxr::GfVec3f>& points,
                    const pxr::VtArray<pxr::GfVec3f>& ups,
                    pxr::VtArray<pxr::GfVec3f>& tangents);

/// Compute best plane from points
pxr::GfPlane 
ComputePlaneFromPoints(const pxr::VtArray<pxr::GfVec3f>& points);

pxr::GfPlane 
ComputePlaneFromPoints(const pxr::VtArray<int>& indices, const pxr::GfVec3f *positions);

/// Compute best line from points
pxr::GfLine
ComputeLineFromPoints(const pxr::VtArray<pxr::GfVec3f> &points);

pxr::GfLine
ComputeLineFromPoints(const pxr::VtArray<int>& indices, const pxr::GfVec3f *positions);

/// Compute covariance matrix
pxr::GfMatrix4f
ComputeCovarianceMatrix(const pxr::VtArray<pxr::GfVec3f>& points);

pxr::GfMatrix4f
ComputeCovarianceMatrix(const pxr::VtArray<int>& indices, const pxr::GfVec3f *positions);


static pxr::GfQuatf RandomQuaternion() {
  float x,y,z, u,v,w, s;
  do { 
    x = RANDOM_LO_HI(-1.0,1.0); 
    y = RANDOM_LO_HI(-1.0,1.0); 
    z = x*x + y*y; 
  } while (z > 1);

  do { 
    u = RANDOM_LO_HI(-1.0,1.0); 
    v = RANDOM_LO_HI(-1.0,1.0); 
    w = u*u + v*v; 
  } while (w > 1);

  s = sqrt((1-z) / w);
  return pxr::GfQuatf(x, y, s*u, s*v).GetNormalized();
}


JVR_NAMESPACE_CLOSE_SCOPE

#endif