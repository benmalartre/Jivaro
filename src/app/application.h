#ifndef AMN_APPLICATION_APPLICATION_H
#define AMN_APPLICATION_APPLICATION_H

#pragma once

#include "../common.h"
#include "selection.h"
#include "time.h"
#include "window.h"
#include "view.h"
#include "camera.h"
#include "../graph/node.h"
#include "../graph/graph.h"

AMN_NAMESPACE_OPEN_SCOPE

class TimelineUI;
class PropertyUI;
class ViewportUI;
class GraphUI;
class ExplorerUI;

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

  // create a child window
  static Window* CreateChildWindow(int width, int height, Window* parent, 
    const std::string& name="Child");

  // browse file
  std::string BrowseFile(const char* folder, const char* filters[], 
    const int numFilters);
  
  // init application
  void Init();
  void Term();

  // update application
  void Update();

  // the main loop
  void MainLoop();

  // cleanup
  void CleanUp();

  void OpenScene(const std::string& filename);

  // selection
  void SetSelection(const pxr::SdfPathVector& selection);
  void AddToSelection(const pxr::SdfPath& path);
  void RemoveFromSelection(const pxr::SdfPath& path);
  void ClearSelection();
  pxr::GfBBox3d GetSelectionBoundingBox();
  pxr::GfBBox3d GetStageBoundingBox();

  // time
  Time& GetTime() { return _time; };

  // window
  Window* GetMainWindow() {return _mainWindow;};
  Window* GetChildWindow(size_t index) {return _childWindows[index];};

  // usd stages
  //std::vector<pxr::UsdStageRefPtr>& GetStages(){return _stages;};
  pxr::UsdStageRefPtr& GetStage() { return _stage; };

private:
  std::string                       _fileName;
  Window*                           _mainWindow;
  std::vector<Window*>              _childWindows;
  //std::vector<pxr::UsdStageRefPtr>  _stages;
  pxr::UsdStageRefPtr               _stage;
  Selection                         _selection;
  //pxr::UsdGeomBBoxCache*            _bboxCache;
  //pxr::UsdGeomXformCache*           _xformCache;

  // uis
  ViewportUI*                       _viewport;
  GraphUI*                          _graph;
  ExplorerUI*                       _explorer;
  TimelineUI*                       _timeline;
  PropertyUI*                       _property;

  // time
  Time                              _time;
};

extern Application* AMN_APPLICATION;

AMN_NAMESPACE_CLOSE_SCOPE // namespace amn

#endif // AMN_APPLICATION_APPLICATION_H

