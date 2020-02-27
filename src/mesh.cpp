#include "default.h"
#include "mesh.h"
#include "utils.h"

namespace embree {

  // adds a cube to the scene
UsdEmbreeMesh* 
TranslateMesh (RTCDevice device, RTCScene scene, 
  const pxr::UsdGeomMesh& usdMesh, double time)
{
  size_t num_vertices, num_triangles;
  UsdEmbreeMesh* result = new UsdEmbreeMesh();
  result->_mesh = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
  bool hasPoints = false;
  pxr::UsdAttribute pointsAttr = usdMesh.GetPointsAttr();
  if(pointsAttr && pointsAttr.HasAuthoredValue())
  {
    pointsAttr.Get(&result->_positions, time);
    num_vertices = result->_positions.size();

    rtcSetSharedGeometryBuffer(result->_mesh,             // RTCGeometry
                              RTC_BUFFER_TYPE_VERTEX,     // RTCBufferType
                              0,                          // Slot
                              RTC_FORMAT_FLOAT3,          // RTCFormat
                              result->_positions.cdata(), // Datas Ptr
                              0,                          // Offset
                              sizeof(pxr::GfVec3f),       // Stride
                              num_vertices);              // Num Elements*/
    hasPoints = true;
  }

  bool hasTriangles = false;
  pxr::UsdAttribute countsAttr = usdMesh.GetFaceVertexCountsAttr();
  pxr::UsdAttribute indicesAttr = usdMesh.GetFaceVertexIndicesAttr();
  if(countsAttr && countsAttr.HasAuthoredValue() &&
    indicesAttr && indicesAttr.HasAuthoredValue())
  {
    pxr::VtArray<int> counts;
    pxr::VtArray<int> indices;
    countsAttr.Get(&counts, time);
    indicesAttr.Get(&indices, time);

    num_triangles = TriangulateMesh(counts, 
                                    indices, 
                                    result->_triangles, 
                                    result->_samples);

    rtcSetSharedGeometryBuffer(result->_mesh,             // RTCGeometry
                              RTC_BUFFER_TYPE_INDEX,      // RTCBufferType
                              0,                          // Slot
                              RTC_FORMAT_UINT3,           // RTCFormat
                              result->_triangles.cdata(), // Datas Ptr
                              0,                          // Offset
                              3 * sizeof(int),            // Stride
                              num_triangles);             // Num Elements
    hasTriangles = true;
  }
  Vec3fa* vertex_colors = (Vec3fa*) alignedMalloc(num_vertices*sizeof(Vec3fa),16);
  for(int v=0;v<num_vertices;++v)
  {
    vertex_colors[v][0] = RANDOM_0_1;
    vertex_colors[v][1] = RANDOM_0_1;
    vertex_colors[v][2] = RANDOM_0_1;
  }

  CheckNormals(usdMesh, time, result);

  if(hasPoints && hasTriangles)
  {
    rtcSetGeometryVertexAttributeCount(result->_mesh, 1);
    rtcSetSharedGeometryBuffer(result->_mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, 
    RTC_FORMAT_FLOAT3,vertex_colors,0,sizeof(Vec3fa),num_vertices);
  
    rtcCommitGeometry(result->_mesh);
    result->_geomId = rtcAttachGeometry(scene, result->_mesh);
    rtcReleaseGeometry(result->_mesh);
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
  int num_samples = 0;
  for(auto count : counts)
  {
    num_triangles += count - 2;
    num_samples += count;
  }

  triangles.resize(num_triangles * 3);
  samples.resize(num_samples);

  int base = 0;
  int cnt = 0;
  int cnt2 = 0;
  for(auto count: counts)
  {
    for(int i = 1; i < count - 1; ++i)
    {
      triangles[cnt++] = indices[base + i    ];
      triangles[cnt++] = indices[base + i + 1];
      triangles[cnt++] = indices[base        ];
    }
    for(int i = 0; i < count; ++i)
    {
      samples[cnt2++] = base + i;
    }
    base += count;
  }
  return cnt / 3;
}

// triangulation datas from usd
template<typename T>
int 
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

void 
CheckNormals(const pxr::UsdGeomMesh& usdMesh,
            double time,
            UsdEmbreeMesh* mesh)
{
  pxr::UsdAttribute normalsAttr = usdMesh.GetNormalsAttr();
  if(normalsAttr && normalsAttr.HasAuthoredValue())
  {
    mesh->_hasNormals = false;
    pxr::VtArray<pxr::GfVec3f> normals;
    normalsAttr.Get(&normals, time);
    int num_normals = normals.size();

    pxr::TfToken normalsInterpolation = usdMesh.GetNormalsInterpolation();
    if(normalsInterpolation == pxr::UsdGeomTokens->constant)
    {
      mesh->_normalsInterpolationMode = CONSTANT;
    }
    else if(normalsInterpolation == pxr::UsdGeomTokens->uniform)
    {
      mesh->_normalsInterpolationMode = UNIFORM;
    }
    else if(normalsInterpolation == pxr::UsdGeomTokens->varying)
    {
      mesh->_normalsInterpolationMode = VARYING;
    }
    else if(normalsInterpolation == pxr::UsdGeomTokens->vertex)
    {
      if(num_normals == mesh->_positions.size())
      {
        TriangulateData(mesh->_triangles, 
                        normals,
                        mesh->_normals);
        mesh->_hasNormals = true;
        mesh->_normalsInterpolationMode = VERTEX;
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
      if(num_normals == mesh->_samples.size())
      {
        TriangulateData(mesh->_samples, 
                        normals,
                        mesh->_normals);
        mesh->_hasNormals = true;
        mesh->_normalsInterpolationMode = FACE_VARYING;
      }
      else
      {
        std::cerr << "FUCK" << "\n" << 
          "Problem with vertex varying normals datas : " <<
            "fallback to compute them...";
      }
    }

    if(mesh->_hasNormals)
    {
      std::cerr << "Pass Normals to Fuckin Embree..." << std::endl;
      rtcSetSharedGeometryBuffer(mesh->_mesh,             // RTCGeometry
                                RTC_BUFFER_TYPE_VERTEX,     // RTCBufferType
                                1,                          // Slot
                                RTC_FORMAT_FLOAT3,          // RTCFormat
                                mesh->_normals.cdata(),     // Datas Ptr
                                0,                          // Offset
                                sizeof(pxr::GfVec3f),       // Stride
                                mesh->_normals.size());     // Num Elements*/
    }
  }
}

} // namespace embree