
#pragma once

// include embree API
#include <embree3/rtcore.h>
#include <math/math.h>
#include <math/vec.h>
#include <math/vec3.h>
#include <math/vec3fa.h>
#include <math/vec3ba.h>
#include <math/sampling.h>
#include <math/random_sampler.h>
#include <sys/filename.h>
#include <image/image.h>
#include <algorithms/parallel_for.h>
#include <tasking/taskschedulertbb.h>

#include "../default.h"
#include "../app/camera.h"
#include "ray.h"
#include "context.h"

AMN_NAMESPACE_OPEN_SCOPE
class AmnViewportUI;
class AmnCamera;

extern AmnUsdEmbreeContext* EMBREE_CTXT;

#define TILE_SIZE_X 8
#define TILE_SIZE_Y 8
#define ENABLE_SMOOTH_NORMALS 1

static std::string rtcore("start_threads=1,set_affinity=1");

// face forward for shading normals
inline embree::Vec3fa FaceForward(const embree::Vec3fa& N, 
                                  const embree::Vec3fa& I, 
                                  const embree::Vec3fa& Ng ) 
{
  embree::Vec3fa NN = N; return embree::dot(I, Ng) < 0 ? NN : embree::neg(NN);
}

// device management
RTCScene DeviceInit (AmnCamera* camera);
void CommitScene ();
void DeviceCleanup ();

inline pxr::GfVec2f _GetDeviceRatio(int width, int height)
{
  if(width>height)
    return pxr::GfVec2f(1.f, static_cast<float>(height) / static_cast<float>(width));
  else
    return pxr::GfVec2f(static_cast<float>(width) / static_cast<float>(height), 1.f);
}

inline float _GetNormalizedDeviceX(int x, int width, float ratio)
{
  return 2.f*(static_cast<float>(x)/static_cast<float>(width))-1.f * ratio;
}

inline float _GetNormalizedDeviceY(int y, int height, float ratio)
{
  return 1.f - 2.f*(static_cast<float>(y)/static_cast<float>(height)) * ratio;
}

// render tile function prototype
typedef void (* RenderTileFunc)(int taskIndex,
                                int threadIndex,
                                int* pixels,
                                const unsigned int width,
                                const unsigned int height,
                                const float time,
                                const AmnCamera* camera,
                                const int numTilesX,
                                const int numTilesY);
static RenderTileFunc RenderTile;

// task that renders a single screen tile
embree::Vec3fa RenderPixelStandard(float x, 
                                  float y, 
                                  const AmnCamera* camera);

// standard shading function
void RenderTileStandard(int taskIndex,
                        int threadIndex,
                        int* pixels,
                        const unsigned int width,
                        const unsigned int height,
                        const float time,
                        const AmnCamera* camera,
                        const int numTilesX,
                        const int numTilesY);

// task that renders a single pixel with ambient occlusion 
embree::Vec3fa RenderPixelAmbientOcclusion(float x, 
                                          float y, 
                                          const AmnCamera* camera);

// ambient occlusion shading function
void RenderTileAmbientOcclusion(int taskIndex,
                                int threadIndex,
                                int* pixels,
                                const unsigned int width,
                                const unsigned int height,
                                const float time,
                                const AmnCamera* camera,
                                const int numTilesX,
                                const int numTilesY);
                                
// task that renders a single pixel with geometry normal
embree::Vec3fa RenderPixelNormal(float x, 
                                float y, 
                                const AmnCamera* camera);

// geometry normal shading function
void RenderTileNormal(int taskIndex,
                      int threadIndex,
                      int* pixels,
                      const unsigned int width,
                      const unsigned int height,
                      const float time,
                      const AmnCamera* camera,
                      const int numTilesX,
                      const int numTilesY);
                      
// task that renders a single screen tile
void RenderTileTask (int taskIndex, int threadIndex, int* pixels,
                    const unsigned int width,
                    const unsigned int height,
                    const float time,
                    const AmnCamera* camera,
                    const int numTilesX,
                    const int numTilesY);

// called by the C++ code to render
void DeviceRender (int* pixels,
                  const unsigned int width,
                  const unsigned int height,
                  const float time,
                  const AmnCamera* camera);

// render to file
void RenderToFile(const embree::FileName& fileName, 
                  AmnCamera* viewCamera,
                  int width, 
                  int height);

// save to file
void SaveToFile(const embree::FileName& fileName);

// render to memory
void RenderToMemory(AmnCamera* viewCamera, bool interact=false);


AMN_NAMESPACE_CLOSE_SCOPE