
#include "common.h"
#include "app/application.h"

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