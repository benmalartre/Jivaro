
#include "default.h"
#include "app/application.h"
#include "embree/context.h"

AMN_NAMESPACE_OPEN_SCOPE

extern UsdEmbreeContext* EMBREE_CTXT = 0;

AMN_NAMESPACE_CLOSE_SCOPE



// main application entry point
//------------------------------------------------------------------------------
AMN_NAMESPACE_USING_DIRECTIVE

int main(void)
{
  glfwInit();
  Application app(800,600);
  app.Init();
  app.MainLoop();
  glfwTerminate();
  return 1;
}