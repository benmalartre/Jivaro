#include "context.h"

namespace embree {
  void
  UsdEmbreeContext::Initialize()
  {
    // create device
    _device = rtcNewDevice(embree::rtcore.c_str());
    std::cout << "DEVICE " << _device << std::endl;
    //error_handler(nullptr,rtcGetDeviceError(g_device));

    // create scene 
    _scene = rtcNewScene(_device);
    std::cout << "DEVICE " << _scene << std::endl;

    // add ground plane
    AddGroundPlane(_scene);
  }

  void
  UsdEmbreeContext::Term()
  {
    // commit changes to scene
    rtcCommitScene (_scene);
  }

  void
  UsdEmbreeContext::Clean()
  {
    rtcReleaseScene (_scene); 
    _scene = nullptr;
    //alignedFree(face_colors); face_colors = nullptr;
    //alignedFree(vertex_colors); vertex_colors = nullptr;
  }

  // adds a cube to the scene
unsigned AddCube ()
{
  // create a triangulated cube with 12 triangles and 8 vertices
  RTCGeometry mesh = rtcNewGeometry(_device, RTC_GEOMETRY_TYPE_TRIANGLE);
  // create face and vertex color arrays
  face_colors = (Vec3fa*) alignedMalloc(12*sizeof(Vec3fa),16);
  vertex_colors = (Vec3fa*) alignedMalloc(8*sizeof(Vec3fa),16);
  // set vertices and vertex colors
  Vertex* vertices = (Vertex*) rtcSetNewGeometryBuffer(mesh,RTC_BUFFER_TYPE_VERTEX,0,RTC_FORMAT_FLOAT3,sizeof(Vertex),8);
  vertex_colors[0] = Vec3fa(0,0,0); 
  vertices[0].x = -1; vertices[0].y = -1; vertices[0].z = -1;
  vertex_colors[1] = Vec3fa(0,0,1); vertices[1].x = -1; vertices[1].y = -1; vertices[1].z = +1;
  vertex_colors[2] = Vec3fa(0,1,0); vertices[2].x = -1; vertices[2].y = +1; vertices[2].z = -1;
  vertex_colors[3] = Vec3fa(0,1,1); vertices[3].x = -1; vertices[3].y = +1; vertices[3].z = +1;
  vertex_colors[4] = Vec3fa(1,0,0); vertices[4].x = +1; vertices[4].y = -1; vertices[4].z = -1;
  vertex_colors[5] = Vec3fa(1,0,1); vertices[5].x = +1; vertices[5].y = -1; vertices[5].z = +1;
  vertex_colors[6] = Vec3fa(1,1,0); vertices[6].x = +1; vertices[6].y = +1; vertices[6].z = -1;
  vertex_colors[7] = Vec3fa(1,1,1); vertices[7].x = +1; vertices[7].y = +1; vertices[7].z = +1;
  // set triangles and face colors
  int tri = 0;
  Triangle* triangles = (Triangle*) rtcSetNewGeometryBuffer(mesh,RTC_BUFFER_TYPE_INDEX,0,RTC_FORMAT_UINT3,sizeof(Triangle),12);

  // left side
  face_colors[tri] = Vec3fa(1,0,0); triangles[tri].v0 = 0; triangles[tri].v1 = 1; triangles[tri].v2 = 2; tri++;
  face_colors[tri] = Vec3fa(1,0,0); triangles[tri].v0 = 1; triangles[tri].v1 = 3; triangles[tri].v2 = 2; tri++;

  // right side
  face_colors[tri] = Vec3fa(0,1,0); triangles[tri].v0 = 4; triangles[tri].v1 = 6; triangles[tri].v2 = 5; tri++;
  face_colors[tri] = Vec3fa(0,1,0); triangles[tri].v0 = 5; triangles[tri].v1 = 6; triangles[tri].v2 = 7; tri++;

  // bottom side
  face_colors[tri] = Vec3fa(0.5f);  triangles[tri].v0 = 0; triangles[tri].v1 = 4; triangles[tri].v2 = 1; tri++;
  face_colors[tri] = Vec3fa(0.5f);  triangles[tri].v0 = 1; triangles[tri].v1 = 4; triangles[tri].v2 = 5; tri++;

  // top side
  face_colors[tri] = Vec3fa(1.0f);  triangles[tri].v0 = 2; triangles[tri].v1 = 3; triangles[tri].v2 = 6; tri++;
  face_colors[tri] = Vec3fa(1.0f);  triangles[tri].v0 = 3; triangles[tri].v1 = 7; triangles[tri].v2 = 6; tri++;

  // front side
  face_colors[tri] = Vec3fa(0,0,1); triangles[tri].v0 = 0; triangles[tri].v1 = 2; triangles[tri].v2 = 4; tri++;
  face_colors[tri] = Vec3fa(0,0,1); triangles[tri].v0 = 2; triangles[tri].v1 = 6; triangles[tri].v2 = 4; tri++;

  // back side
  face_colors[tri] = Vec3fa(1,1,0); triangles[tri].v0 = 1; triangles[tri].v1 = 5; triangles[tri].v2 = 3; tri++;
  face_colors[tri] = Vec3fa(1,1,0); triangles[tri].v0 = 3; triangles[tri].v1 = 5; triangles[tri].v2 = 7; tri++;

  rtcSetGeometryVertexAttributeCount(mesh,1);
  rtcSetSharedGeometryBuffer(mesh,RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,0,RTC_FORMAT_FLOAT3,vertex_colors,0,sizeof(Vec3fa),8);
  
  rtcCommitGeometry(mesh);
  unsigned int geomID = rtcAttachGeometry(_scene,mesh);
  rtcReleaseGeometry(mesh);
  return geomID;
}

// adds a ground plane to the scene 
unsigned AddGroundPlane ( )
{
  // create a triangulated plane with 2 triangles and 4 vertices 
  RTCGeometry mesh = rtcNewGeometry (_device, RTC_GEOMETRY_TYPE_TRIANGLE);

  // set vertices
  Vertex* vertices = (Vertex*) rtcSetNewGeometryBuffer(mesh,RTC_BUFFER_TYPE_VERTEX,0,RTC_FORMAT_FLOAT3,sizeof(Vertex),4);
  vertices[0].x = -10; vertices[0].y = -2; vertices[0].z = -10;
  vertices[1].x = -10; vertices[1].y = -2; vertices[1].z = +10;
  vertices[2].x = +10; vertices[2].y = -2; vertices[2].z = -10;
  vertices[3].x = +10; vertices[3].y = -2; vertices[3].z = +10;

  // set triangles 
  Triangle* triangles = (Triangle*) rtcSetNewGeometryBuffer(mesh,RTC_BUFFER_TYPE_INDEX,0,RTC_FORMAT_UINT3,sizeof(Triangle),2);
  triangles[0].v0 = 0; triangles[0].v1 = 1; triangles[0].v2 = 2;
  triangles[1].v0 = 1; triangles[1].v1 = 3; triangles[1].v2 = 2;
  
  rtcCommitGeometry(mesh);
  unsigned int geomID = rtcAttachGeometry(_scene,mesh);
  rtcReleaseGeometry(mesh);
  return geomID;
}

} // namespace embree