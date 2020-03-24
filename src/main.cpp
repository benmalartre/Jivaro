
#include "default.h"
#include "app/application.h"
#include "embree/context.h"

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