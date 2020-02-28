#pragma once

#include "default.h"
#include "window.h"
#include "view.h"
#include "camera.h"
#include "device.h"
#include "mesh.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/mesh.h>


namespace AMN
{
  extern RTCScene g_scene;
  extern embree::Vec3fa* face_colors; 
  extern embree::Vec3fa* vertex_colors;
  extern RTCDevice g_device;
  extern bool g_changed;
  extern float g_debug;
  
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
    
    // init aplication
    void Init();

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

