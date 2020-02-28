#include "default.h"
#include "mesh.h"
#include "utils.h"

namespace AMN {

  // adds a cube to the scene
UsdEmbreeMesh* 
TranslateMesh(RTCDevice device, RTCScene scene, 
  const pxr::UsdGeomMesh& usdMesh, float time)
{
  size_t num_vertices, num_triangles;
  UsdEmbreeMesh* result = new UsdEmbreeMesh();
  result->_geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
  bool hasPoints = false;
  pxr::UsdAttribute pointsAttr = usdMesh.GetPointsAttr();
  if(pointsAttr && pointsAttr.HasAuthoredValue())
  {
    pointsAttr.Get(&result->_positions, time);
    num_vertices = result->_positions.size();

    rtcSetSharedGeometryBuffer(result->_geom,             // RTCGeometry
                              RTC_BUFFER_TYPE_VERTEX,     // RTCBufferType
                              0,                          // Slot
                              RTC_FORMAT_FLOAT3,          // RTCFormat
                              result->_positions.cdata(), // Datas Ptr
                              0,                          // Offset
                              sizeof(pxr::GfVec3f),       // Stride
                              num_vertices);              // Num Elements*/
    hasPoints = true;
  }
  // if there are no points, no point to continue 
  if(!hasPoints)
  {
    std::cerr << usdMesh.GetPrim().GetPrimPath() << "\n" << 
          "Problem getting points datas : " <<
            "this mesh is invalid!";
    delete result;
    return NULL;
  }

  bool hasTriangles = false;
  pxr::UsdAttribute countsAttr = usdMesh.GetFaceVertexCountsAttr();
  pxr::UsdAttribute indicesAttr = usdMesh.GetFaceVertexIndicesAttr();
  pxr::VtArray<int> counts;
  pxr::VtArray<int> indices;
  if(countsAttr && countsAttr.HasAuthoredValue() &&
    indicesAttr && indicesAttr.HasAuthoredValue())
  {
    
    countsAttr.Get(&counts, time);
    indicesAttr.Get(&indices, time);

    result->_numOriginalSamples = 0;
    for(auto count : counts)result->_numOriginalSamples += count;

    num_triangles = TriangulateMesh(counts, 
                                    indices, 
                                    result->_triangles, 
                                    result->_samples);

    rtcSetSharedGeometryBuffer(result->_geom,             // RTCGeometry
                              RTC_BUFFER_TYPE_INDEX,      // RTCBufferType
                              0,                          // Slot
                              RTC_FORMAT_UINT3,           // RTCFormat
                              result->_triangles.cdata(), // Datas Ptr
                              0,                          // Offset
                              3 * sizeof(int),            // Stride
                              num_triangles);             // Num Elements
    hasTriangles = true;
  }

  // if there are no triangles no point to continue 
  if(!hasTriangles)
  {
    std::cerr << usdMesh.GetPrim().GetPrimPath() << "\n" << 
          "Problem computing triangle datas : " <<
            "this mesh is invalid!";
    delete result;
    return NULL;
  }

  /*
  Vec3fa* vertex_colors = (Vec3fa*) alignedMalloc(num_vertices*sizeof(Vec3fa),16);
  for(int v=0;v<num_vertices;++v)
  {
    vertex_colors[v][0] = RANDOM_0_1;
    vertex_colors[v][1] = RANDOM_0_1;
    vertex_colors[v][2] = RANDOM_0_1;
  }
  */

  CheckNormals(usdMesh, time, result);
  if(!result->_hasNormals)
  {
    ComputeVertexNormals(result->_positions,
                        counts,
                        indices,
                        result->_triangles,
                        result->_normals);
    result->_hasNormals = true;
    result->_normalsInterpolationType = VERTEX;
  }

  if(hasPoints && hasTriangles)
  {
    /*
    rtcSetGeometryVertexAttributeCount(result->_geom, 1);
    rtcSetSharedGeometryBuffer(result->_geom, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, 
    RTC_FORMAT_FLOAT3,vertex_colors,0,sizeof(Vec3fa),num_vertices);
    */
    rtcCommitGeometry(result->_geom);
    result->_geomId = rtcAttachGeometry(scene, result->_geom);
    rtcReleaseGeometry(result->_geom);
    return result;
  }
  else return NULL;

 return 0;
}

// triangulate mesh from usd
int 
TriangulateMesh(const pxr::VtArray<int>& counts, 
                const pxr::VtArray<int>& indices, 
                pxr::VtArray<int>& triangles,
                pxr::VtArray<int>& samples)
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

// triangulation datas from usd
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
}

// check usd file for authored normals
//------------------------------------------------------------------------------
bool 
CheckNormals(const pxr::UsdGeomMesh& usdMesh,
            double time,
            UsdEmbreeMesh* mesh)
{
  mesh->_hasNormals = false;
  pxr::UsdAttribute normalsAttr = usdMesh.GetNormalsAttr();
  if(normalsAttr && normalsAttr.HasAuthoredValue())
  {
    pxr::VtArray<pxr::GfVec3f> normals;
    normalsAttr.Get(&normals, time);
    int num_normals = normals.size();

    pxr::TfToken normalsInterpolation = usdMesh.GetNormalsInterpolation();
    if(normalsInterpolation == pxr::UsdGeomTokens->constant)
    {
      mesh->_normalsInterpolationType = CONSTANT;
    }
    else if(normalsInterpolation == pxr::UsdGeomTokens->uniform)
    {
      mesh->_normalsInterpolationType = UNIFORM;
    }
    else if(normalsInterpolation == pxr::UsdGeomTokens->varying)
    {
      mesh->_normalsInterpolationType = VARYING;
    }
    else if(normalsInterpolation == pxr::UsdGeomTokens->vertex)
    {
      if(num_normals == mesh->_positions.size())
      {
        TriangulateData(mesh->_triangles, 
                        normals,
                        mesh->_normals);
        mesh->_hasNormals = true;
        mesh->_normalsInterpolationType = VERTEX;
      }
      else
      {
        std::cerr << "FUCK" << "\n" << 
          "Problem with vertex varying normals datas : " <<
            "fallback to compute them...";
      }
    }
    else if(normalsInterpolation == pxr::UsdGeomTokens->faceVarying)
    {
      std::cout << "FACE VARYING NORMALS INTERPOLATION :D" << std::endl;
      if(num_normals == mesh->_numOriginalSamples)
      {
        TriangulateData(mesh->_samples, 
                        normals,
                        mesh->_normals);
        mesh->_hasNormals = true;
        mesh->_normalsInterpolationType = FACE_VARYING;
        std::cerr << "TRIANGULATED FACE VARYING NORMALS" << std::endl;
      }
      else
      {
        std::cerr << "FUCK" << "\n" << 
          "Problem with face varying normals datas : " <<
            "fallback to compute them...";
      }
    }

    if(mesh->_hasNormals)
    {
      rtcSetSharedGeometryBuffer(mesh->_geom,             // RTCGeometry
                                RTC_BUFFER_TYPE_VERTEX,     // RTCBufferType
                                1,                          // Slot
                                RTC_FORMAT_FLOAT3,          // RTCFormat
                                mesh->_normals.cdata(),     // Datas Ptr
                                0,                          // Offset
                                sizeof(pxr::GfVec3f),       // Stride
                                mesh->_normals.size());     // Num Elements*/
    }
  }
  return mesh->_hasNormals;
}

// compute smooth vertex normals
// this procedure respect the original mesh topology
// it probably have to be triangulated later :D
//------------------------------------------------------------------------------
void ComputeVertexNormals(const pxr::VtArray<pxr::GfVec3f>& positions,
                          const pxr::VtArray<int>& counts,
                          const pxr::VtArray<int>& indices,
                          const pxr::VtArray<int>& triangles,
                          pxr::VtArray<pxr::GfVec3f>& normals)
{
  // we want smooth vertex normals
  normals.resize(positions.size());

  // first compute triangle normals
  int totalNumTriangles = triangles.size()/3;
  pxr::VtArray<pxr::GfVec3f> triangleNormals;
  triangleNormals.resize(totalNumTriangles);

  for(int i = 0; i < totalNumTriangles; ++i)
  {
    pxr::GfVec3f ab = positions[triangles[i*3+1]] - positions[triangles[i*3]];
    pxr::GfVec3f ac = positions[triangles[i*3+2]] - positions[triangles[i*3]];
    triangleNormals[i] = ab ^ ac;
    triangleNormals[i].Normalize();
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
    pxr::GfVec3f n(0.f, 0.f, 0.f);
    for(int j = 0; j < numTriangles; ++j)
    {
      n += triangleNormals[base + j];
    }
    n.Normalize();
    polygonNormals[i] = n;
    base += numTriangles;
  }

  // finaly average vertex normals
  for(auto n : normals) n = pxr::GfVec3f(0.f, 0.f, 0.f);
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
  for(auto n: normals) n.Normalize();
  std::cerr << "COMPUTED VERTEX NORMALS" << std::endl;
}

} // namespace AMN