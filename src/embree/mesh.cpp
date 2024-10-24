#include "../common.h"
#include "../embree/utils.h"
#include "../embree/mesh.h"
#include "../embree/context.h"

JVR_NAMESPACE_OPEN_SCOPE

// translate usd mesh to embree mesh
UsdEmbreeMesh* 
TranslateMesh(
  UsdEmbreeContext* ctxt, 
  const UsdGeomMesh& usdMesh,
  const GfMatrix4d& worldMatrix,
  RTCScene scene
)
{
  size_t num_vertices, num_triangles;
  UsdEmbreeMesh* result = new UsdEmbreeMesh();
  ctxt->_primCacheMap[usdMesh.GetPath()] = std::move(result);

  result->_type = RTC_GEOMETRY_TYPE_TRIANGLE;
  //result->_worldMatrix = usdMesh.GetPrim().GetWor;
  result->_geom = rtcNewGeometry(ctxt->_device, RTC_GEOMETRY_TYPE_TRIANGLE);
  result->_name = usdMesh.GetPrim().GetPrimPath().GetString();
  bool hasPoints = false;
  UsdAttribute pointsAttr = usdMesh.GetPointsAttr();
  if(pointsAttr && pointsAttr.HasAuthoredValue())
  {
    // get datas from usd
    pointsAttr.Get(&result->_vertices, ctxt->_time);
    num_vertices = result->_vertices.size();

    // world transform points
    for(auto& p: result->_vertices)
    {
      GfVec4d tmp = GfVec4d(p[0],p[1],p[2], 1.f) * worldMatrix;
      p[0] = tmp[0];p[1] = tmp[1];p[2] = tmp[2];
    }

    // set embree buffer
    rtcSetSharedGeometryBuffer(result->_geom,             // RTCGeometry
                              RTC_BUFFER_TYPE_VERTEX,     // RTCBufferType
                              0,                          // Slot
                              RTC_FORMAT_FLOAT3,          // RTCFormat
                              result->_vertices.cdata(), // Datas Ptr
                              0,                          // Offset
                              sizeof(GfVec3f),       // Stride
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
  UsdAttribute countsAttr = usdMesh.GetFaceVertexCountsAttr();
  UsdAttribute indicesAttr = usdMesh.GetFaceVertexIndicesAttr();
  VtArray<int> counts;
  VtArray<int> indices;
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

  //GetProperties(usdMesh.GetPrim(), ctxt);

  CheckColors(usdMesh, ctxt->_time, result);
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

  // world transform normals
    for(auto& n: result->_normals)
    {
      GfVec4d tmp = GfVec4d(n[0],n[1],n[2], 0.f) * worldMatrix;
      n[0] = tmp[0];n[1] = tmp[1];n[2] = tmp[2];
    }
  
  if(hasPoints && hasTriangles)
  { 
    /*
    rtcSetGeometryVertexAttributeCount(result->_geom, 1);
    rtcSetSharedGeometryBuffer(result->_geom, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, 
    RTC_FORMAT_FLOAT3,vertex_colors,0,sizeof(Vec3fa),num_vertices);
    */
    //UsdEmbreeSetTransform(result, worldMatrix);
    rtcCommitGeometry(result->_geom);
    result->_geomId = rtcAttachGeometry(scene, result->_geom);
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

// destructor
//------------------------------------------------------------------------------
void DeleteMesh(RTCScene scene, UsdEmbreeMesh* mesh)
{
  rtcDetachGeometry(scene, mesh->_geomId);
  delete mesh;
}


// check usd file for authored normals
//------------------------------------------------------------------------------
bool 
CheckNormals(const UsdGeomMesh& usdMesh,
            const UsdTimeCode& time,
            UsdEmbreeMesh* mesh)
{
  mesh->_hasNormals = false;
  UsdAttribute normalsAttr = usdMesh.GetNormalsAttr();
  if(normalsAttr && normalsAttr.HasAuthoredValue())
  {
    VtArray<GfVec3f> normals;
    normalsAttr.Get(&normals, time);
    int num_normals = normals.size();

    TfToken normalsInterpolation = usdMesh.GetNormalsInterpolation();
    if(normalsInterpolation == UsdGeomTokens->constant)
    {
      mesh->_normalsInterpolationType = CONSTANT;
    }
    else if(normalsInterpolation == UsdGeomTokens->uniform)
    {
      mesh->_normalsInterpolationType = UNIFORM;
    }
    else if(normalsInterpolation == UsdGeomTokens->varying)
    {
      mesh->_normalsInterpolationType = VARYING;
    }

    // vertex varying normals
    else if(normalsInterpolation == UsdGeomTokens->vertex)
    {
      if(num_normals == mesh->_vertices.size())
      {
        TriangulateData<GfVec3f>(mesh->_triangles, 
                                      normals,
                                      mesh->_normals);
        mesh->_hasNormals = true;
        mesh->_normalsInterpolationType = VERTEX;
      }
      else
      {
        std::cerr << "Problem with vertex varying normals datas : " <<
            usdMesh.GetPath().GetText() << " >>> fallback to compute them...";
      }
    }
    // face varying normals (per face-vertex)
    else if(normalsInterpolation == UsdGeomTokens->faceVarying)
    {
      if(num_normals == mesh->_numOriginalSamples)
      {
        TriangulateData<GfVec3f>(mesh->_samples, 
                                      normals,
                                      mesh->_normals);
        mesh->_hasNormals = true;
        mesh->_normalsInterpolationType = FACE_VARYING;
      }
      else
      {
        std::cerr << "Problem with face varying normals datas : " <<
            usdMesh.GetPath().GetText() << " >>> fallback to compute them...";
      }
    }

    if(mesh->_hasNormals)
    {
      rtcSetSharedGeometryBuffer(mesh->_geom,               // RTCGeometry
                                RTC_BUFFER_TYPE_VERTEX,     // RTCBufferType
                                1,                          // Slot
                                RTC_FORMAT_FLOAT3,          // RTCFormat
                                mesh->_normals.cdata(),     // Datas Ptr
                                0,                          // Offset
                                sizeof(GfVec3f),       // Stride
                                mesh->_normals.size());     // Num Elements*/
    }
  }
  return mesh->_hasNormals;
}

bool 
CheckColors(const UsdGeomMesh& usdMesh,
            const UsdTimeCode& time,
            UsdEmbreeMesh* mesh)
{
  mesh->_hasColors = false;

  UsdGeomPrimvar colorPrimVar = usdMesh.GetDisplayColorPrimvar();
  if(colorPrimVar) 
  {
    VtArray<GfVec3f> colors;
    UsdAttribute colorsAttr = colorPrimVar.GetAttr();
    colorsAttr.Get(&colors, time);
    int num_colors = colors.size();
    
    TfToken interpolation = colorPrimVar.GetInterpolation();
    if(interpolation == UsdGeomTokens->constant)
    {
      colorPrimVar.Get(&mesh->_colors);
      mesh->_hasColors = true;
      mesh->_colorsInterpolationType = CONSTANT;
    }
    else if(interpolation == UsdGeomTokens->uniform)
    {
    }
    else if(interpolation == UsdGeomTokens->varying)
    {
    }
    else if(interpolation == UsdGeomTokens->vertex)
    {
      if(num_colors == mesh->_vertices.size())
      {
        TriangulateData<GfVec3f>(mesh->_triangles, 
                                      colors,
                                      mesh->_colors);
        mesh->_hasColors = true;
        mesh->_colorsInterpolationType = VERTEX;
      }
      else
      {
        std::cerr << "Problem with vertex varying colors datas : " <<
            usdMesh.GetPath().GetText() << " >>> fallback to compute them...";
      }
    }
    else if(interpolation == UsdGeomTokens->faceVarying)
    {
      if(num_colors == mesh->_numOriginalSamples)
      {
        TriangulateData<GfVec3f>(mesh->_samples, 
                                      colors,
                                      mesh->_colors);
        mesh->_hasColors = true;
        mesh->_colorsInterpolationType = FACE_VARYING;
      }
      else
      {
        std::cerr << "Problem with face varying colors datas : " <<
          usdMesh.GetPath().GetText() << " >>> fallback to compute them...";     
      }
    } 
  }
  
}


JVR_NAMESPACE_CLOSE_SCOPE