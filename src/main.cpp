
#include "common.h"
#include "app/application.h"

JVR_NAMESPACE_USING_DIRECTIVE

int main(void)
{
  glfwInit();
  
  Application* app = Application::Create(1024, 720);
  app->Init();
  app->MainLoop();
  glfwTerminate();
  return 1;
}