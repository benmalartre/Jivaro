#ifndef JVR_APPLICATION_APPLICATION_H
#define JVR_APPLICATION_APPLICATION_H

#pragma once

#include "../common.h"
#include "../app/scene.h"
#include "../app/selection.h"
#include "../app/time.h"
#include "../app/window.h"
#include "../app/view.h"
#include "../app/camera.h"
#include "../app/tools.h"
#include "../command/manager.h"
#include "../geometry/mesh.h"
//#include <openvdb/openvdb.h>

PXR_NAMESPACE_OPEN_SCOPE

class TimelineUI;
class PropertyUI;
class ViewportUI;
class GraphEditorUI;
class ExplorerUI;
class LayersUI;
class CurveEditorUI;
class Command;
class CommandManager;
class Engine;

class Application : public pxr::TfWeakBase
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
  static Window* CreateChildWindow(int x, int y, int width, int height, Window* parent, 
    const std::string& name="Child", bool decorated=true);


  // browse file
  std::string BrowseFile(int x, int y, const char* folder, const char* filters[], 
    const int numFilters, const char* name="Browse");
  
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
  void SaveScene(const std::string& filename);

  // selection
  Selection* GetSelection(){return &_selection;};
  void SetSelection(const pxr::SdfPathVector& selection);
  void ToggleSelection(const pxr::SdfPathVector& selection);
  void AddToSelection(const pxr::SdfPathVector& path);
  void RemoveFromSelection(const pxr::SdfPathVector& path);
  void ClearSelection();
  pxr::GfBBox3d GetSelectionBoundingBox();
  pxr::GfBBox3d GetStageBoundingBox();

  // notices callback
  void SelectionChangedCallback(const SelectionChangedNotice& n);
  void NewSceneCallback(const NewSceneNotice& n); 
  void SceneChangedCallback(const SceneChangedNotice& n);
  void AttributeChangedCallback(const AttributeChangedNotice& n);

  // commands
  void AddCommand(std::shared_ptr<Command> command);
  void Undo();
  void Redo();
  void Duplicate();
  void Delete();

  // time
  Time& GetTime() { return _time; };

  // window
  Window* GetMainWindow() {return _mainWindow;};
  Window* GetChildWindow(size_t index) {return _childWindows[index];};

  // tools
  Tool* GetTools(){return &_tools;};
  void SetActiveTool(short tool) {_tools.SetActiveTool(tool);};

  // engines
  void AddEngine(Engine* engine);
  void RemoveEngine(Engine* engine);
  std::vector<Engine*> GetEngines() { return _engines; };

  // usd stages
  //std::vector<pxr::UsdStageRefPtr>& GetStages(){return _stages;};
  pxr::UsdStageRefPtr& GetStage() { return _scene->GetCurrentStage(); };

private:
  std::string                       _fileName;
  Window*                           _mainWindow;
  std::vector<Window*>              _childWindows;
  Scene*                            _scene;
  Selection                         _selection;
  Tool                              _tools;

  // uis
  ViewportUI*                       _viewport;
  GraphEditorUI*                    _graph;
  LayersUI*                         _layers;
  ExplorerUI*                       _explorer;
  TimelineUI*                       _timeline;
  PropertyUI*                       _property;
  CurveEditorUI*                    _animationEditor;

  // time
  Time                              _time;

  // mesh
  Mesh*                             _mesh;

  // command
  CommandManager                    _manager;

  // engines
  std::vector<Engine*>              _engines;
};

extern Application* APPLICATION;

static Application* GetApplication() { return APPLICATION; };

PXR_NAMESPACE_CLOSE_SCOPE // namespace JVR

#endif // JVR_APPLICATION_APPLICATION_H

