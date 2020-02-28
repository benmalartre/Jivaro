
#include "default.h"
#include "application.h"

namespace AMN {
  RTCScene g_scene = NULL;
  embree::Vec3fa* face_colors = NULL; 
  embree::Vec3fa* vertex_colors = NULL;
  RTCDevice g_device = NULL;
  bool g_changed = 0;
  float g_debug = 0.f;
} 

// main application entry point
//------------------------------------------------------------------------------
int main(void)
{
  using namespace AMN;
  FilesInDirectory();

  glfwInit();
  Application app(800,600);
  app.Init();
  app.MainLoop();

  
  glfwTerminate();
  return 1;
}