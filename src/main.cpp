
#include "default.h"
#include "application.h"
#include "context.h"

namespace AMN {
  UsdEmbreeContext* EMBREE_CTXT = NULL;
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