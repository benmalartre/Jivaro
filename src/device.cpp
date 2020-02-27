#include "device.h"
#include "context.h"

namespace embree {

// task that renders a single screen tile
Vec3fa renderPixelStandard(UsdEmbreeContext& ctxt, float x, float y, 
  const ISPCCamera& camera, RayStats& stats)
{
  RTCIntersectContext context;
  rtcInitIntersectContext(&context);
  
  // initialize ray
  Ray ray(Vec3fa(camera.xfm.p), Vec3fa(normalize(x*camera.xfm.l.vx + 
    y*camera.xfm.l.vy + camera.xfm.l.vz)), 0.0f, inf);

  // intersect ray with scene
  rtcIntersect1(ctxt._scene,&context,RTCRayHit_(ray));
  RayStats_addRay(stats);

  // shade pixels
  Vec3fa color = Vec3fa(0.0f);
  if (ray.geomID != RTC_INVALID_GEOMETRY_ID)
  {
    Vec3fa diffuse = ctxt.face_colors[ray.primID];
    color = color + diffuse*0.5f;
    Vec3fa lightDir = normalize(Vec3fa(-1,-1,-1));

    // initialize shadow ray
    Ray shadow(ray.org + ray.tfar*ray.dir, neg(lightDir), 0.001f, inf, 0.0f);

    // trace shadow ray
    rtcOccluded1(scene,&context,RTCRay_(shadow));
    RayStats_addShadowRay(stats);

    // add light contribution
    if (shadow.tfar >= 0.0f)
      color = color + diffuse*clamp(-dot(lightDir,normalize(ray.Ng)),0.0f,1.0f);
  }
  return color;
}

// renders a single screen tile
void renderTileStandard(RTCScene scene,
                        int taskIndex,
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
    Vec3fa color = renderPixelStandard((float)x,(float)y,camera,g_stats[threadIndex]);

    // write color to framebuffer
    unsigned int r = (unsigned int) (255.0f * clamp(color.x,0.0f,1.0f));
    unsigned int g = (unsigned int) (255.0f * clamp(color.y,0.0f,1.0f));
    unsigned int b = (unsigned int) (255.0f * clamp(color.z,0.0f,1.0f));
    pixels[y*width+x] = (b << 16) + (g << 8) + r;
  }
}

// renders a single pixel with ambient occlusion 
Vec3fa renderPixelAmbientOcclusion(RTCScene scene, float x, float y,
  const ISPCCamera& camera, RayStats& stats)
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
  //ray.time() = g_debug;

  // intersect ray with scene 
  IntersectContext context;
  InitIntersectionContext(&context);
  rtcIntersect1(scene,&context.context,RTCRayHit_(ray));
  RayStats_addRay(stats);

  // shade pixel
  if (ray.geomID == RTC_INVALID_GEOMETRY_ID) return Vec3fa(0.0f);

  Vec3fa Ng = normalize(ray.Ng);
  Vec3fa Nf = faceforward(Ng,ray.dir,Ng);
  Vec3fa col = Vec3fa(min(1.f,.3f+.8f*abs(dot(Ng,normalize(ray.dir)))));

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
    //shadow.time() = g_debug;

    // trace shadow ray 
    IntersectContext context;
    InitIntersectionContext(&context);
    rtcOccluded1(scene,&context.context,RTCRay_(shadow));
    RayStats_addShadowRay(stats);

    // add light contribution 
    if (shadow.tfar >= 0.0f)
      intensity += 1.0f;
  }
  intensity *= 1.0f/AMBIENT_OCCLUSION_SAMPLES;

  // shade pixel 
  return col * intensity;
}

void renderTileAmbientOcclusion(RTCScene scene,
                                int taskIndex,
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
    Vec3fa color = 
      renderPixelAmbientOcclusion(scene, (float)x, (float)y, 
        camera, g_stats[threadIndex]);

    // write color to framebuffer 
    unsigned int r = (unsigned int) (255.0f * clamp(color.x,0.0f,1.0f));
    unsigned int g = (unsigned int) (255.0f * clamp(color.y,0.0f,1.0f));
    unsigned int b = (unsigned int) (255.0f * clamp(color.z,0.0f,1.0f));
    pixels[y*width+x] = (b << 16) + (g << 8) + r;
  }
}

// task that renders a single screen tile
void renderTileTask (RTCScene scene,
                    int taskIndex, 
                    int threadIndex, 
                    int* pixels,
                    const unsigned int width,
                    const unsigned int height,
                    const float time,
                    const ISPCCamera& camera,
                    const int numTilesX,
                    const int numTilesY)
{
  renderTile(scene, taskIndex, threadIndex, pixels, width,
              height, time, camera, numTilesX, numTilesY);
}

// called by the C++ code to render
void device_render (RTCScene scene,
                    int* pixels,
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
      renderTileTask(scene, (int)i, threadIndex, pixels, width, height, 
        time, camera, numTilesX, numTilesY);
  }); 
}

void renderToFile(RTCScene scene, Camera camera, const FileName& fileName)
{
  int* pixels = (int*) alignedMalloc(WIDTH*HEIGHT*sizeof(int),64);
  ISPCCamera ispccamera = camera.getISPCCamera(WIDTH,HEIGHT);
  //initRayStats();
  device_render(scene, 
                pixels, 
                (const unsigned)WIDTH, 
                (const unsigned)HEIGHT, 
                0.0f, 
                ispccamera);
  Ref<Image> image = new Image4uc(WIDTH, HEIGHT, (Col4uc*)pixels);
  storeImage(image, fileName);
}


} // namespace embree