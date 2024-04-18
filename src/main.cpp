
#include "common.h"
#include "utils/keys.h"
#include "utils/options.h"
#include "app/application.h"

JVR_NAMESPACE_USING_DIRECTIVE

int main(int argc, char *const *argv) {
  
  CommandLineOptions options(argc, argv);

  // Initialize glfw
  if (!glfwInit())
      return -1;

  BuildKeyMap();
  Application* app = Application::Get();
  app->Init(1024, 720);
  while (app->Update());
  glfwTerminate();
  delete app;
  return 1;
}