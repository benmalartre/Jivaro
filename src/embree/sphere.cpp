#include "../common.h"
#include "sphere.h"
#include "context.h"

JVR_NAMESPACE_OPEN_SCOPE

// translate usd sphere to embree mesh
UsdEmbreeSphere* 
TranslateSphere(
  UsdEmbreeContext* ctxt, 
  const UsdGeomSphere& usdSphere,
  const GfMatrix4d& worldMatrix,
  RTCScene scene)
{
  
  UsdEmbreeSphere* result = new UsdEmbreeSphere();
  result->_type = RTC_GEOMETRY_TYPE_TRIANGLE;
  result->_geom = rtcNewGeometry(ctxt->_device, RTC_GEOMETRY_TYPE_TRIANGLE);
  result->_name = usdSphere.GetPrim().GetPrimPath().GetString();
  result->_resolution = 32;

  size_t num_lats = result->_resolution * 2;
  size_t num_longs = result->_resolution + 1;

  size_t num_vertices = (num_longs - 2) * num_lats + 2;
  size_t num_triangles = (num_vertices - 2 + num_lats) * 2;

  UsdAttribute radiusAttr = usdSphere.GetRadiusAttr();
  if(radiusAttr && radiusAttr.HasAuthoredValue())
  {
    radiusAttr.Get(&result->_radius , ctxt->_time);
  }
  else result->_radius = 1.f;

  BuildPoints(num_lats, 
              num_longs, 
              result->_vertices,
              result->_normals,
              result->_uvs, 
              result->_radius,
              GfMatrix4f(worldMatrix)[0]);

  rtcSetSharedGeometryBuffer(result->_geom,             // RTCGeometry
                            RTC_BUFFER_TYPE_VERTEX,     // RTCBufferType
                            0,                          // Slot
                            RTC_FORMAT_FLOAT3,          // RTCFormat
                            result->_vertices.cdata(),  // Datas Ptr
                            0,                          // Offset
                            sizeof(GfVec3f),       // Stride
                            num_vertices);              // Num Elements*/

  BuildTriangles(num_lats, num_longs, result->_triangles);

  //UsdEmbreeSetTransform(result, worldMatrix);
  rtcCommitGeometry(result->_geom);
  result->_geomId = rtcAttachGeometry(scene, result->_geom);
  rtcReleaseGeometry(result->_geom);
  
 return 0;
}

void DeleteSphere(RTCScene scene, UsdEmbreeSphere* sphere)
{
  rtcDetachGeometry(scene, sphere->_geomId);
  delete sphere;
}

void 
BuildPoints(int num_lats, 
            int num_longs, 
            VtArray<GfVec3f>& positions,
            VtArray<GfVec3f>& normals, 
            VtArray<GfVec2f>& uvs,  
            float radius,
            float* worldMatrix)
{ 
  size_t num_vertices = (num_longs - 2) * num_lats + 2;
  positions.resize(num_vertices);
  normals.resize(num_vertices);
  uvs.resize(num_vertices);
  int k;
  for(int i=0; i < num_longs; ++i)
  {
    float lng = M_PI *(-0.5f + i/(num_longs-1));
    float y = radius * sinf(lng);
    float yr = radius * cos(lng);
    if(i == 0)
    {
      positions[0] = GfVec3f(0, -radius, 0);
      normals[0] = GfVec3f(0, -1, 0);
      uvs[0] = GfVec2f(0.5, 0);

    }
    else if(i == num_longs-1)
    {
      positions[num_vertices-1] = GfVec3f(0, radius, 0);
      normals[num_vertices-1] = GfVec3f(0, 1, 0);
      uvs[num_vertices-1] = GfVec2f(0.5, 1.0);
    }
    else
    {
      for(int j=0; j<num_lats; ++j)
      {
        float lat = 2 * M_PI *((j-1) * (1/num_lats));
        float x = cos(lat);
        float z = sin(lat);
        
        k = (i-1) * num_lats + j + 1;
        positions[k] = GfVec3f(x*yr, y, z*yr);
        normals[k] = positions[k].GetNormalized();
        uvs[k] = GfVec2f(x, y);
      }
    }
  }
}

void BuildTriangles(int num_lats,
                    int num_longs,
                    VtArray<int>& triangles)
{ 
  size_t num_vertices = (num_longs - 2) * num_lats + 2;
  size_t num_triangles = (num_longs-3) * num_lats * 4 + num_lats * 2;
  size_t num_indices = num_triangles * 3;
  triangles.resize(num_indices);
  int offset = 0;

  for(int i=0; i < num_longs - 1; ++i)
  {
    for(int j=0; j < num_lats; ++j)
    {
      if(i == 0)
      {
        triangles[offset++] = (j + 1) % num_lats + 1;
        triangles[offset++] = j + 1;
        triangles[offset++] = 0;
      }
      else if(i == num_longs-2)
      {
        triangles[offset++] = num_vertices - 1;
        triangles[offset++] = num_vertices - num_lats + j - 1;
        if(j == num_lats-1)
          triangles[offset++] = num_vertices - num_lats - 1;
        else
          triangles[offset++] = num_vertices - num_lats + j;
      }
      else
      {
        int x[4];
        x[0] = (i - 1) * num_lats + j + 1;
        x[3] = x[0] + num_lats;
          
        if(j == num_lats - 1)
        {
          x[1] = x[0] - num_lats + 1;
          x[2] = x[0] + 1;
        }
        else
        {
          x[1] = x[0] + 1;  
          x[2] = x[0] + num_lats + 1;
        }
        triangles[offset++] = x[0];
        triangles[offset++] = x[1];
        triangles[offset++] = x[2];
        triangles[offset++] = x[0];
        triangles[offset++] = x[2];
        triangles[offset++] = x[3];
      }
      
    }
  } 
}

JVR_NAMESPACE_CLOSE_SCOPE