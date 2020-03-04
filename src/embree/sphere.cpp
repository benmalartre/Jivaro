#include "../default.h"
#include "../utils/utils.h"
#include "sphere.h"
#include "context.h"

AMN_NAMESPACE_OPEN_SCOPE

// translate usd sphere to embree mesh
AmnUsdEmbreeSphere* 
TranslateSphere(AmnUsdEmbreeContext* ctxt, const pxr::UsdGeomSphere& usdSphere)
{
  
  AmnUsdEmbreeSphere* result = new AmnUsdEmbreeSphere();
  result->_type = RTC_GEOMETRY_TYPE_TRIANGLE;
  //result->_worldMatrix = usdMesh.GetPrim().GetWor;
  result->_geom = rtcNewGeometry(ctxt->_device, RTC_GEOMETRY_TYPE_TRIANGLE);
  result->_name = usdSphere.GetPrim().GetPrimPath().GetString();
  result->_resolution = 32;

  size_t num_lats = result->_resolution * 2;
  size_t num_longs = result->_resolution + 1;

  size_t num_vertices = (num_longs - 2) * num_lats + 2;
  size_t num_triangles = (num_vertices - 2 + num_lats) * 2;

  pxr::UsdAttribute radiusAttr = usdSphere.GetRadiusAttr();
  if(radiusAttr && radiusAttr.HasAuthoredValue())
  {
    radiusAttr.Get(&result->_radius , ctxt->_time);
  }
  else result->_radius = 1.f;

  BuildPoints(num_lats, 
              num_longs, 
              result->_positions, 
              result->_radius,
              result->_worldMatrix);

  rtcSetSharedGeometryBuffer(result->_geom,             // RTCGeometry
                            RTC_BUFFER_TYPE_VERTEX,     // RTCBufferType
                            0,                          // Slot
                            RTC_FORMAT_FLOAT3,          // RTCFormat
                            result->_positions.cdata(), // Datas Ptr
                            0,                          // Offset
                            sizeof(pxr::GfVec3f),       // Stride
                            num_vertices);              // Num Elements*/

  
  /*
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
  */
 return 0;
}

void 
BuildPoints(int num_lats, 
            int num_longs, 
            pxr::VtArray<GfVec3f>& positions, 
            float radius,
            float* worldMatrix)
{ 
  size_t num_vertices = (num_longs - 2) * num_lats + 2;
  positions.resize(num_vertices);
  int k;
  for(int i=0; i < num_longs; ++i)
  {
    float lng = PI *(-0.5f + i/(num_longs-1));
    float y = radius * sinf(lng);
    float yr = radius * cos(lng);
    pxr::GfVec3f p;
    if(i == 0)
    {
      p = pxr::GfVec3f(0, -radius, 0);
      positions[0] = p;
    }
    else if(i == num_longs-1)
    {
      p = pxr::GfVec3f(0, radius, 0);
    }
    else
    {
      for(int j=0; j<num_lats; ++j)
      {
        float lat = 2 * PI *((j-1) * (1/lats));
        float x = cos(lat);
        float z = sin(lat);
        p = pxr::GfVec3f(x*yr, y, z*yr);

      }
    }
    
  }
  /*
   For i = 0 To longs-1
      lng = #F32_PI *(-0.5 + i/(longs-1))
      y = radius * Sin(lng)
      yr = radius * Cos(lng)
      If i=0
        Vector3::Set(p,0,-radius,0)
        CArray::SetValue(*topo\vertices,0,p)
  
  
      ElseIf i = longs-1
        Vector3::Set(p,0,radius,0)
        CArray::SetValue(*topo\vertices,nbp-1,p)
  
  
      Else
        For j = 0 To lats-1
          lat = 2*#F32_PI * ((j-1)*(1/lats))
          x = Cos(lat)
          z = Sin(lat)
          Vector3::Set(p,x*yr,y,z*yr)
          k = (i-1)*lats+j+1
          CArray::SetValue(*topo\vertices,k,p)
  
        Next j
      EndIf
    Next i
  */
}

AMN_NAMESPACE_CLOSE_SCOPE