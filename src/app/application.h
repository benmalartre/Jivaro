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
extern AmnUsdEmbreeContext* EMBREE_CTXT;

class AmnApplication
{
public:
  static const char* APPLICATION_NAME;
  // constructor
  AmnApplication(unsigned width, unsigned height);
  AmnApplication(bool fullscreen=true);

  // destructor
  ~AmnApplication();

    // create a fullscreen window
  static AmnWindow* CreateFullScreenWindow();

  // create a standard window of specified size
  static AmnWindow* CreateStandardWindow(int width, int height);
  
  // init aplication
  void Init();
  void Term();

  // the main loop
  void MainLoop();

  // cleanup
  void CleanUp();

private:
  std::string                       _fileName;
  AmnWindow*                        _mainWindow;
  AmnUsdEmbreeContext*              _context;
  int                               _width;
  int                               _height;
  std::vector<pxr::UsdStageRefPtr>  _stages;

  AmnGraphUI*                       _test;
};

AMN_NAMESPACE_CLOSE_SCOPE // namespace pxr

