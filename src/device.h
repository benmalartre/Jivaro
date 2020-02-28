
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

#include "default.h"
#include "camera.h"
#include "ray.h"

namespace AMN {

#define TILE_SIZE_X 8
#define TILE_SIZE_Y 8
#define WIDTH 1024
#define HEIGHT 720

extern RTCScene g_scene;
extern embree::Vec3fa* face_colors; 
extern embree::Vec3fa* vertex_colors;
extern RTCDevice g_device;
extern bool g_changed;
extern float g_debug;

static embree::Camera camera;
static std::string rtcore("start_threads=1,set_affinity=1");

// face forward for shading normals
inline embree::Vec3fa FaceForward(const embree::Vec3fa& N, 
                                  const embree::Vec3fa& I, 
                                  const embree::Vec3fa& Ng ) 
{
  embree::Vec3fa NN = N; return embree::dot(I, Ng) < 0 ? NN : embree::neg(NN);
}

// render statistics
struct RayStats
{
  int numRays;
  int pad[32-1];
};

#if defined(RAY_STATS)
#if defined(ISPC)
inline void RayStats_addRay(RayStats& stats)       { stats.numRays += popcnt(1); }
inline void RayStats_addShadowRay(RayStats& stats) { stats.numRays += popcnt(1); }
#else // C++
__forceinline void RayStats_addRay(RayStats& stats)        { stats.numRays++; }
__forceinline void RayStats_addShadowRay(RayStats& stats)  { stats.numRays++; }
#endif
#else // disabled
inline void RayStats_addRay(RayStats& stats)       {}
inline void RayStats_addShadowRay(RayStats& stats) {}
#endif

static RayStats* g_stats;

inline bool NativePacketSupported(RTCDevice device)
{
  if (sizeof(float) == 1*4) return true;
  else if (sizeof(float) == 4*4) return rtcGetDeviceProperty(device,RTC_DEVICE_PROPERTY_NATIVE_RAY4_SUPPORTED);
  else if (sizeof(float) == 8*4) return rtcGetDeviceProperty(device,RTC_DEVICE_PROPERTY_NATIVE_RAY8_SUPPORTED);
  else if (sizeof(float) == 16*4) return rtcGetDeviceProperty(device,RTC_DEVICE_PROPERTY_NATIVE_RAY16_SUPPORTED);
  else return false;
}

// scene management
unsigned int AddCube (RTCScene scene_i);
unsigned int AddGroundPlane (RTCScene scene_i);

// device management
RTCScene DeviceInit (char* cfg);
void CommitScene ();
void DeviceCleanup ();

// render tile function prototype
typedef void (* RenderTileFunc)(int taskIndex,
                                int threadIndex,
                                int* pixels,
                                const unsigned int width,
                                const unsigned int height,
                                const float time,
                                const embree::ISPCCamera& camera,
                                const int numTilesX,
                                const int numTilesY);
static RenderTileFunc RenderTile;

// task that renders a single screen tile
embree::Vec3fa RenderPixelStandard(float x, float y, const embree::ISPCCamera& camera, RayStats& stats);

// standard shading function
void RenderTileStandard(int taskIndex,
                        int threadIndex,
                        int* pixels,
                        const unsigned int width,
                        const unsigned int height,
                        const float time,
                        const embree::ISPCCamera& camera,
                        const int numTilesX,
                        const int numTilesY);

// task that renders a single pixel with ambient occlusion 
embree::Vec3fa RenderPixelAmbientOcclusion(float x, float y, const embree::ISPCCamera& camera, RayStats& stats);

// ambient occlusion shading function
void RenderTileAmbientOcclusion(int taskIndex,
                                int threadIndex,
                                int* pixels,
                                const unsigned int width,
                                const unsigned int height,
                                const float time,
                                const embree::ISPCCamera& camera,
                                const int numTilesX,
                                const int numTilesY);
                                
// task that renders a single pixel with geometry normal
embree::Vec3fa RenderPixelNormal(float x, float y, const embree::ISPCCamera& camera, RayStats& stats);

// geometry normal shading function
void RenderTileNormal(int taskIndex,
                      int threadIndex,
                      int* pixels,
                      const unsigned int width,
                      const unsigned int height,
                      const float time,
                      const embree::ISPCCamera& camera,
                      const int numTilesX,
                      const int numTilesY);
                      
// task that renders a single screen tile
void RenderTileTask (int taskIndex, int threadIndex, int* pixels,
                         const unsigned int width,
                         const unsigned int height,
                         const float time,
                         const embree::ISPCCamera& camera,
                         const int numTilesX,
                         const int numTilesY);

// called by the C++ code to render
void DeviceRender (int* pixels,
                           const unsigned int width,
                           const unsigned int height,
                           const float time,
                           const embree::ISPCCamera& camera);

// render to file
void RenderToFile(const embree::FileName& fileName);



} // namespace AMN