#include "mesh.h"
#include "utils.h"

namespace embree {


  // adds a cube to the scene
UsdEmbreeMesh* 
TranslateMesh (RTCDevice& device, RTCScene& scene, 
  const pxr::UsdGeomMesh& usdMesh, double time)
{
  size_t num_vertices, num_triangles;
  UsdEmbreeMesh* result = new UsdEmbreeMesh();
  result->_mesh = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
  std::cout << "MESH : "<<result->_mesh << std::endl;
  bool hasPoints = false;
  pxr::UsdAttribute pointsAttr = usdMesh.GetPointsAttr();
  if(pointsAttr && pointsAttr.HasAuthoredValue())
  {
    pointsAttr.Get(&result->_positions, time);
    num_vertices = result->_positions.size();
    std::cout <<"POINTS START"<<std::endl;
    Vertex2* vertices = (Vertex2*) rtcSetNewGeometryBuffer(result->_mesh,
      RTC_BUFFER_TYPE_VERTEX,0,RTC_FORMAT_FLOAT3,sizeof(Vertex2),num_vertices);
    std::cout <<"MEM ALLOCATED"<< vertices <<std::endl;
    std::cout << num_vertices << " vs " << result->_positions.size() << std::endl;
    for(int p=0;p<num_vertices;++p)
    {
      vertices[p].x = 0;//result->_positions[p][0];
      vertices[p].y = 0;//result->_positions[p][1];
      vertices[p].z = 0;//result->_positions[p][2];
    }
    std::cout <<"POINTS PASSED"<<std::endl;
    /*
    rtcSetSharedGeometryBuffer(result->_mesh,             // RTCGeometry
                              RTC_BUFFER_TYPE_VERTEX,     // RTCBufferType
                              0,                          // Slot
                              RTC_FORMAT_FLOAT3,          // RTCFormat
                              &result->_positions[0][0],  // Datas Ptr
                              0,                          // Offset
                              0,                          // Stride
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
    num_triangles = TriangulateMesh(counts, indices, result->_triangles);

    std::cout <<"INDICES START"<<std::endl;
    Triangle2* triangles = 
    (Triangle2*) rtcSetNewGeometryBuffer(result->_mesh, RTC_BUFFER_TYPE_INDEX, 0, 
      RTC_FORMAT_UINT3,sizeof(Triangle2),num_triangles);
    for(int i=0;i<num_triangles;++i)
    {
      triangles[i].v0 = result->_triangles[i*3];
      triangles[i].v1 = result->_triangles[i*3+1];
      triangles[i].v2 = result->_triangles[i*3+2];
    }
    /*
    rtcSetSharedGeometryBuffer(result->_mesh,             // RTCGeometry
                              RTC_BUFFER_TYPE_INDEX,      // RTCBufferType
                              0,                          // Slot
                              RTC_FORMAT_UINT3,           // RTCFormat
                              &result->_triangles[0],     // Datas Ptr
                              0,                          // Offset
                              0,                          // Stride
                              num_triangles);             // Num Elements*/
    hasTriangles = true;
  }
  std::cout <<"INDICES PASSED"<<std::endl;
  Vec3fa* vertex_colors = (Vec3fa*) alignedMalloc(num_vertices*sizeof(Vec3fa),16);
  for(int v=0;v<num_vertices;++v)
  {
    vertex_colors[v][0] = RANDOM_0_1;
    vertex_colors[v][1] = RANDOM_0_1;
    vertex_colors[v][2] = RANDOM_0_1;
  }

  if(hasPoints && hasTriangles)
  {
    std::cout << "COMMIT FUCKIN GEOM !!!" << std::endl;
    //rtcSetGeometryVertexAttributeCount(result->_mesh, 1);
    //rtcSetSharedGeometryBuffer(result->_mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, 
    //RTC_FORMAT_FLOAT3,vertex_colors,0,sizeof(Vec3fa),num_vertices);
  
    rtcCommitGeometry(result->_mesh);
    result->_geomId = rtcAttachGeometry(scene, result->_mesh);
    //rtcReleaseGeometry(result->_mesh);
    return result;
  }
  else return NULL;

  /*
  // create a triangulated cube with 12 triangles and 8 vertices
  RTCGeometry mesh = rtcNewGeometry(g_device, RTC_GEOMETRY_TYPE_TRIANGLE);
  // create face and vertex color arrays
  face_colors = (Vec3fa*) alignedMalloc(12*sizeof(Vec3fa),16);
  vertex_colors = (Vec3fa*) alignedMalloc(8*sizeof(Vec3fa),16);
  // set vertices and vertex colors
  Vertex* vertices = 
    (Vertex*) rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX, 0, 
      RTC_FORMAT_FLOAT3,sizeof(Vertex),8);
  vertex_colors[0] = Vec3fa(0,0,0); 
  vertices[0].x = -1; vertices[0].y = -1; vertices[0].z = -1;
  vertex_colors[1] = Vec3fa(0,0,1); 
  vertices[1].x = -1; vertices[1].y = -1; vertices[1].z = +1;
  vertex_colors[2] = Vec3fa(0,1,0); 
  vertices[2].x = -1; vertices[2].y = +1; vertices[2].z = -1;
  vertex_colors[3] = Vec3fa(0,1,1); 
  vertices[3].x = -1; vertices[3].y = +1; vertices[3].z = +1;
  vertex_colors[4] = Vec3fa(1,0,0); 
  vertices[4].x = +1; vertices[4].y = -1; vertices[4].z = -1;
  vertex_colors[5] = Vec3fa(1,0,1); 
  vertices[5].x = +1; vertices[5].y = -1; vertices[5].z = +1;
  vertex_colors[6] = Vec3fa(1,1,0); 
  vertices[6].x = +1; vertices[6].y = +1; vertices[6].z = -1;
  vertex_colors[7] = Vec3fa(1,1,1); 
  vertices[7].x = +1; vertices[7].y = +1; vertices[7].z = +1;
  // set triangles and face colors
  int tri = 0;
  Triangle* triangles = 
    (Triangle*) rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_INDEX, 0, 
      RTC_FORMAT_UINT3,sizeof(Triangle),12);

  // left side
  face_colors[tri] = Vec3fa(1,0,0); 
  triangles[tri].v0 = 0; triangles[tri].v1 = 1; triangles[tri].v2 = 2; tri++;
  face_colors[tri] = Vec3fa(1,0,0); 
  triangles[tri].v0 = 1; triangles[tri].v1 = 3; triangles[tri].v2 = 2; tri++;

  // right side
  face_colors[tri] = Vec3fa(0,1,0); 
  triangles[tri].v0 = 4; triangles[tri].v1 = 6; triangles[tri].v2 = 5; tri++;
  face_colors[tri] = Vec3fa(0,1,0); 
  triangles[tri].v0 = 5; triangles[tri].v1 = 6; triangles[tri].v2 = 7; tri++;

  // bottom side
  face_colors[tri] = Vec3fa(0.5f);  
  triangles[tri].v0 = 0; triangles[tri].v1 = 4; triangles[tri].v2 = 1; tri++;
  face_colors[tri] = Vec3fa(0.5f);  
  triangles[tri].v0 = 1; triangles[tri].v1 = 4; triangles[tri].v2 = 5; tri++;

  // top side
  face_colors[tri] = Vec3fa(1.0f);  
  triangles[tri].v0 = 2; triangles[tri].v1 = 3; triangles[tri].v2 = 6; tri++;
  face_colors[tri] = Vec3fa(1.0f);  
  triangles[tri].v0 = 3; triangles[tri].v1 = 7; triangles[tri].v2 = 6; tri++;

  // front side
  face_colors[tri] = Vec3fa(0,0,1); 
  triangles[tri].v0 = 0; triangles[tri].v1 = 2; triangles[tri].v2 = 4; tri++;
  face_colors[tri] = Vec3fa(0,0,1); 
  triangles[tri].v0 = 2; triangles[tri].v1 = 6; triangles[tri].v2 = 4; tri++;

  // back side
  face_colors[tri] = Vec3fa(1,1,0); 
  triangles[tri].v0 = 1; triangles[tri].v1 = 5; triangles[tri].v2 = 3; tri++;
  face_colors[tri] = Vec3fa(1,1,0); 
  triangles[tri].v0 = 3; triangles[tri].v1 = 5; triangles[tri].v2 = 7; tri++;

  rtcSetGeometryVertexAttributeCount(mesh,1);
  rtcSetSharedGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, 
    RTC_FORMAT_FLOAT3,vertex_colors,0,sizeof(Vec3fa),8);
  
  rtcCommitGeometry(mesh);
  unsigned int geomID = rtcAttachGeometry(scene_i,mesh);
  rtcReleaseGeometry(mesh);
  return geomID;
  */
 return 0;
}

int TriangulateMesh(const pxr::VtArray<int>& counts, 
                    const pxr::VtArray<int>& indices, 
                    pxr::VtArray<int>& triangles)
{
  int num_triangles = 0;
  for(auto count : counts)num_triangles += count - 2;

  triangles.resize(num_triangles * 3);
  int base = 0;
  int cnt = 0;
  for(auto count: counts)
  {
    for(int i = 1; i < count - 1; ++i)
    {
      triangles[cnt++] = indices[base + i    ];
      triangles[cnt++] = indices[base + i + 1];
      triangles[cnt++] = indices[base        ];
      std::cout << "TRIANGLE : " << indices[base+i] << "," << indices[base+i+1] << "," << indices[base] << std::endl;
    }
    base += count;
  }
  return cnt / 3;
}

} // namespace embree