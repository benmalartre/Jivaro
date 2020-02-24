#pragma once

#include "default.h"
#include "window.h"
//#include "application.h"
#include "camera.h"
//#include "scene.h"
//#include "scene_device.h"

/* include GLFW for window management */
#include <GLFW/glfw3.h>

/* include ImGUI */
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"

namespace AMN
{

  class Application
  {
  public:
    static const char* APPLICATION_NAME;
    // constructor
    Application(unsigned width, unsigned height);
    Application(bool fullscreen=true);

    // destructor
    ~Application(){};

     // create a fullscreen window
    static Window* CreateFullScreenWindow();

    // create a standard window of specified size
    static Window* CreateStandardWindow(int width, int height);
    
    // the main loop
    void MainLoop();

    // cleanup
    void CleanUp();

  private:
    
    std::string _fileName;
    Window*     _mainWindow;

    int         _width;
    int         _height;

  };
} // namespace AMN

