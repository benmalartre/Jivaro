
#include "default.h"
#include "app/application.h"
#include "embree/context.h"

PXR_NAMESPACE_OPEN_SCOPE

extern AmnUsdEmbreeContext* EMBREE_CTXT = 0;

PXR_NAMESPACE_CLOSE_SCOPE



// main application entry point
//------------------------------------------------------------------------------
PXR_NAMESPACE_USING_DIRECTIVE

int main(void)
{
  FilesInDirectory();

  glfwInit();
  AmnApplication app(800,600);
  app.Init();
  app.MainLoop();
  
  glfwTerminate();
  return 1;
}