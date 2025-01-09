#include <pxr/base/arch/env.h>

#include "common.h"
#include "utils/keys.h"
#include "app/application.h"

JVR_NAMESPACE_USING_DIRECTIVE

int main(int argc, char *const *argv) {
  
  // Initialize glfw
  if (!glfwInit())
      return -1;

  BuildKeyMap();
  Application* app = Application::Get();
  app->Init(1024, 720);

  // Main loop
  while (app->Update());
  delete app;
  glfwTerminate();
  return 1;
}