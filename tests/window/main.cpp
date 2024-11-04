//#include <pxr/imaging/garch/glDebugWindow.h>


//int main()
//{
//   pxr::GarchGLDebugWindow window("OpenGL Pixar Window", 800, 600);
//   window.Init();
//   window.Run();
//   window.ExitApp();
//   return 0;
//}

#include "../../src/common.h"
#include "../../src/app/window.h"
#include "../../src/app/registry.h"

JVR_NAMESPACE_USING_DIRECTIVE

int main(){
  
  glfwInit();
  Window* window = WindowRegistry::CreateStandardWindow("zob", GfVec4i(0,0, 800,600));
  while (!glfwWindowShouldClose(window->GetGlfwWindow()))
    window->Update();

  delete window;
  glfwTerminate();
  
  
}
