#include <pxr/imaging/garch/glDebugWindow.h>


int main()
{
   pxr::GarchGLDebugWindow window("OpenGL Pixar Window", 800, 600);
   window.Init();
   window.Run();
   window.ExitApp();
   return 0;
}

