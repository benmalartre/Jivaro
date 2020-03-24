#pragma once

#include "default.h"
#include "window.h"
#include "view.h"
#include "camera.h"
#include "../graph/node.h"
#include "../graph/graph.h"
#include "../widgets/graph.h"
#include "../embree/device.h"
#include "../embree/context.h"
#include "../embree/prim.h"
#include "../embree/mesh.h"

AMN_NAMESPACE_OPEN_SCOPE

class Application
{
public:
  static const char* APPLICATION_NAME;
  // constructor
  Application(unsigned width, unsigned height);
  Application(bool fullscreen=true);

  // destructor
  ~Application();

    // create a fullscreen window
  static Window* CreateFullScreenWindow();

  // create a standard window of specified size
  static Window* CreateStandardWindow(int width, int height);
  
  // init aplication
  void Init();
  void Term();

  // the main loop
  void MainLoop();

  // cleanup
  void CleanUp();

private:
  std::string                       _fileName;
  Window*                           _mainWindow;
  UsdEmbreeContext*                 _context;
  int                               _width;
  int                               _height;
  std::vector<pxr::UsdStageRefPtr>  _stages;
  GraphUI*                          _test;
};

AMN_NAMESPACE_CLOSE_SCOPE // namespace pxr

