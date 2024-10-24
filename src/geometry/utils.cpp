#include "../geometry/utils.h"
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/plane.h>

JVR_NAMESPACE_OPEN_SCOPE

//==================================================================================
// HELPERS
//==================================================================================
void _GetManipTargetXformVectors(UsdGeomXformCommonAPI& xformApi,
  ManipXformVectors& vectors, UsdTimeCode& time)
{
  xformApi.GetXformVectors(&vectors.translation, &vectors.rotation, &vectors.scale,
    &vectors.pivot, &vectors.rotOrder, time);
}

void GetBarycenter(const GfVec3f& p, const GfVec3f& a, 
  const GfVec3f& b, const GfVec3f& c, float* u, float* v, float* w)
{
  GfVec3f v0 = b - a;
  GfVec3f v1 = c - a;
  GfVec3f v2 = p - a;
  float d00 = v0*v0;
  float d01 = v0*v1;
  float d11 = v1*v1;
  float d20 = v2*v0;
  float d21 = v2*v1;
  float denom = d00 * d11 - d01 * d01;
  *v = (d11 * d20 - d01 * d21) / denom;
  *w = (d00 * d21 - d01 * d20) / denom;
  *u = 1.0f - *v - *w;
}

void 
MakeCircle(std::vector<GfVec3f>* points, float radius, 
  const GfMatrix4f& m, size_t n)
{
  float step = (360.f / (float)n) * DEGREES_TO_RADIANS;
  size_t baseIndex = points->size();
  points->resize(baseIndex + n);
  for (size_t k = 0; k < n; ++k) {
    (*points)[baseIndex + k] = m.TransformAffine(
      GfVec3f(std::sinf(step * k) * radius, 0.f, 
        std::cosf(step * k) * radius));
  }
}

void
MakeArc(std::vector<GfVec3f>* points, float radius, 
  const GfMatrix4f& m, size_t n, float startAngle, float endAngle)
{
  float step;
  if (startAngle < endAngle) {
    step = ((endAngle - startAngle) / (float)n)* DEGREES_TO_RADIANS;
  }
  else {
    step = ((startAngle - endAngle) / (float)n) * DEGREES_TO_RADIANS;
  }
  size_t baseIndex = points->size();
  points->resize(baseIndex + n);
  for (size_t k = 0; k < n; ++k) {
    if (startAngle < endAngle) {
      (*points)[baseIndex + k] = m.TransformAffine(
        GfVec3f(std::sinf(step * k + endAngle) * radius, 0.f, 
          std::cosf(step * k + endAngle) * radius));
    }
    else {
      (*points)[baseIndex + k] = m.TransformAffine(
        GfVec3f(std::sinf(step * k + startAngle) * radius, 0.f, 
          std::cosf(step * k + startAngle) * radius));
    }
  }
}

int 
TriangulateMesh(const VtArray<int>& counts, 
                const VtArray<int>& indices, 
                VtArray<Triangle>& triangles)
{
  int num_triangles = 0;
  for(int count : counts)
  {
    num_triangles += count - 2;
  }

  triangles.resize(num_triangles);

  int base = 0;
  int tri = 0;
  for(int count: counts)
  {
    for(int i = 1; i < count - 1; ++i)
    {
      triangles[tri] = {
        static_cast<uint32_t>(tri), 
        GfVec3i(
          indices[base], 
          indices[base + i],
          indices[base + i + 1])
      };
      tri++;
    }
    base += count;
  }
  return num_triangles;
}

int
TriangulateMesh(const VtArray<int>& counts,
  const VtArray<int>& indices,
  VtArray<int>& triangles)
{
  int num_triangles = 0;
  for (int count : counts)
  {
    num_triangles += count - 2;
  }

  triangles.resize(num_triangles * 3);

  int base = 0;
  int tri = 0;
  for (int count : counts)
  {
    for (int i = 1; i < count - 1; ++i)
    {
      for (int j = 0; j < 3; ++j) {
        triangles[tri++] = indices[base + j];
      }
      tri++;
    }
    base += count;
  }
  return num_triangles;
}

void
UpdateTriangles(VtArray<Triangle>& triangles, size_t removeVertexIdx)
{
  for (auto& triangle : triangles) {
    for (short axis = 0; axis < 3; ++axis) {
      int idx = triangle.vertices[axis];
      triangle.vertices[axis] = idx < removeVertexIdx ? idx : idx - 1;
    }
  }
}

void
UpdateTriangles(VtArray<int>& triangles, size_t removeVertexIdx)
{
  for (size_t triangleIdx = 0; triangleIdx < triangles.size(); ++triangleIdx) {
    triangles[triangleIdx] = triangles[triangleIdx] - (removeVertexIdx < triangles[triangleIdx]);
  }
}

void 
ComputeVertexNormals( const VtArray<GfVec3f>& positions,
                      const VtArray<int>& counts,
                      const VtArray<int>& indices,
                      const VtArray<Triangle>& triangles,
                      VtArray<GfVec3f>& normals)
{
  // we want smooth vertex normals
  normals.resize(positions.size());
  memset(normals.data(), 0.f, normals.size() * sizeof(GfVec3f));

  // first compute triangle normals
  int totalNumTriangles = triangles.size();
  VtArray<GfVec3f> triangleNormals;
  triangleNormals.resize(totalNumTriangles);

  for(int i = 0; i < totalNumTriangles; ++i)
  {
    const Triangle* T = &triangles[i];
    GfVec3f ab = positions[T->vertices[1]] - positions[T->vertices[0]];
    GfVec3f ac = positions[T->vertices[2]] - positions[T->vertices[0]];
    triangleNormals[i] = ab ^ ac;
    if(!triangleNormals[i].GetLengthSq() < 1e-6)
      triangleNormals[i].Normalize();
    //std::cout << "fail get triangle normal at index " << T->GetIndex() << std::endl;
  }

  // then polygons normals
  int numPolygons = counts.size();
  VtArray<GfVec3f> polygonNormals;
  polygonNormals.resize(numPolygons);
  int base = 0;
  for(int i=0; i < counts.size(); ++i)
  {
    int numVertices = counts[i];
    int numTriangles = numVertices - 2;
    GfVec3f n(0.f);
    for(int j = 0; j < numTriangles; ++j)
    {
      n += triangleNormals[base + j];
    }
    polygonNormals[i] = n.GetNormalized();
    base += numTriangles;
  }

  // finaly average vertex normals  
  base = 0;
  for(int i = 0; i < counts.size(); ++i)
  {
    int numVertices = counts[i];
    for(int j = 0; j < numVertices; ++j)
    {
      normals[indices[base + j]] += polygonNormals[i];
    }
    base += numVertices;
  }
  
  for(auto& n: normals) n.Normalize();
}

void 
ComputeTriangleNormals( const VtArray<GfVec3f>& positions,
                        const VtArray<Triangle>& triangles,
                        VtArray<GfVec3f>& normals)
{
  int totalNumTriangles = triangles.size();
  normals.resize(totalNumTriangles);

  for(int i = 0; i < totalNumTriangles; ++i)
  {
    const Triangle* T = &triangles[i];
    GfVec3f ab = positions[T->vertices[1]] - positions[T->vertices[0]];
    GfVec3f ac = positions[T->vertices[2]] - positions[T->vertices[0]];
    normals[i] = (ab ^ ac).GetNormalized();
  }
}

void
ComputeLineTangents(const GfVec3f* points, const GfVec3f* ups,
  GfVec3f* tangents, size_t numPoints)
{
  size_t last = numPoints - 1;
  GfVec3f current, previous, next;
  switch (numPoints) {
  case 0:
    break;
  case 1:
    tangents[0] = GfVec3f(0.f, 1.f, 0.f);
    break;
  case 2:
    current = (points[1] - points[0]).GetNormalized();
    tangents[0] = current;
    tangents[1] = current;
    break;
  default:
    tangents[0] = (points[1] - points[0]).GetNormalized();
    tangents[last] = (points[last] - points[last-1]).GetNormalized();
    for (size_t i = 1; i < numPoints; ++i) {
      tangents[i] =
        ((points[i] - points[i - 1]) + 
        (points[i + 1] - points[i])).GetNormalized();
    }
    break;
  }
}

GfVec3f _ComputeCentroidFromPoints(size_t n, const GfVec3f* positions, const int *indices=NULL)
{
  GfVec3f sum(0.0);
  for(size_t i = 0; i < n; ++i) {
    sum += positions[indices ? indices[i] : i];
  }
  return sum * (1.0 / (float)n);
}

// Constructs a plane from a collection of points
// so that the summed squared distance to all points is minimzized
GfPlane _ComputePlaneFromPoints(size_t n, const GfVec3f* positions, const int *indices=NULL) 
{
  const GfVec3f centroid = _ComputeCentroidFromPoints(n, positions, indices);

  // Calc full 3x3 covariance matrix, excluding symmetries:
  float xx = 0.0; float xy = 0.0; float xz = 0.0;
  float yy = 0.0; float yz = 0.0; float zz = 0.0;

  for(size_t i = 0; i < n; ++i) {
    GfVec3f r = positions[indices ? indices[i] : i] - centroid;
    xx += r[0] * r[0];
    xy += r[0] * r[1];
    xz += r[0] * r[2];
    yy += r[1] * r[1];
    yz += r[1] * r[2];
    zz += r[2] * r[2];
  }

  float det_x = yy*zz - yz*yz;
  float det_y = xx*zz - xz*xz;
  float det_z = xx*yy - xy*xy;

  float det_max = det_x;
  if (det_y > det_max)det_max = det_y;
  if (det_z > det_max) det_max = det_z;
  if(det_max <= 0.0) {
    return GfPlane(); // The points don't span a plane
  }

  // Pick path with best conditioning:
  if(det_max == det_x) {
    return GfPlane(centroid, 
      GfVec3f(det_x, xz*yz - xy*zz, xy*yz - xz*yy).GetNormalized());
  } else if (det_max == det_y) {
    return GfPlane(centroid, 
      GfVec3f(xz*yz - xy*zz, det_y, xy*xz - yz*xx).GetNormalized());
  } else {
    return GfPlane(centroid, 
      GfVec3f(xy*yz - xz*yy, xy*xz - yz*xx, det_z).GetNormalized());
  };
}

GfPlane ComputePlaneFromPoints(const VtArray<GfVec3f>& points) 
{
  return _ComputePlaneFromPoints(points.size(), &points[0]);
}

GfPlane ComputePlaneFromPoints(const VtArray<int>& indices, const GfVec3f *positions) 
{
  return _ComputePlaneFromPoints(indices.size(), positions, &indices[0]);
}

// Constructs a line from a collection of points
// so that the summed squared distance to all points is minimzized
GfLine _ComputeLineFromPoints(size_t n, const GfVec3f* positions, const int *indices=NULL, size_t maxIterations=6) 
{
  const GfVec3f centroid = _ComputeCentroidFromPoints(n, positions, indices);

  GfVec3f direction(0, 1, 0);
  if (!n || !maxIterations) return GfLine(centroid, direction);
  
  GfVec3f point;
  GfVec3f last = direction;
  for (size_t i = 0; i < maxIterations; ++i) {
    for (size_t p = 0; p < n; ++p) {
      point = positions[indices ? indices[p] : p] - centroid;
      direction += point * GfDot(point, last);
    }
    direction.Normalize();
    if (GfIsClose(last, direction, 0.00001)) break;
    last = direction;
  }

  return GfLine(centroid, last);
}

GfLine ComputeLineFromPoints(const VtArray<GfVec3f>& points) 
{
  return _ComputeLineFromPoints(points.size(), &points[0]);
}

GfLine ComputeLineFromPoints(const VtArray<int>& indices, const GfVec3f *positions) 
{
  return _ComputeLineFromPoints(indices.size(), positions, &indices[0]);
}

// Constructs the covariant matrix from a collection of points
GfMatrix4f _ComputeCovarianceMatrix(size_t n, const GfVec3f* positions, const int *indices=NULL) 
{
  const GfVec3f centroid = _ComputeCentroidFromPoints(n, positions, indices);

  float xx = 0.0; float xy = 0.0; float xz = 0.0;
  float yy = 0.0; float yz = 0.0; float zz = 0.0;

  for(size_t i = 0; i < n; ++i) {
    GfVec3f r = positions[indices ? indices[i] : i] - centroid;
    xx += r[0] * r[0];
    xy += r[0] * r[1];
    xz += r[0] * r[2];
    yy += r[1] * r[1];
    yz += r[1] * r[2];
    zz += r[2] * r[2];
  }

  return GfMatrix4f(
    xx, xy, xz, 0.f,
    xy, yy, yz, 0.f,
    xz, yz, zz, 0.f,
    centroid[0], centroid[1], centroid[2], 1.f
    );
}

GfMatrix4f ComputeCovarianceMatrix(const VtArray<GfVec3f>& points) 
{
  return _ComputeCovarianceMatrix(points.size(), &points[0]);
}

GfMatrix4f ComputeCovarianceMatrix(const VtArray<int>& indices, const GfVec3f *positions) 
{
  return _ComputeCovarianceMatrix(indices.size(), positions, &indices[0]);
}


JVR_NAMESPACE_CLOSE_SCOPE
