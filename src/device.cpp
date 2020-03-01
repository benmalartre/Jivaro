#include "device.h"
#include "utils.h"
#include "context.h"
#include "prim.h"
#include "mesh.h"
#include "widgets/viewport.h"

namespace AMN {
  
  using namespace embree;

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
  RenderTile = RenderTileStandard;
  //RenderTile = RenderTileNormal;

  return EMBREE_CTXT->_scene;
  //key_pressed_handler = device_key_pressed_default;
}

// called by the C++ code for initialization
void CommitScene ()
{ 
  // commit changes to scene
  rtcCommitScene (EMBREE_CTXT->_scene);
}

Vec3fa GetSmoothNormal(Ray& ray)
{
  UsdEmbreePrim* prim = EMBREE_CTXT->_prims[ray.geomID];
  pxr::GfVec3f smoothNormal(0.f, 1.f, 0.f);
  if(prim->_type == RTC_GEOMETRY_TYPE_TRIANGLE)
  {
    UsdEmbreeMesh* mesh = (UsdEmbreeMesh*)prim;
    pxr::VtArray<pxr::GfVec3f>& normals = mesh->_normals;
    INTERPOLATION_TYPE interpType = mesh->_normalsInterpolationType;
    if(interpType == VERTEX){
      smoothNormal = 
        normals[mesh->_triangles[ray.primID*3]] * (1 - ray.u - ray.v) + 
        normals[mesh->_triangles[ray.primID*3+1]] * ray.u + 
        normals[mesh->_triangles[ray.primID*3+2]] * ray.v;
    }
    else if(interpType == FACE_VARYING){
      smoothNormal = 
        normals[ray.primID*3] * (1 - ray.u - ray.v) + 
        normals[ray.primID*3+1] * ray.u + 
        normals[ray.primID*3+2] * ray.v;
    }
    else return ray.Ng;
  }
  return Vec3fa(smoothNormal[0], smoothNormal[1], smoothNormal[2]);
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
    UsdEmbreePrim* prim = EMBREE_CTXT->_prims[ray.geomID];
    ray.Ng = GetSmoothNormal(ray);

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
  else 
  {
    #if ENABLE_SMOOTH_NORMALS
      ray.Ng = GetSmoothNormal(ray);
    #endif
  }
  return normalize(Vec3fa(ray.Ng.x,ray.Ng.y,ray.Ng.z));
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

void RenderToMemory()
{
  
  ISPCCamera ispccamera = 
    camera.getISPCCamera(EMBREE_CTXT->_width, EMBREE_CTXT->_height);
  //initRayStats();
  DeviceRender( EMBREE_CTXT->_pixels,
                EMBREE_CTXT->_width,
                EMBREE_CTXT->_height,
                0.0f,
                ispccamera);
}

// render to viewport
void RenderToViewport(ViewportUI* viewport)
{
  RenderToMemory();
  viewport->SetPixels(EMBREE_CTXT->_width,
                      EMBREE_CTXT->_height, 
                      EMBREE_CTXT->_pixels);
}

// called by the C++ code for cleanup
void DeviceCleanup ()
{
  rtcReleaseScene (EMBREE_CTXT->_scene); EMBREE_CTXT->_scene = nullptr;
  //alignedFree(EMBREE_CTXT->_face_colors); EMBREE_CTXT->_face_colors = nullptr;
  //alignedFree(EMBREE_CTXT->_vertex_colors); EMBREE_CTXT->_vertex_colors = nullptr;
}

} // namespace embree