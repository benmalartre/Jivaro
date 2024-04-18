
#include "common.h"
#include "utils/keys.h"
#include "app/application.h"

JVR_NAMESPACE_USING_DIRECTIVE


int main(void)
{
  glfwInit();
  BuildKeyMap();
  Application* app = Application::Get();
  app->Init(1024, 720);
  while (app->Update());
  glfwTerminate();
  delete app;
  return 1;
}