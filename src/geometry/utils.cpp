#include "../geometry/utils.h"
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/plane.h>

JVR_NAMESPACE_OPEN_SCOPE

void GetBarycenter(const pxr::GfVec3f& p, const pxr::GfVec3f& a, 
  const pxr::GfVec3f& b, const pxr::GfVec3f& c, float* u, float* v, float* w)
{
  pxr::GfVec3f v0 = b - a;
  pxr::GfVec3f v1 = c - a;
  pxr::GfVec3f v2 = p - a;
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
MakeCircle(std::vector<pxr::GfVec3f>* points, float radius, 
  const pxr::GfMatrix4f& m, size_t n)
{
  float step = (360.f / (float)n) * DEGREES_TO_RADIANS;
  size_t baseIndex = points->size();
  points->resize(baseIndex + n);
  for (size_t k = 0; k < n; ++k) {
    (*points)[baseIndex + k] = m.TransformAffine(
      pxr::GfVec3f(std::sinf(step * k) * radius, 0.f, 
        std::cosf(step * k) * radius));
  }
}

void
MakeArc(std::vector<pxr::GfVec3f>* points, float radius, 
  const pxr::GfMatrix4f& m, size_t n, float startAngle, float endAngle)
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
        pxr::GfVec3f(std::sinf(step * k + endAngle) * radius, 0.f, 
          std::cosf(step * k + endAngle) * radius));
    }
    else {
      (*points)[baseIndex + k] = m.TransformAffine(
        pxr::GfVec3f(std::sinf(step * k + startAngle) * radius, 0.f, 
          std::cosf(step * k + startAngle) * radius));
    }
  }
}

int 
TriangulateMesh(const pxr::VtArray<int>& counts, 
                const pxr::VtArray<int>& indices, 
                pxr::VtArray<Triangle>& triangles)
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
        pxr::GfVec3i(
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

void
UpdateTriangles(pxr::VtArray<Triangle>& triangles, size_t removeVertexIdx)
{
  for (auto& triangle : triangles) {
    for (short axis = 0; axis < 3; ++axis) {
      int idx = triangle.vertices[axis];
      triangle.vertices[axis] = idx < removeVertexIdx ? idx : idx - 1;
    }
  }
}

void 
ComputeVertexNormals( const pxr::VtArray<pxr::GfVec3f>& positions,
                      const pxr::VtArray<int>& counts,
                      const pxr::VtArray<int>& indices,
                      const pxr::VtArray<Triangle>& triangles,
                      pxr::VtArray<pxr::GfVec3f>& normals)
{
  // we want smooth vertex normals
  normals.resize(positions.size());
  memset(normals.data(), 0.f, normals.size() * sizeof(pxr::GfVec3f));

  // first compute triangle normals
  int totalNumTriangles = triangles.size();
  pxr::VtArray<pxr::GfVec3f> triangleNormals;
  triangleNormals.resize(totalNumTriangles);

  for(int i = 0; i < totalNumTriangles; ++i)
  {
    const Triangle* T = &triangles[i];
    pxr::GfVec3f ab = positions[T->vertices[1]] - positions[T->vertices[0]];
    pxr::GfVec3f ac = positions[T->vertices[2]] - positions[T->vertices[0]];
    triangleNormals[i] = (ab ^ ac).GetNormalized();
  }

  // then polygons normals
  int numPolygons = counts.size();
  pxr::VtArray<pxr::GfVec3f> polygonNormals;
  polygonNormals.resize(numPolygons);
  int base = 0;
  for(int i=0; i < counts.size(); ++i)
  {
    int numVertices = counts[i];
    int numTriangles = numVertices - 2;
    pxr::GfVec3f n(0.f);
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
ComputeTriangleNormals( const pxr::VtArray<pxr::GfVec3f>& positions,
                        const pxr::VtArray<Triangle>& triangles,
                        pxr::VtArray<pxr::GfVec3f>& normals)
{
  int totalNumTriangles = triangles.size();
  normals.resize(totalNumTriangles);

  for(int i = 0; i < totalNumTriangles; ++i)
  {
    const Triangle* T = &triangles[i];
    pxr::GfVec3f ab = positions[T->vertices[1]] - positions[T->vertices[0]];
    pxr::GfVec3f ac = positions[T->vertices[2]] - positions[T->vertices[0]];
    normals[i] = (ab ^ ac).GetNormalized();
  }
}

void
ComputeLineTangents(const pxr::GfVec3f* points, const pxr::GfVec3f* ups,
  pxr::GfVec3f* tangents, size_t numPoints)
{
  size_t last = numPoints - 1;
  pxr::GfVec3f current, previous, next;
  switch (numPoints) {
  case 0:
    break;
  case 1:
    tangents[0] = pxr::GfVec3f(0.f, 1.f, 0.f);
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

// Constructs a plane from a collection of points
// so that the summed squared distance to all points is minimzized
pxr::GfPlane ComputePlaneFromPoints(const pxr::VtArray<pxr::GfVec3f>& points) 
{
  if(points.size()) {
      return pxr::GfPlane();
  }

  pxr::GfVec3f sum(0.0);
  for(const auto& point: points) {
      sum += point;
  }
  pxr::GfVec3f centroid = sum * (1.0 / (float)points.size());

  // Calc full 3x3 covariance matrix, excluding symmetries:
  float xx = 0.0; float xy = 0.0; float xz = 0.0;
  float yy = 0.0; float yz = 0.0; float zz = 0.0;

  for(const auto& point: points) {
    pxr::GfVec3f r = point - centroid;
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
      return pxr::GfPlane(); // The points don't span a plane
    }

  // Pick path with best conditioning:
  if(det_max == det_x) {
    return pxr::GfPlane(centroid, 
      pxr::GfVec3f(det_x, xz*yz - xy*zz, xy*yz - xz*yy).GetNormalized());
  } else if (det_max == det_y) {
    return pxr::GfPlane(centroid, 
      pxr::GfVec3f(xz*yz - xy*zz, det_y, xy*xz - yz*xx).GetNormalized());
  } else {
    return pxr::GfPlane(centroid, 
      pxr::GfVec3f(xy*yz - xz*yy, xy*xz - yz*xx, det_z).GetNormalized());
  };
}

JVR_NAMESPACE_CLOSE_SCOPE
