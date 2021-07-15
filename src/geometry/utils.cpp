#include "utils.h"
#include <pxr\base\gf\matrix4f.h>

AMN_NAMESPACE_OPEN_SCOPE

void 
MakeCircle(std::vector<pxr::GfVec3f>* points, float radius, const pxr::GfMatrix4f& m, size_t n)
{
  float step = (360.f / (float)n) * DEGREES_TO_RADIANS;
  size_t baseIndex = points->size();
  points->resize(baseIndex + n);
  for (size_t k = 0; k < n; ++k) {
    (*points)[baseIndex + k] = m.TransformAffine(
      pxr::GfVec3f(std::sinf(step * k) * radius, 0.f, std::cosf(step * k) * radius));
  }
}

void
MakeArc(std::vector<pxr::GfVec3f>* points, float radius, const pxr::GfMatrix4f& m, size_t n, float startAngle, float endAngle)
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
        pxr::GfVec3f(std::sinf(step * k + endAngle) * radius, 0.f, std::cosf(step * k + endAngle) * radius));
    }
    else {
      (*points)[baseIndex + k] = m.TransformAffine(
        pxr::GfVec3f(std::sinf(step * k + startAngle) * radius, 0.f, std::cosf(step * k + startAngle) * radius));
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
ComputeLineTangents(const pxr::VtArray<pxr::GfVec3f>& points,
  const pxr::VtArray<pxr::GfVec3f>& ups,
  pxr::VtArray<pxr::GfVec3f>& tangents)
{
  size_t numPoints = points.size();
  tangents.resize(numPoints);
  pxr::GfVec3f current, previous, next;
  switch (numPoints) {
  case 0:
    return;
  case 1:
    tangents[0] = pxr::GfVec3f(0.f, 1.f, 0.f);
    return;
  case 2:
    current = points[1] - points[0];
    next = current;
  default:
    current = points[1] - points[0];
    next = points[2] - points[1];
  }

  for (int i = 1; i < numPoints; ++i) {

  }
}


AMN_NAMESPACE_CLOSE_SCOPE
