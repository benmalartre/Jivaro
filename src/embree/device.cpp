#include "device.h"
#include "context.h"
#include "prim.h"
#include "mesh.h"
#include "instance.h"
#include "../app/camera.h"
#include "../utils/utils.h"
#include "../widgets/viewport.h"

JVR_NAMESPACE_OPEN_SCOPE

void SetEmbreeContext(UsdEmbreeContext* ctxt)
{
  EMBREE_CTXT = ctxt;
}

using namespace embree;

// called by the C++ code for initialization
RTCScene DeviceInit (Camera* camera)
{ 
  // camera
  /* Z-Up
  camera.from = embree::Vec3fa(40.f,60.f,35.f);
  camera.to   = embree::Vec3fa(0.0f,-60.0f,0.0f);
  camera.up   = embree::Vec3fa(0.0f,-1.0f,1.0f);
  // Y-Up
  camera->from = embree::Vec3fa(12.f,12.f,12.f);
  camera->to   = embree::Vec3fa(0.0f,10.0f,0.0f);
  camera->up   = embree::Vec3fa(0.0f,1.0f,0.0f);
  */  
  // create device
  EMBREE_CTXT->_device = rtcNewDevice(rtcore.c_str());
  //error_handler(nullptr,rtcGetDeviceError(g_device));

  // create rendering scene 
  EMBREE_CTXT->_scene = rtcNewScene(EMBREE_CTXT->_device);
  rtcSetSceneBuildQuality(EMBREE_CTXT->_scene,RTC_BUILD_QUALITY_LOW);
  rtcSetSceneFlags(EMBREE_CTXT->_scene,RTC_SCENE_FLAG_DYNAMIC);

  // set start render mode
  RenderTile = RenderTileStandard;
  //RenderTile = RenderTileNormal;
  //RenderTile = RenderTileAmbientOcclusion;
  
  return EMBREE_CTXT->_scene;
  //key_pressed_handler = device_key_pressed_default;
}

// called by the C++ code for initialization
void CommitScene ()
{ 
  // commit changes to scene
  rtcCommitScene (EMBREE_CTXT->_scene);
}

Vec3fa _GetMeshNormal(Ray& ray)
{
  UsdEmbreePrim* prim = EMBREE_CTXT->_prims[ray.geomID];
  GfVec3f normal(0.f, 1.f, 0.f);
  if(prim->_type == RTC_GEOMETRY_TYPE_TRIANGLE)
  {
    UsdEmbreeMesh* mesh = (UsdEmbreeMesh*)prim;
    VtArray<GfVec3f>& normals = mesh->_normals;
    JVR_INTERPOLATION_TYPE interpType = mesh->_normalsInterpolationType;
    if(interpType == VERTEX){
      normal = 
        normals[mesh->_triangles[ray.primID*3]] * (1 - ray.u - ray.v) + 
        normals[mesh->_triangles[ray.primID*3+1]] * ray.u + 
        normals[mesh->_triangles[ray.primID*3+2]] * ray.v;
    }
    else if(interpType == FACE_VARYING){
      normal = 
        normals[ray.primID*3] * (1 - ray.u - ray.v) + 
        normals[ray.primID*3+1] * ray.u + 
        normals[ray.primID*3+2] * ray.v;
    }
    else return ray.Ng;
  }
  normal.Normalize();
  return Vec3fa(normal[0], normal[1], normal[2]);
}

GfVec3f _GetMeshColor(Ray& ray)
{
  UsdEmbreePrim* prim = EMBREE_CTXT->_prims[ray.geomID];
  GfVec3f color(1.f, 1.f, 1.f);
  if(prim->_type == RTC_GEOMETRY_TYPE_TRIANGLE)
  {
    UsdEmbreeMesh* mesh = (UsdEmbreeMesh*)prim;
    VtArray<GfVec3f>& colors = mesh->_colors;
    JVR_INTERPOLATION_TYPE interpType = mesh->_colorsInterpolationType;
    if(interpType == CONSTANT) {
      if(!colors.size())color = GfVec3f(1,0,0);
      else color = colors[0];
    }
    else if(interpType == VERTEX) {
      color = 
        colors[mesh->_triangles[ray.primID*3]] * (1 - ray.u - ray.v) + 
        colors[mesh->_triangles[ray.primID*3+1]] * ray.u + 
        colors[mesh->_triangles[ray.primID*3+2]] * ray.v;
    }
    else if(interpType == FACE_VARYING) {
      color = 
        colors[ray.primID*3] * (1 - ray.u - ray.v) + 
        colors[ray.primID*3+1] * ray.u + 
        colors[ray.primID*3+2] * ray.v;
    }
    else return color;
  }
  return color;
}

Vec3fa _GetSubdivNormal(Ray& ray)
{
  Vec3fa dPdu,dPdv;
  rtcInterpolate1(
    EMBREE_CTXT->_prims[ray.geomID]->_geom,
    ray.primID,
    ray.u,
    ray.v,
    RTC_BUFFER_TYPE_VERTEX,
    0,
    nullptr,
    &dPdu.x,
    &dPdv.x,
    3
  );
  ray.Ng = normalize(cross(dPdu,dPdv));
}

GfVec3f _GetSubdivColor(Ray& ray)
{
  Vec3fa C;
  UsdEmbreePrim* prim = EMBREE_CTXT->_prims[ray.geomID];

  GfVec3f color(1.f, 1.f, 1.f);
  if(prim->_type == RTC_GEOMETRY_TYPE_SUBDIVISION)
  {
    UsdEmbreeSubdiv* mesh = (UsdEmbreeSubdiv*)prim;
    VtArray<GfVec3f>& colors = mesh->_colors;
    JVR_INTERPOLATION_TYPE interpType = mesh->_colorsInterpolationType;
    if(interpType == CONSTANT) {
      if(!colors.size())color = GfVec3f(1,0,0);
      else color = colors[0];
    }
    else if(interpType == VERTEX){
    
      color = 
        colors[ray.primID*3] * (1 - ray.u - ray.v) + 
        colors[ray.primID*3+1] * ray.u + 
        colors[ray.primID*3+2] * ray.v;
    
     color = GfVec3f(1.f,0.7f, 0.3f);
    }
    else if(interpType == FACE_VARYING){
      color = GfVec3f(1.f,0.7f, 0.3f);
      
      color = 
        colors[ray.primID*3] * (1 - ray.u - ray.v) + 
        colors[ray.primID*3+1] * ray.u + 
        colors[ray.primID*3+2] * ray.v;
      
    }
    else return color;
  }
  return color;
}

// task that renders a single screen tile
Vec3fa RenderPixelStandard(float x, float y, const Camera* camera)
{
  RTCIntersectContext context;
  rtcInitIntersectContext(&context);
  
  // initialize ray
  GfRay gfRay = camera->ComputeRay(GfVec2d(x, y));

  Ray ray(
    pxr2embree(gfRay.GetStartPoint()),
    pxr2embree(gfRay.GetDirection()), 
    0.0f, 
    inf
  );

  // intersect ray with scene
  rtcIntersect1(EMBREE_CTXT->_scene,&context,RTCRayHit_(ray));

  // shade pixels
  Vec3fa color = Vec3fa(0.0f);
  if (ray.geomID != RTC_INVALID_GEOMETRY_ID)
  {

    UsdEmbreePrim* prim = EMBREE_CTXT->_prims[ray.geomID];
    GfVec3f rColor(
      (float)prim->_color[0],
      (float)prim->_color[1],
      (float)prim->_color[2]
    );
    
    if(prim->_type == RTC_GEOMETRY_TYPE_TRIANGLE)
    {
      ray.Ng = _GetMeshNormal(ray);
      rColor = _GetMeshColor(ray);
    }
      
    else if(prim->_type == RTC_GEOMETRY_TYPE_SUBDIVISION)
    {
      ray.Ng = _GetSubdivNormal(ray);
      rColor = _GetSubdivColor(ray); 
    }

    if (ray.instID[0] != RTC_INVALID_GEOMETRY_ID)
    {
      UsdEmbreeInstance* instance = (UsdEmbreeInstance*)EMBREE_CTXT->_prims[ray.instID[0]];
      rColor = instance->_color;
    }
  
    Vec3fa diffuse = Vec3fa(rColor[0], rColor[1], rColor[2]);
    color = color + diffuse*0.5f;
    Vec3fa lightDir = normalize(Vec3fa(1,-1,1));

    // initialize shadow ray
    Ray shadow(ray.org + ray.tfar*ray.dir, neg(lightDir), 0.001f, inf, 0.0f);

    // trace shadow ray
    rtcOccluded1(EMBREE_CTXT->_scene,&context,RTCRay_(shadow));

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
                        const Camera* camera,
                        const int numTilesX,
                        const int numTilesY)
{
  const unsigned int tileY = taskIndex / numTilesX;
  const unsigned int tileX = taskIndex - tileY * numTilesX;
  const unsigned int x0 = tileX * TILE_SIZE_X;
  const unsigned int x1 = min(x0+TILE_SIZE_X,width);
  const unsigned int y0 = tileY * TILE_SIZE_Y;
  const unsigned int y1 = min(y0+TILE_SIZE_Y,height);

  GfVec2f ratio = _GetDeviceRatio(width, height);

  for (unsigned int y=y0; y<y1; y++) for (unsigned int x=x0; x<x1; x++)
  {
    // calculate pixel color 
    Vec3fa color = RenderPixelStandard(
                    _GetNormalizedDeviceX(x, width, ratio[0]),
                    _GetNormalizedDeviceY(y, height, ratio[1]),
                    camera
                  );

    // write color to framebuffer
    unsigned int r = (unsigned int) (255.0f * clamp(color.x,0.0f,1.0f));
    unsigned int g = (unsigned int) (255.0f * clamp(color.y,0.0f,1.0f));
    unsigned int b = (unsigned int) (255.0f * clamp(color.z,0.0f,1.0f));
    pixels[y*width+x] = (b << 16) + (g << 8) + r;
  }
}

// renders a single pixel with ambient occlusion 
Vec3fa RenderPixelAmbientOcclusion(float x, float y, const Camera* camera)
{
  // initialize ray
  GfRay gfRay= camera->ComputeRay(GfVec2d(x, y));
  Ray ray;
  ray.org = pxr2embree(gfRay.GetStartPoint());
  ray.dir = pxr2embree(gfRay.GetDirection());
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

  // shade pixel
  if (ray.geomID == RTC_INVALID_GEOMETRY_ID) return Vec3fa(0.0f);
  UsdEmbreePrim* prim = EMBREE_CTXT->_prims[ray.geomID];
  Vec3fa Ng = ray.Ng;

  if(prim->_type == RTC_GEOMETRY_TYPE_TRIANGLE)
    Ng = _GetMeshNormal(ray);
  
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
    Sample3f dir = cosineSampleHemisphere(sample.x,sample.y,Ng);

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
                                const Camera* camera,
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

  GfVec2f ratio = _GetDeviceRatio(width, height);

  for (unsigned int y=y0; y<y1; y++) for (unsigned int x=x0; x<x1; x++)
  {
    Vec3fa color = 
      RenderPixelAmbientOcclusion(
        _GetNormalizedDeviceX(x, width, ratio[0]),
        _GetNormalizedDeviceY(y, height, ratio[1]),
        camera
      );

    // write color to framebuffer 
    unsigned int r = (unsigned int) (255.0f * clamp(color.x,0.0f,1.0f));
    unsigned int g = (unsigned int) (255.0f * clamp(color.y,0.0f,1.0f));
    unsigned int b = (unsigned int) (255.0f * clamp(color.z,0.0f,1.0f));
    pixels[y*width+x] = (b << 16) + (g << 8) + r;
  }
}

// renders a single pixel with geometry normal shading
Vec3fa RenderPixelNormal( float x, float y, const Camera* camera)
{
  // initialize ray
  GfRay gfRay = camera->ComputeRay(GfVec2d(x, y));
  Ray ray;
  ray.org = pxr2embree(gfRay.GetStartPoint());
  ray.dir = pxr2embree(gfRay.GetDirection());
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

  // shade pixel
  if (ray.geomID == RTC_INVALID_GEOMETRY_ID) return Vec3fa(0.0f,0.0f,0.0f);
  //else return abs(normalize(Vec3fa(ray.Ng.x,ray.Ng.y,ray.Ng.z)));
  else 
  {
    UsdEmbreePrim* prim = EMBREE_CTXT->_prims[ray.geomID];
    if(prim->_type == RTC_GEOMETRY_TYPE_TRIANGLE)
      ray.Ng = _GetMeshNormal(ray);
  }
  return normalize(Vec3fa(ray.Ng.x,ray.Ng.y,ray.Ng.z));
}

void RenderTileNormal(int taskIndex,
                      int threadIndex,
                      int* pixels,
                      const unsigned int width,
                      const unsigned int height,
                      const float time,
                      const Camera* camera,
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

  GfVec2f ratio = _GetDeviceRatio(width, height);

  for (unsigned int y=y0; y<y1; y++) for (unsigned int x=x0; x<x1; x++)
  {
    Vec3fa color = 
      RenderPixelNormal(
        _GetNormalizedDeviceX(x, width, ratio[0]),
        _GetNormalizedDeviceY(y, height, ratio[1]),
        camera
      );

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
                    const Camera* camera,
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
                  const Camera* camera)
{
  const int numTilesX = (width +TILE_SIZE_X-1)/TILE_SIZE_X;
  const int numTilesY = (height+TILE_SIZE_Y-1)/TILE_SIZE_Y;

  parallel_for(size_t(0),size_t(numTilesX*numTilesY),[&](const range<size_t>& range) {
    const int threadIndex = (int)TaskScheduler::threadIndex();
    for (size_t i=range.begin(); i<range.end(); i++)
      RenderTileTask((int)i,threadIndex,pixels,width,height,time,camera,numTilesX,numTilesY);
  }); 
  /*
  std::cout << "PIXELS : " << pixels << std::endl;
  std::cout << "WIDTH : " << width << std::endl;
  std::cout << "HEIGHT : " << height << std::endl;
  std::cout << "CAMERA : " << camera << std::endl;
  std::cout << "TILE X : " << numTilesX << std::endl;
  std::cout << "TILE Y : " << numTilesY << std::endl;
  for(int i=0;i<numTilesX*numTilesY;++i)
  {
    RenderTileTask((int)i,0,pixels,width,height,time,camera,numTilesX,numTilesY);
  }
  */
}

// render to file
void RenderToFile(const FileName& fileName, Camera* camera, int width, int height)
{
  int* pixels = (int*) embree::alignedMalloc(width * height * sizeof(int), 64);
  DeviceRender( pixels, width, height, 0.0f, camera);
  Ref<Image> image = new Image4uc(width, 
                                  height, 
                                  (Col4uc*)pixels);
  storeImage(image, fileName);
  embree::alignedFree(pixels);
}

// save to file
void SaveToFile(const FileName& fileName)
{
  Ref<Image> image = new Image4uc(EMBREE_CTXT->_width, 
                                  EMBREE_CTXT->_height, 
                                  (Col4uc*)EMBREE_CTXT->_pixels);
  storeImage(image, fileName);
}

// render to memory
void RenderToMemory(Camera* camera, bool interact)
{
  int width = EMBREE_CTXT->_width;
  int height = EMBREE_CTXT->_height;
  
  std::cout << "EMBREE CONTEXT : " << EMBREE_CTXT << std::endl;
  std::cout << "EMBREE PIXELS : " << EMBREE_CTXT->_pixels << std::endl;
  std::cout << "WIDTH : " << width << " vs " << EMBREE_CTXT->_width << std::endl;
  std::cout << "HEIGHT : " << height << " vs " << EMBREE_CTXT->_height << std::endl;
  std::cout << "CAMERA : " << camera << std::endl;
  
  if(interact)
  {

    DeviceRender( EMBREE_CTXT->_lowPixels,
                  width>>4,
                  height>>4,
                  0.0f,
                  camera);
  }
  else
  {
    
    DeviceRender( EMBREE_CTXT->_pixels,
                  width,
                  height,
                  0.0f,
                  camera);
  }
  
}

// called by the C++ code for cleanup
void DeviceCleanup ()
{
  rtcReleaseScene (EMBREE_CTXT->_scene); EMBREE_CTXT->_scene = NULL;
}

JVR_NAMESPACE_CLOSE_SCOPE