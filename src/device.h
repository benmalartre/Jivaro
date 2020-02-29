
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
#include "context.h"

namespace AMN {

class ViewportUI;
// screen-space-quad 
extern GLuint SCREENSPACEQUAD_VAO;
extern GLuint SCREENSPACEQUAD_VBO;

extern GLuint SCREENSPACEQUAD_VERTEX_SHADER;
extern GLuint SCREENSPACEQUAD_FRAGMENT_SHADER;
extern GLuint SCREENSPACEQUAD_PROGRAM_SHADER;

extern UsdEmbreeContext* EMBREE_CTXT;

#define TILE_SIZE_X 8
#define TILE_SIZE_Y 8
#define ENABLE_SMOOTH_NORMALS 1

static embree::Camera camera;
static std::string rtcore("start_threads=1,set_affinity=1");

// face forward for shading normals
inline embree::Vec3fa FaceForward(const embree::Vec3fa& N, 
                                  const embree::Vec3fa& I, 
                                  const embree::Vec3fa& Ng ) 
{
  embree::Vec3fa NN = N; return embree::dot(I, Ng) < 0 ? NN : embree::neg(NN);
}

// device management
RTCScene DeviceInit ();
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
embree::Vec3fa RenderPixelStandard(float x, 
                                  float y, 
                                  const embree::ISPCCamera& camera, 
                                  RayStats& stats);

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
embree::Vec3fa RenderPixelAmbientOcclusion(float x, 
                                          float y, 
                                          const embree::ISPCCamera& camera, 
                                          RayStats& stats);

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
embree::Vec3fa RenderPixelNormal(float x, 
                                float y, 
                                const embree::ISPCCamera& camera, 
                                RayStats& stats);

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

// render to memory
void RenderToMemory();

// render to viewport
void RenderToViewport(ViewportUI* viewport);

} // namespace AMN