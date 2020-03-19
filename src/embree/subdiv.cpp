#include "../default.h"
#include "../utils/utils.h"
#include "utils.h"
#include "subdiv.h"
#include "context.h"

AMN_NAMESPACE_OPEN_SCOPE

// translate usd mesh to embree subdiv
AmnUsdEmbreeSubdiv* 
TranslateSubdiv(
  AmnUsdEmbreeContext* ctxt, 
  const pxr::UsdGeomMesh& usdMesh,
  const pxr::GfMatrix4d& worldMatrix
)
{

  size_t num_vertices, num_faces, num_indices;
  AmnUsdEmbreeSubdiv* result = new AmnUsdEmbreeSubdiv();

  result->_type = RTC_GEOMETRY_TYPE_SUBDIVISION;
  //result->_worldMatrix = usdMesh.GetPrim().GetWor;
  result->_geom = rtcNewGeometry(ctxt->_device, RTC_GEOMETRY_TYPE_SUBDIVISION);
  result->_name = usdMesh.GetPrim().GetPrimPath().GetString();
  bool hasPoints = false;
  pxr::UsdAttribute pointsAttr = usdMesh.GetPointsAttr();
  if(pointsAttr && pointsAttr.HasAuthoredValue())
  {
    // get datas from usd
    pointsAttr.Get(&result->_vertices, ctxt->_time);
    num_vertices = result->_vertices.size();

    // world transform points
    for(auto& p: result->_vertices)
    {
      pxr::GfVec4d tmp = pxr::GfVec4d(p[0],p[1],p[2], 1.f) * worldMatrix;
      p[0] = tmp[0];p[1] = tmp[1];p[2] = tmp[2];
    }

    // set embree buffer
    rtcSetSharedGeometryBuffer(result->_geom,             // RTCGeometry
                              RTC_BUFFER_TYPE_VERTEX,     // RTCBufferType
                              0,                          // Slot
                              RTC_FORMAT_FLOAT3,          // RTCFormat
                              result->_vertices.cdata(), // Datas Ptr
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

  bool hasTopo = false;
  pxr::UsdAttribute countsAttr = usdMesh.GetFaceVertexCountsAttr();
  pxr::UsdAttribute indicesAttr = usdMesh.GetFaceVertexIndicesAttr();
  //pxr::VtArray<int> counts;
  //pxr::VtArray<int> indices;
  if(countsAttr && countsAttr.HasAuthoredValue() &&
    indicesAttr && indicesAttr.HasAuthoredValue())
  {
    
    countsAttr.Get(&result->_counts, ctxt->_time);
    indicesAttr.Get(&result->_indices, ctxt->_time);

    num_faces = result->_counts.size();
    num_indices = result->_indices.size();

    rtcSetSharedGeometryBuffer(result->_geom,             // RTCGeometry
                              RTC_BUFFER_TYPE_INDEX,      // RTCBufferType
                              0,                          // Slot
                              RTC_FORMAT_UINT,            // RTCFormat
                              result->_indices.cdata(),   // Datas Ptr
                              0,                          // Offset
                              sizeof(unsigned int),       // Stride
                              num_indices);               // Num Elements

    rtcSetSharedGeometryBuffer(result->_geom,             // RTCGeometry
                              RTC_BUFFER_TYPE_FACE,       // RTCBufferType
                              0,                          // Slot
                              RTC_FORMAT_UINT,            // RTCFormat
                              result->_counts.cdata(),    // Datas Ptr
                              0,                          // Offset
                              sizeof(unsigned int),       // Stride
                              num_faces);                 // Num Elements

    hasTopo = true;
  }

  // if there are no triangles no point to continue 
  if(!hasTopo)
  {
    std::cerr << usdMesh.GetPrim().GetPrimPath() << "\n" << 
          "Problem getting topo data : " <<
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

/*
  CheckNormals(usdMesh, ctxt->_time, result);
  if(!result->_hasNormals)
  {
    ComputeVertexNormals(result->_vertices,
                        counts,
                        indices,
                        result->_triangles,
                        result->_normals);
    result->_hasNormals = true;
    result->_normalsInterpolationType = VERTEX;
  }
  */
  if(hasPoints && hasTopo)
  {
    /*
    // world transform normals
    for(auto& n: result->_normals)
    {
      pxr::GfVec4d tmp = pxr::GfVec4d(n[0],n[1],n[2], 0.f) * worldMatrix;
      n[0] = tmp[0];n[1] = tmp[1];n[2] = tmp[2];
    }
    
    rtcSetGeometryVertexAttributeCount(result->_geom, 1);
    rtcSetSharedGeometryBuffer(result->_geom, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, 
    RTC_FORMAT_FLOAT3,vertex_colors,0,sizeof(Vec3fa),num_vertices);
    */
    float* level = 
      (float*) rtcSetNewGeometryBuffer(
        result->_geom, 
        RTC_BUFFER_TYPE_LEVEL, 
        0, 
        RTC_FORMAT_FLOAT, 
        sizeof(float), 
        num_indices
      );
    for (unsigned int i=0; i<num_indices; i++)level[i] = result->_vertices[result->_indices[i]][1]/5;

    //AmnUsdEmbreeSetTransform(result, worldMatrix);
    rtcCommitGeometry(result->_geom);
    result->_geomId = rtcAttachGeometry(ctxt->_scene, result->_geom);
    rtcReleaseGeometry(result->_geom);
    return result;
  }
  else
  {
    std::cerr << usdMesh.GetPrim().GetPrimPath() << "\n" << 
      "Problem computing subdiv datas : " <<
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
            AmnUsdEmbreeSubdiv* subdiv)
{
  /*
  subdiv->_hasNormals = false;
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
      if(num_normals == mesh->_vertices.size())
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
                                mesh->_normals.size());     // Num Elements
    }
  }
  return mesh->_hasNormals;
  */
 return false;
}


AMN_NAMESPACE_CLOSE_SCOPE