#include "../default.h"
#include "utils.h"
#include "mesh.h"
#include "context.h"

AMN_NAMESPACE_OPEN_SCOPE

// translate usd mesh to embree mesh
AmnUsdEmbreeMesh* 
TranslateMesh(AmnUsdEmbreeContext* ctxt, const pxr::UsdGeomMesh& usdMesh)
{
  size_t num_vertices, num_triangles;
  AmnUsdEmbreeMesh* result = new AmnUsdEmbreeMesh();
  result->_type = RTC_GEOMETRY_TYPE_TRIANGLE;
  //result->_worldMatrix = usdMesh.GetPrim().GetWor;
  result->_geom = rtcNewGeometry(ctxt->_device, RTC_GEOMETRY_TYPE_TRIANGLE);
  result->_name = usdMesh.GetPrim().GetPrimPath().GetString();
  bool hasPoints = false;
  pxr::UsdAttribute pointsAttr = usdMesh.GetPointsAttr();
  if(pointsAttr && pointsAttr.HasAuthoredValue())
  {
    pointsAttr.Get(&result->_positions, ctxt->_time);
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
    
    countsAttr.Get(&counts, ctxt->_time);
    indicesAttr.Get(&indices, ctxt->_time);

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

  CheckNormals(usdMesh, ctxt->_time, result);
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
    result->_geomId = rtcAttachGeometry(ctxt->_scene, result->_geom);
    rtcReleaseGeometry(result->_geom);
    return result;
  }
  else
  {
    std::cerr << usdMesh.GetPrim().GetPrimPath() << "\n" << 
      "Problem computing mesh datas : " <<
      "this mesh is invalid!";
    delete result;
    return NULL;
  };

 return 0;
}

// check usd file for authored normals
//------------------------------------------------------------------------------
bool 
CheckNormals(const pxr::UsdGeomMesh& usdMesh,
            const pxr::UsdTimeCode& time,
            AmnUsdEmbreeMesh* mesh)
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

    // vertex varying normals
    else if(normalsInterpolation == pxr::UsdGeomTokens->vertex)
    {
      if(num_normals == mesh->_positions.size())
      {
        TriangulateData<pxr::GfVec3f>(mesh->_triangles, 
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
    // face varying normals (per face-vertex)
    else if(normalsInterpolation == pxr::UsdGeomTokens->faceVarying)
    {
      std::cout << "FACE VARYING NORMALS INTERPOLATION :D" << std::endl;
      if(num_normals == mesh->_numOriginalSamples)
      {
        TriangulateData<pxr::GfVec3f>(mesh->_samples, 
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


AMN_NAMESPACE_CLOSE_SCOPE