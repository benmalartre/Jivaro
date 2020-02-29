#pragma once

#include "default.h"
#include "window.h"
#include "view.h"
#include "camera.h"
#include "device.h"
#include "context.h"
#include "prim.h"
#include "mesh.h"

namespace AMN
{
  extern UsdEmbreeContext* EMBREE_CTXT;
  
  class Application
  {
  public:
    static const char* APPLICATION_NAME;
    // constructor
    Application(unsigned width, unsigned height);
    Application(bool fullscreen=true);

    // destructor
    ~Application(){if(_context)delete _context;};

     // create a fullscreen window
    static Window* CreateFullScreenWindow();

    // create a standard window of specified size
    static Window* CreateStandardWindow(int width, int height);
    
    // init aplication
    void Init();

    // the main loop
    void MainLoop();

    // cleanup
    void CleanUp();

  private:
    std::string         _fileName;
    Window*             _mainWindow;
    UsdEmbreeContext*   _context;
    int                 _width;
    int                 _height;
  };
} // namespace AMN

