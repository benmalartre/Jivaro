#include "../embree/utils.h"
#include "../embree/context.h"

JVR_NAMESPACE_OPEN_SCOPE

// triangulate mesh from usd
//------------------------------------------------------------------------------
int 
TriangulateMesh(const VtArray<int>& counts, 
                const VtArray<int>& indices, 
                VtArray<int>& triangles,
                VtArray<int>& samples)
{
  int num_triangles = 0;
  for(auto count : counts)
  {
    num_triangles += count - 2;
  }

  triangles.resize(num_triangles * 3);
  samples.resize(num_triangles * 3);

  int base = 0;
  int cnt = 0;
  int cnt2 = 0;
  for(auto count: counts)
  {
    for(int i = 1; i < count - 1; ++i)
    {
      triangles[cnt] = indices[base        ];
      samples[cnt++] = base;
      triangles[cnt] = indices[base + i    ];
      samples[cnt++] = base + i;
      triangles[cnt] = indices[base + i + 1];
      samples[cnt++] = base + i + 1;
    }
    base += count;
  }
  return cnt / 3;
}

// compute smooth vertex normals
// this procedure respect the original mesh topology
//------------------------------------------------------------------------------
void ComputeVertexNormals(const VtArray<GfVec3f>& positions,
                          const VtArray<int>& counts,
                          const VtArray<int>& indices,
                          const VtArray<int>& triangles,
                          VtArray<GfVec3f>& normals)
{
  // we want smooth vertex normals
  normals.resize(positions.size());

  // first compute triangle normals
  int totalNumTriangles = triangles.size()/3;
  VtArray<GfVec3f> triangleNormals;
  triangleNormals.resize(totalNumTriangles);

  for(int i = 0; i < totalNumTriangles; ++i)
  {
    GfVec3f ab = positions[triangles[i*3+1]] - positions[triangles[i*3]];
    GfVec3f ac = positions[triangles[i*3+2]] - positions[triangles[i*3]];
    triangleNormals[i] = ab ^ ac;
    triangleNormals[i].Normalize();
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
    GfVec3f n(0.f, 0.f, 0.f);
    for(int j = 0; j < numTriangles; ++j)
    {
      n += triangleNormals[base + j];
    }
    n.Normalize();
    polygonNormals[i] = n;
    base += numTriangles;
  }
  for(auto& n: polygonNormals) n.Normalize();

  // finaly average vertex normals
  for(auto& n : normals) n = GfVec3f(0.f, 0.f, 0.f);
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
GetProperties(const UsdPrim& prim, UsdEmbreeContext* ctxt)
{
  TfTokenVector propertyNames = prim.GetPropertyNames();
  for(auto t: propertyNames)
    std::cout << t.GetText() << std::endl;

  TfTokenVector appliedSchemas = prim.GetAppliedSchemas();
  for(auto t: appliedSchemas)
    std::cout << t.GetText() << std::endl;
}


JVR_NAMESPACE_CLOSE_SCOPE
