
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

namespace embree {

#define TILE_SIZE_X 8
#define TILE_SIZE_Y 8
#define WIDTH 1280
#define HEIGHT 640

extern RTCScene g_scene;
extern Vec3fa* face_colors; 
extern Vec3fa* vertex_colors;
extern RTCDevice g_device;
extern bool g_changed;
extern float g_debug;

static Camera camera;
static std::string rtcore("start_threads=1,set_affinity=1");

// face forward for shading normals
inline Vec3fa faceforward( const Vec3fa& N, const Vec3fa& I, const Vec3fa& Ng ) {
  Vec3fa NN = N; return dot(I, Ng) < 0 ? NN : neg(NN);
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

inline bool nativePacketSupported(RTCDevice device)
{
  if (sizeof(float) == 1*4) return true;
  else if (sizeof(float) == 4*4) return rtcGetDeviceProperty(device,RTC_DEVICE_PROPERTY_NATIVE_RAY4_SUPPORTED);
  else if (sizeof(float) == 8*4) return rtcGetDeviceProperty(device,RTC_DEVICE_PROPERTY_NATIVE_RAY8_SUPPORTED);
  else if (sizeof(float) == 16*4) return rtcGetDeviceProperty(device,RTC_DEVICE_PROPERTY_NATIVE_RAY16_SUPPORTED);
  else return false;
}

// scene management
unsigned int addCube (RTCScene scene_i);
unsigned int addGroundPlane (RTCScene scene_i);

// device management
RTCScene device_init (char* cfg);
void commit_scene ();
void device_cleanup ();

// render tile function prototype
typedef void (* renderTileFunc)(int taskIndex,
                                int threadIndex,
                                int* pixels,
                                const unsigned int width,
                                const unsigned int height,
                                const float time,
                                const ISPCCamera& camera,
                                const int numTilesX,
                                const int numTilesY);
static renderTileFunc renderTile;

// task that renders a single screen tile
Vec3fa renderPixelStandard(float x, float y, const ISPCCamera& camera, RayStats& stats);

// standard shading function
void renderTileStandard(int taskIndex,
                        int threadIndex,
                        int* pixels,
                        const unsigned int width,
                        const unsigned int height,
                        const float time,
                        const ISPCCamera& camera,
                        const int numTilesX,
                        const int numTilesY);

// task that renders a single pixel with ambient occlusion 
Vec3fa renderPixelAmbientOcclusion(float x, float y, const ISPCCamera& camera, RayStats& stats);

// ambient occlusion shading function
void renderTileAmbientOcclusion(int taskIndex,
                                int threadIndex,
                                int* pixels,
                                const unsigned int width,
                                const unsigned int height,
                                const float time,
                                const ISPCCamera& camera,
                                const int numTilesX,
                                const int numTilesY);
                                

// task that renders a single screen tile
void renderTileTask (int taskIndex, int threadIndex, int* pixels,
                         const unsigned int width,
                         const unsigned int height,
                         const float time,
                         const ISPCCamera& camera,
                         const int numTilesX,
                         const int numTilesY);

// called by the C++ code to render
void device_render (int* pixels,
                           const unsigned int width,
                           const unsigned int height,
                           const float time,
                           const ISPCCamera& camera);

// render to file
void renderToFile(const FileName& fileName);



} // namespace embree