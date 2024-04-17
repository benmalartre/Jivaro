#ifndef JVR_APPLICATION_APPLICATION_H
#define JVR_APPLICATION_APPLICATION_H

#include <pxr/usd/usd/stageCache.h>
#include "../common.h"
#include "../exec/execution.h"
#include "../app/time.h"
#include "../app/notice.h"
#include "../app/selection.h"
#include "../command/manager.h"


JVR_NAMESPACE_OPEN_SCOPE

class PopupUI;
class Engine;
class Window;
class Scene;

class Application : public pxr::TfWeakBase
{
public:
  static const char* name;
  // constructor
  Application();

  // destructor
  ~Application();

    // create a fullscreen window
  static Window* CreateFullScreenWindow(const std::string& name);

  // create a standard window of specified size
  static Window* CreateStandardWindow(const std::string& name, const pxr::GfVec4i& dimension);

  // create a child window
  static Window* CreateChildWindow(const std::string& name, const pxr::GfVec4i& dimension, Window* parent);

  // browse file
  std::string BrowseFile(int x, int y, const char* folder, const char* filters[], 
    const int numFilters, const char* name="Browse", bool readOrWrite=false);
  
  // init application
  void Init(unsigned width, unsigned height, bool fullscreen=false);
  void Term();

  // update application
  bool Update();

  void NewScene(const std::string& filename);
  void OpenScene(const std::string& filename);
  void SaveScene();
  void SaveSceneAs(const std::string& filename);

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
  void UndoStackNoticeCallback(const UndoStackNotice& n);

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
  Window* GetActiveWindow() { return _activeWindow ? _activeWindow : _mainWindow; };
  void SetActiveWindow(Window* window) { _activeWindow = window; };
  void SetFocusWindow(Window* window) { _focusWindow = window; };
  void AddWindow(Window* window);
  void RemoveWindow(Window* window);

  // popup
  PopupUI* GetPopup() { return _popup; };
  void SetPopup(PopupUI* popup);
  void UpdatePopup();

  // tools
  void SetActiveTool(size_t tool);
  void AddDeferredCommand(CALLBACK_FN fn);
  void ExecuteDeferredCommands();

  // engines
  void AddEngine(Engine* engine);
  void RemoveEngine(Engine* engine);
  void SetActiveEngine(Engine* engine);
  Engine* GetActiveEngine();
  std::vector<Engine*> GetEngines() { return _engines; };
  void DirtyAllEngines();

  // stage cache
  pxr::UsdStageRefPtr& GetStage(){return _stage;};
  void SetStage(pxr::UsdStageRefPtr& stage);
  pxr::UsdStageCache& GetStageCache() { return _stageCache; }

  // execution
  void ToggleExec();
  void SetExec(bool state);
  bool GetExec();

  virtual void InitExec(pxr::UsdStageRefPtr& stage);
  virtual void UpdateExec(pxr::UsdStageRefPtr& stage, float time);
  virtual void TerminateExec(pxr::UsdStageRefPtr& stage);

  // usd stages
  //std::vector<pxr::UsdStageRefPtr>& GetStages(){return _stages;};
  pxr::UsdStageRefPtr GetDisplayStage();
  pxr::UsdStageRefPtr GetWorkStage();

  pxr::SdfLayerRefPtr GetCurrentLayer();

protected:
  Execution*                        _exec;

private:
  bool                              _IsAnyEngineDirty();
  std::string                       _fileName;
  Window*                           _mainWindow;
  std::vector<Window*>              _childWindows;
  Window*                           _activeWindow;
  Window*                           _focusWindow;
  Selection                         _selection;

  // uis
  PopupUI*                          _popup;

  // time
  Time                              _time;

  // command
  CommandManager                    _manager;
  std::vector<CALLBACK_FN>          _deferred;

  // engines
  pxr::UsdStageCache                _stageCache;
  pxr::UsdStageRefPtr               _stage;
  pxr::SdfLayerRefPtr               _layer;
  std::vector<Engine*>              _engines;
  Engine*                           _activeEngine;
  bool                              _execute;

};


static Application* GetApplication() { 
  static Application __APP;
  return &__APP; 
};

#define ADD_COMMAND(CMD, ...) \
GetApplication()->AddCommand(std::shared_ptr<CMD>( new CMD(__VA_ARGS__)));

JVR_NAMESPACE_CLOSE_SCOPE // namespace JVR

#endif // JVR_APPLICATION_APPLICATION_H

