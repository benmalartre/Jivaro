#include "device.h"
#include "utils.h"
#include "context.h"

namespace AMN {
  using namespace embree;
// adds a cube to the scene
/*
unsigned int AddCube (RTCScene scene_i)
{
  // create a triangulated cube with 12 triangles and 8 vertices
  RTCGeometry mesh = rtcNewGeometry(g_device, RTC_GEOMETRY_TYPE_TRIANGLE);
  
  // create face and vertex color arrays
  face_colors = (Vec3fa*) alignedMalloc(12*sizeof(Vec3fa),16);
  vertex_colors = (Vec3fa*) alignedMalloc(8*sizeof(Vec3fa),16);
  
  // set vertices and vertex colors
  pxr::GfVec3f* vertices = 
    (pxr::GfVec3f*) rtcSetNewGeometryBuffer(mesh,RTC_BUFFER_TYPE_VERTEX,0,RTC_FORMAT_FLOAT3,sizeof(pxr::GfVec3f),8);
  vertex_colors[0] = Vec3fa(0,0,0); 
  vertices[0][0] = -1; vertices[0][1] = -1; vertices[0][2] = -1;
  vertex_colors[1] = Vec3fa(0,0,1); vertices[1][0] = -1; vertices[1][1] = -1; vertices[1][2] = +1;
  vertex_colors[2] = Vec3fa(0,1,0); vertices[2][0] = -1; vertices[2][1] = +1; vertices[2][2] = -1;
  vertex_colors[3] = Vec3fa(0,1,1); vertices[3][0] = -1; vertices[3][1] = +1; vertices[3][2] = +1;
  vertex_colors[4] = Vec3fa(1,0,0); vertices[4][0] = +1; vertices[4][1] = -1; vertices[4][2] = -1;
  vertex_colors[5] = Vec3fa(1,0,1); vertices[5][0] = +1; vertices[5][1] = -1; vertices[5][2] = +1;
  vertex_colors[6] = Vec3fa(1,1,0); vertices[6][0] = +1; vertices[6][1] = +1; vertices[6][2] = -1;
  vertex_colors[7] = Vec3fa(1,1,1); vertices[7][0] = +1; vertices[7][1] = +1; vertices[7][2] = +1;
  
  // set triangles and face colors
  int tri = 0;
  pxr::GfVec3i* triangles = 
    (pxr::GfVec3i*) rtcSetNewGeometryBuffer(mesh,RTC_BUFFER_TYPE_INDEX,0,RTC_FORMAT_UINT3,sizeof(pxr::GfVec3i),12);

  // left side
  face_colors[tri] = Vec3fa(1,0,0); triangles[tri][0] = 0; triangles[tri][1] = 1; triangles[tri][2] = 2; tri++;
  face_colors[tri] = Vec3fa(1,0,0); triangles[tri][0] = 1; triangles[tri][1] = 3; triangles[tri][2] = 2; tri++;

  // right side
  face_colors[tri] = Vec3fa(0,1,0); triangles[tri][0] = 4; triangles[tri][1] = 6; triangles[tri][2] = 5; tri++;
  face_colors[tri] = Vec3fa(0,1,0); triangles[tri][0] = 5; triangles[tri][1] = 6; triangles[tri][2] = 7; tri++;

  // bottom side
  face_colors[tri] = Vec3fa(0.5f);  triangles[tri][0] = 0; triangles[tri][1] = 4; triangles[tri][2] = 1; tri++;
  face_colors[tri] = Vec3fa(0.5f);  triangles[tri][0] = 1; triangles[tri][1] = 4; triangles[tri][2] = 5; tri++;

  // top side
  face_colors[tri] = Vec3fa(1.0f);  triangles[tri][0] = 2; triangles[tri][1] = 3; triangles[tri][2] = 6; tri++;
  face_colors[tri] = Vec3fa(1.0f);  triangles[tri][0] = 3; triangles[tri][1] = 7; triangles[tri][2] = 6; tri++;

  // front side
  face_colors[tri] = Vec3fa(0,0,1); triangles[tri][0] = 0; triangles[tri][1] = 2; triangles[tri][2] = 4; tri++;
  face_colors[tri] = Vec3fa(0,0,1); triangles[tri][0] = 2; triangles[tri][1] = 6; triangles[tri][2] = 4; tri++;

  // back side
  face_colors[tri] = Vec3fa(1,1,0); triangles[tri][0] = 1; triangles[tri][1] = 5; triangles[tri][2] = 3; tri++;
  face_colors[tri] = Vec3fa(1,1,0); triangles[tri][0] = 3; triangles[tri][1] = 5; triangles[tri][2] = 7; tri++;

  rtcSetGeometryVertexAttributeCount(mesh,1);
  rtcSetSharedGeometryBuffer(mesh,RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,0,RTC_FORMAT_FLOAT3,vertex_colors,0,sizeof(Vec3fa),8);
  
  rtcCommitGeometry(mesh);
  unsigned int geomID = rtcAttachGeometry(scene_i,mesh);
  rtcReleaseGeometry(mesh);
  return geomID;
}

// adds a ground plane to the scene 
unsigned int AddGroundPlane (RTCScene scene_i)
{
  // create a triangulated plane with 2 triangles and 4 vertices 
  RTCGeometry mesh = rtcNewGeometry (g_device, RTC_GEOMETRY_TYPE_TRIANGLE);

  // set vertices
  pxr::GfVec3f* vertices = (pxr::GfVec3f*) rtcSetNewGeometryBuffer(mesh, 
    RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(pxr::GfVec3f), 4);
  vertices[0][0] = -10; vertices[0][1] = -2; vertices[0][2] = -10;
  vertices[1][0] = -10; vertices[1][1] = -2; vertices[1][2] = +10;
  vertices[2][0] = +10; vertices[2][1] = -2; vertices[2][2] = -10;
  vertices[3][0] = +10; vertices[3][1] = -2; vertices[3][2] = +10;

  // set triangles 
  pxr::GfVec3i* triangles = 
    (pxr::GfVec3i*) rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_INDEX, 0,
      RTC_FORMAT_UINT3, sizeof(pxr::GfVec3i), 2);
  triangles[0][0] = 0; triangles[0][1] = 1; triangles[0][2] = 2;
  triangles[1][0] = 1; triangles[1][1] = 3; triangles[1][2] = 2;
  
  rtcCommitGeometry(mesh);
  unsigned int geomID = rtcAttachGeometry(scene_i,mesh);
  rtcReleaseGeometry(mesh);
  return geomID;
}
*/

// called by the C++ code for initialization
RTCScene DeviceInit ()
{ 
  // camera
  /* Z-Up
  camera.from = embree::Vec3fa(40.f,60.f,35.f);
  camera.to   = embree::Vec3fa(0.0f,-60.0f,0.0f);
  camera.up   = embree::Vec3fa(0.0f,-1.0f,1.0f);*/
  // Y-Up
  camera.from = embree::Vec3fa(12.f,12.f,12.f);
  camera.to   = embree::Vec3fa(0.0f,10.0f,0.0f);
  camera.up   = embree::Vec3fa(0.0f,1.0f,0.0f);

  // create device
  EMBREE_CTXT->_device = rtcNewDevice(rtcore.c_str());
  //error_handler(nullptr,rtcGetDeviceError(g_device));

  // create scene 
  EMBREE_CTXT->_scene = rtcNewScene(EMBREE_CTXT->_device);

  // add cube 
  //addCube(g_scene);

  // add ground plane
  //addGroundPlane(g_scene);


  // set start render mode
  //RenderTile = RenderTileAmbientOcclusion;
  //RenderTile = RenderTileStandard;
  RenderTile = RenderTileNormal;

  return EMBREE_CTXT->_scene;
  //key_pressed_handler = device_key_pressed_default;
}

// called by the C++ code for initialization
void CommitScene ()
{ 
  // commit changes to scene
  rtcCommitScene (EMBREE_CTXT->_scene);
}

// task that renders a single screen tile
Vec3fa RenderPixelStandard(float x, float y, const ISPCCamera& camera, RayStats& stats)
{
  RTCIntersectContext context;
  rtcInitIntersectContext(&context);
  
  // initialize ray
  Ray ray(Vec3fa(camera.xfm.p), Vec3fa(normalize(x*camera.xfm.l.vx + y*camera.xfm.l.vy + camera.xfm.l.vz)), 0.0f, inf);

  // intersect ray with scene
  rtcIntersect1(EMBREE_CTXT->_scene,&context,RTCRayHit_(ray));
  //RayStats_addRay(stats);

  // shade pixels
  Vec3fa color = Vec3fa(0.0f);
  if (ray.geomID != RTC_INVALID_GEOMETRY_ID)
  {
    //const Triangle& tri = triangles[ray.primID];
    pxr::GfVec4f rColor = AMN::UnpackColor(AMN::RandomColorByIndex(ray.primID));
    Vec3fa diffuse = Vec3fa(rColor[0], rColor[1], rColor[2]);//face_colors[ray.primID];
    color = color + diffuse*0.5f;
    Vec3fa lightDir = normalize(Vec3fa(-1,-1,-1));

    // initialize shadow ray
    Ray shadow(ray.org + ray.tfar*ray.dir, neg(lightDir), 0.001f, inf, 0.0f);

    // trace shadow ray
    rtcOccluded1(EMBREE_CTXT->_scene,&context,RTCRay_(shadow));
    //RayStats_addShadowRay(stats);

    // add light contribution
    if (shadow.tfar >= 0.0f)
      color = color + diffuse*clamp(-dot(lightDir,normalize(ray.Ng)),0.0f,1.0f);
  }
  return color;
}

// renders a single screen tile
void RenderTileStandard(int taskIndex,
                        int threadIndex,
                        int* pixels,
                        const unsigned int width,
                        const unsigned int height,
                        const float time,
                        const ISPCCamera& camera,
                        const int numTilesX,
                        const int numTilesY)
{
  const unsigned int tileY = taskIndex / numTilesX;
  const unsigned int tileX = taskIndex - tileY * numTilesX;
  const unsigned int x0 = tileX * TILE_SIZE_X;
  const unsigned int x1 = min(x0+TILE_SIZE_X,width);
  const unsigned int y0 = tileY * TILE_SIZE_Y;
  const unsigned int y1 = min(y0+TILE_SIZE_Y,height);

  for (unsigned int y=y0; y<y1; y++) for (unsigned int x=x0; x<x1; x++)
  {
    // calculate pixel color 
    Vec3fa color = RenderPixelStandard((float)x,(float)y,camera,EMBREE_CTXT->_stats[threadIndex]);

    // write color to framebuffer
    unsigned int r = (unsigned int) (255.0f * clamp(color.x,0.0f,1.0f));
    unsigned int g = (unsigned int) (255.0f * clamp(color.y,0.0f,1.0f));
    unsigned int b = (unsigned int) (255.0f * clamp(color.z,0.0f,1.0f));
    pixels[y*width+x] = (b << 16) + (g << 8) + r;
  }
}

// renders a single pixel with ambient occlusion 
Vec3fa RenderPixelAmbientOcclusion(float x, float y, const ISPCCamera& camera, RayStats& stats)
{
  // initialize ray
  Ray ray;
  ray.org = Vec3fa(camera.xfm.p);
  ray.dir = Vec3fa(normalize(x*camera.xfm.l.vx + y*camera.xfm.l.vy + camera.xfm.l.vz));
  ray.tnear() = 0.0f;
  ray.tfar = inf;
  ray.geomID = RTC_INVALID_GEOMETRY_ID;
  ray.primID = RTC_INVALID_GEOMETRY_ID;
  ray.mask = -1;
  ray.time() = EMBREE_CTXT->_debug;

  // intersect ray with scene 
  IntersectContext context;
  InitIntersectionContext(&context);
  rtcIntersect1(EMBREE_CTXT->_scene,&context.context,RTCRayHit_(ray));
  //RayStats_addRay(stats);

  // shade pixel
  if (ray.geomID == RTC_INVALID_GEOMETRY_ID) return Vec3fa(0.0f);

  Vec3fa Ng = normalize(ray.Ng);
  Vec3fa Nf = FaceForward(Ng,ray.dir,Ng);
  Vec3fa col = embree::Vec3fa(embree::min(1.f,.3f+.8f*embree::abs(embree::dot(Ng,embree::normalize(ray.dir)))));

  // calculate hit point 
  float intensity = 0;
  Vec3fa hitPos = ray.org + ray.tfar * ray.dir;

#define AMBIENT_OCCLUSION_SAMPLES 64
  // trace some ambient occlusion rays
  RandomSampler sampler;
  RandomSampler_init(sampler, (int)x, (int)y, 0);
  for (int i=0; i<AMBIENT_OCCLUSION_SAMPLES; i++)
  {
    Vec2f sample = RandomSampler_get2D(sampler);
    Sample3f dir = cosineSampleHemisphere(sample.x,sample.y,Nf);

    // initialize shadow ray
    Ray shadow;
    shadow.org = hitPos;
    shadow.dir = dir.v;
    shadow.tnear() = 0.001f;
    shadow.tfar = inf;
    shadow.geomID = RTC_INVALID_GEOMETRY_ID;
    shadow.primID = RTC_INVALID_GEOMETRY_ID;
    shadow.mask = -1;
    shadow.time() = EMBREE_CTXT->_debug;

    // trace shadow ray 
    IntersectContext context;
    InitIntersectionContext(&context);
    rtcOccluded1(EMBREE_CTXT->_scene,&context.context,RTCRay_(shadow));
    //RayStats_addShadowRay(stats);

    // add light contribution 
    if (shadow.tfar >= 0.0f)
      intensity += 1.0f;
  }
  intensity *= 1.0f/AMBIENT_OCCLUSION_SAMPLES;

  // shade pixel 
  return col * intensity;
}

void RenderTileAmbientOcclusion(int taskIndex,
                                int threadIndex,
                                int* pixels,
                                const unsigned int width,
                                const unsigned int height,
                                const float time,
                                const ISPCCamera& camera,
                                const int numTilesX,
                                const int numTilesY)
{
  const int t = taskIndex;
  const unsigned int tileY = t / numTilesX;
  const unsigned int tileX = t - tileY * numTilesX;
  const unsigned int x0 = tileX * TILE_SIZE_X;
  const unsigned int x1 = min(x0+TILE_SIZE_X,width);
  const unsigned int y0 = tileY * TILE_SIZE_Y;
  const unsigned int y1 = min(y0+TILE_SIZE_Y,height);

  for (unsigned int y=y0; y<y1; y++) for (unsigned int x=x0; x<x1; x++)
  {
    Vec3fa color = RenderPixelAmbientOcclusion((float)x,(float)y,camera,EMBREE_CTXT->_stats[threadIndex]);

    // write color to framebuffer 
    unsigned int r = (unsigned int) (255.0f * clamp(color.x,0.0f,1.0f));
    unsigned int g = (unsigned int) (255.0f * clamp(color.y,0.0f,1.0f));
    unsigned int b = (unsigned int) (255.0f * clamp(color.z,0.0f,1.0f));
    pixels[y*width+x] = (b << 16) + (g << 8) + r;
  }
}

// renders a single pixel with geometry normal shading
Vec3fa RenderPixelNormal( float x, float y, const ISPCCamera& camera, RayStats& stats)
{
  // initialize ray
  Ray ray;
  ray.org = Vec3fa(camera.xfm.p);
  ray.dir = Vec3fa(normalize(x*camera.xfm.l.vx + y*camera.xfm.l.vy + camera.xfm.l.vz));
  ray.tnear() = 0.0f;
  ray.tfar = inf;
  ray.geomID = RTC_INVALID_GEOMETRY_ID;
  ray.primID = RTC_INVALID_GEOMETRY_ID;
  ray.mask = -1;
  ray.time() = EMBREE_CTXT->_debug;

  // intersect ray with scene
  IntersectContext context;
  InitIntersectionContext(&context);
  rtcIntersect1(EMBREE_CTXT->_scene,&context.context,RTCRayHit_(ray));
  //RayStats_addRay(stats);

  // shade pixel
  if (ray.geomID == RTC_INVALID_GEOMETRY_ID) return Vec3fa(0.0f,0.0f,1.0f);
  //else return abs(normalize(Vec3fa(ray.Ng.x,ray.Ng.y,ray.Ng.z)));
  else return normalize(Vec3fa(ray.Ng.x,ray.Ng.y,ray.Ng.z));
}

void RenderTileNormal(int taskIndex,
                      int threadIndex,
                      int* pixels,
                      const unsigned int width,
                      const unsigned int height,
                      const float time,
                      const ISPCCamera& camera,
                      const int numTilesX,
                      const int numTilesY)
{
  const int t = taskIndex;
  const unsigned int tileY = t / numTilesX;
  const unsigned int tileX = t - tileY * numTilesX;
  const unsigned int x0 = tileX * TILE_SIZE_X;
  const unsigned int x1 = min(x0+TILE_SIZE_X,width);
  const unsigned int y0 = tileY * TILE_SIZE_Y;
  const unsigned int y1 = min(y0+TILE_SIZE_Y,height);

  for (unsigned int y=y0; y<y1; y++) for (unsigned int x=x0; x<x1; x++)
  {
    Vec3fa color = RenderPixelNormal((float)x,(float)y,camera,EMBREE_CTXT->_stats[threadIndex]);

    /* write color to framebuffer */
    unsigned int r = (unsigned int) (255.0f * clamp(color.x,0.0f,1.0f));
    unsigned int g = (unsigned int) (255.0f * clamp(color.y,0.0f,1.0f));
    unsigned int b = (unsigned int) (255.0f * clamp(color.z,0.0f,1.0f));
    pixels[y*width+x] = (b << 16) + (g << 8) + r;
  }
}

// task that renders a single screen tile
void RenderTileTask (int taskIndex, int threadIndex, int* pixels,
                    const unsigned int width,
                    const unsigned int height,
                    const float time,
                    const ISPCCamera& camera,
                    const int numTilesX,
                    const int numTilesY)
{
  RenderTile(taskIndex,threadIndex,pixels,width,height,time,camera,numTilesX,numTilesY);
}

// called by the C++ code to render
void DeviceRender (int* pixels,
                  const unsigned int width,
                  const unsigned int height,
                  const float time,
                  const ISPCCamera& camera)
{
  const int numTilesX = (width +TILE_SIZE_X-1)/TILE_SIZE_X;
  const int numTilesY = (height+TILE_SIZE_Y-1)/TILE_SIZE_Y;
  parallel_for(size_t(0),size_t(numTilesX*numTilesY),[&](const range<size_t>& range) {
    const int threadIndex = (int)TaskScheduler::threadIndex();
    for (size_t i=range.begin(); i<range.end(); i++)
      RenderTileTask((int)i,threadIndex,pixels,width,height,time,camera,numTilesX,numTilesY);
  }); 
}

void RenderToFile(const FileName& fileName)
{
  
  ISPCCamera ispccamera = 
    camera.getISPCCamera(EMBREE_CTXT->_width, EMBREE_CTXT->_height);
  //initRayStats();
  std::cout << "LAUNCH RENDER THREAD..." << std::endl;
  DeviceRender( EMBREE_CTXT->_pixels,
                EMBREE_CTXT->_width,
                EMBREE_CTXT->_height,
                0.0f,
                ispccamera);
  Ref<Image> image = new Image4uc(EMBREE_CTXT->_width, 
                                  EMBREE_CTXT->_height, 
                                  (Col4uc*)EMBREE_CTXT->_pixels);
  storeImage(image, fileName);
}

// called by the C++ code for cleanup
void DeviceCleanup ()
{
  rtcReleaseScene (EMBREE_CTXT->_scene); EMBREE_CTXT->_scene = nullptr;
  //alignedFree(EMBREE_CTXT->_face_colors); EMBREE_CTXT->_face_colors = nullptr;
  //alignedFree(EMBREE_CTXT->_vertex_colors); EMBREE_CTXT->_vertex_colors = nullptr;
}

} // namespace embree