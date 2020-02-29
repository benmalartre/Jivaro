
#include "default.h"
#include "application.h"
#include "context.h"

namespace AMN {
  UsdEmbreeContext* EMBREE_CTXT = NULL;

  GLuint SCREENSPACEQUAD_VAO = 0;
  GLuint SCREENSPACEQUAD_VBO = 0;

  GLuint SCREENSPACEQUAD_VERTEX_SHADER = 0;
  GLuint SCREENSPACEQUAD_FRAGMENT_SHADER = 0;
  GLuint SCREENSPACEQUAD_PROGRAM_SHADER = 0;
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