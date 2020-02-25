#pragma once

#include "default.h"
#include "window.h"
#include "camera.h"

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

