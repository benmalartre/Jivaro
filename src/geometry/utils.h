#ifndef JVR_GEOMETRY_UTILS_H
#define JVR_GEOMETRY_UTILS_H

#include "../common.h"
#include "triangle.h"
#include <iostream>
#include "pxr/pxr.h"

#include "pxr/base/vt/array.h"
#include "pxr/base/gf/vec3i.h"
#include "pxr/base/gf/vec3f.h"

JVR_NAMESPACE_OPEN_SCOPE

//@see https://stackoverflow.com/questions/33333363/built-in-mod-vs-custom-mod-function-improve-the-performance-of-modulus-op/33333636#33333636
static inline size_t FastMod(const size_t i, const size_t c) 
{
  return i >= c ? i % c : i;
}

// Kahan and Babuska summation, Neumaier variant; accumulates less FP error
static inline double DoubleArraySum(const std::vector<double>& x) 
{
  double sum = x[0];
  double err = 0.0;

  for (size_t i = 1; i < x.size(); i++) {
    const double k = x[i];
    const double m = sum + k;
    err += std::fabs(sum) >= std::fabs(k) ? sum - m + k : k - m + sum;
    sum = m;
  }
  return sum + err;
}

template<typename T>
static inline double ComputeDistanceSquared(const T& a, const T& b) 
{
  return (b - a).GetLengthSquared();
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
    const pxr::GfVec3d abXac(ab ^ ac);

    // this is the from a to circumsphere center
    pxr::GfVec3d delta = (
      (abXac ^ ab) * ac.GetLengthSq() + 
      (ac ^ abXac) * ab.GetLengthSq()
    ) / (2.f*abXac.GetLengthSq());
    return delta.GetLength();
}

static inline pxr::GfVec3d ComputeCircumCenter(const pxr::GfVec3d& a, 
  const pxr::GfVec3d& b, const pxr::GfVec3d& c)
{
    const pxr::GfVec3d ab(b - a);
    const pxr::GfVec3d ac(c - a);
    const pxr::GfVec3d abXac(ab ^ ac);

    // this is the from a to circumsphere center
    pxr::GfVec3d delta = (
      (abXac ^ ab) * ac.GetLengthSq() + 
      (ac ^ abXac) * ab.GetLengthSq()
    ) / (2.f*abXac.GetLengthSq());
    return a + delta;
}

static inline bool _Orient(const double px, const double py, const double qx, 
  const double qy, const double rx, const double ry) 
{
  return (qy - py) * (rx - qx) - (qx - px) * (ry - qy) < 0.0;
}

static inline bool CheckPointInSphere(const pxr::GfVec3d& center, float radius, const pxr::GfVec3d& point)
{
  return (point - center).GetLengthSq() < (radius * radius);
}

const double EPSILON = std::numeric_limits<double>::epsilon();
const size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

template<typename T>
inline bool CheckPointsEquals(const T& a, const T& b) {
  return pxr::GfIsClose(a, b, EPSILON);
};

// monotonically increases with real angle, but doesn't need expensive trigonometry
inline double _GetPseudoAngle(const double dx, const double dy) {
    const double p = dx / (std::abs(dx) + std::abs(dy));
    return (dy > 0.0 ? 3.0 - p : 1.0 + p) / 4.0; // [0..1)
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