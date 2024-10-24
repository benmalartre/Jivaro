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
  GfVec3d translation;
  GfVec3f rotation;
  GfVec3f scale;
  GfVec3f pivot;
  UsdGeomXformCommonAPI::RotationOrder rotOrder;
};

struct ManipTargetDesc {
  SdfPath path;
  GfMatrix4f base;
  GfMatrix4f offset;
  GfMatrix4f parent;
  ManipXformVectors previous;
  ManipXformVectors current;
};

typedef std::vector<ManipTargetDesc> ManipTargetDescList;

void _GetManipTargetXformVectors(UsdGeomXformCommonAPI& xformApi,
  ManipXformVectors& vectors, UsdTimeCode& time);

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

static inline double ComputeCircumRadius(const GfVec3d& a, 
  const GfVec3d& b, const GfVec3d& c) 
{
    const GfVec3d ab(b - a);
    const GfVec3d ac(c - a);
    const GfVec3d n(ab ^ ac);

    // this is the from a to circumsphere center
    GfVec3d delta = (
      (n ^ ab) * ac.GetLengthSq() + 
      (ac ^ n) * ab.GetLengthSq()
    ) / (2.f * n.GetLengthSq());
    return delta.GetLength();
}

static inline GfVec3d ComputeCircumCenter(const GfVec3d& a, 
  const GfVec3d& b, const GfVec3d& c)
{
    const GfVec3d ab(b - a);
    const GfVec3d ac(c - a);
    const GfVec3d n(ab ^ ac);

    // this is the from a to circumsphere center
    GfVec3d delta = (
      (n ^ ab) * ac.GetLengthSq() + 
      (ac ^ n) * ab.GetLengthSq()
    ) / (2.f * n.GetLengthSq());
    return a + delta;
}

static inline bool CheckPointInSphere(const GfVec3d& center, float radius, const GfVec3d& point)
{
  return (point - center).GetLengthSq() < (radius * radius);
}

template<typename T>
inline bool CheckPointsEquals(const T& a, const T& b, float eps=std::numeric_limits<float>::epsilon()) {
  return GfIsClose(a, b, eps);
};

/// Barycentric coordinates
void GetBarycenter(const GfVec3f& p, const GfVec3f& a, const GfVec3f& b, 
  const GfVec3f& c, float* u, float* v, float* w);

/// Make circle
void
MakeCircle(std::vector<GfVec3f>* points, float radius, const GfMatrix4f& m, size_t n);

/// Make arc (circle part)
void
MakeArc(std::vector<GfVec3f>* points, float radius, const GfMatrix4f& m, size_t n, float startAngle, float endAngle);

/// Triangulate a polygonal mesh
int 
TriangulateMesh(const VtArray<int>& counts, 
                const VtArray<int>& indices, 
                VtArray<Triangle>& triangles);

void
UpdateTriangles(VtArray<Triangle>& triangles, size_t removeVertexIdx);


/// Compute smooth vertex normals on a triangulated polymesh
void 
ComputeVertexNormals(const VtArray<GfVec3f>& positions,
                     const VtArray<int>& counts,
                     const VtArray<int>& indices,
                     const VtArray<Triangle>& triangles,
                     VtArray<GfVec3f>& normals);

/// Compute triangle normals
void 
ComputeTriangleNormals( const VtArray<GfVec3f>& positions,
                        const VtArray<Triangle>& triangles,
                        VtArray<GfVec3f>& normals);
                          
/// Triangulate data
/// No checks are made on data type or array bounds
/// 
template<typename T>
void
TriangulateDatas( const VtArray<Triangle>& triangles,
                  const VtArray<T>& datas,
                  VtArray<T>& result)
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
ComputeLineTangents(const VtArray<GfVec3f>& points,
                    const VtArray<GfVec3f>& ups,
                    VtArray<GfVec3f>& tangents);

/// Compute best plane from points
GfPlane 
ComputePlaneFromPoints(const VtArray<GfVec3f>& points);

GfPlane 
ComputePlaneFromPoints(const VtArray<int>& indices, const GfVec3f *positions);

/// Compute best line from points
GfLine
ComputeLineFromPoints(const VtArray<GfVec3f> &points);

GfLine
ComputeLineFromPoints(const VtArray<int>& indices, const GfVec3f *positions);

/// Compute covariance matrix
GfMatrix4f
ComputeCovarianceMatrix(const VtArray<GfVec3f>& points);

GfMatrix4f
ComputeCovarianceMatrix(const VtArray<int>& indices, const GfVec3f *positions);

static const GfVec3f 
OrthogonalVector(const GfVec3f &v)
{
  float x = GfAbs(v[0]);
  float y = GfAbs(v[1]);
  float z = GfAbs(v[2]);

  GfVec3f other = x < y ? (x < z ? GfVec3f::XAxis() : GfVec3f::ZAxis()) : 
    (y < z ? GfVec3f::YAxis() : GfVec3f::ZAxis());
  return GfCross(v, other);
}

static GfQuatf 
GetRotationBetweenVectors(const GfVec3f &u, const GfVec3f &v)
{
  const GfVec3f nu = u.GetNormalized();
  const GfVec3f nv = v.GetNormalized();
  if (nu == -nv)
    return GfQuatf(0, OrthogonalVector(nu).GetNormalized());
  
  GfVec3f half = (nu + nv).GetNormalized();
  return GfQuatf(GfDot(nu, half), GfCross(nu, half));
}


static GfQuatf 
RandomQuaternion() {
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
  return GfQuatf(x, y, s*u, s*v).GetNormalized();
}


JVR_NAMESPACE_CLOSE_SCOPE

#endif