#ifndef JVR_APPLICATION_APPLICATION_H
#define JVR_APPLICATION_APPLICATION_H
#include <map>

#include <pxr/usd/usd/stageCache.h>

#include "../common.h"
#include "../exec/execution.h"
#include "../app/model.h"
#include "../app/registry.h"
#include "../command/manager.h"


JVR_NAMESPACE_OPEN_SCOPE

class PopupUI;
class ViewportUI;
class Engine;
class Window;
class Scene;
class View;

class Application : public TfWeakBase
{
public:
  static const char* name;
  // constructor
  Application();

  // destructor
  ~Application();


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

  // commands
  void Undo();
  void Redo();
  void Duplicate();
  void Delete();

  // window
  Window* GetMainWindow() {return _windows->GetMainWindow();};
  Window* GetChildWindow(size_t index) {return _windows->GetChildWindow(index);};
  Window* GetActiveWindow() { return _windows->GetActiveWindow(); };
  void SetActiveWindow(Window* window) { _windows->SetActiveWindow(window); };
  /*
  void AddWindow(Window* window);
  void RemoveWindow(Window* window);
  void SetWindowDirty(Window* window);
  void SetAllWindowsDirty();
  */

  // playback viewport
  bool IsPlaybackView(View* view);
  void SetPlaybackView(View* view);

  // popup
  PopupUI* GetPopup() { return _popup; };
  void SetPopup(PopupUI* popup);
  void UpdatePopup();

  void AddDeferredCommand(CALLBACK_FN fn);
  void ExecuteDeferredCommands();

  // model
  Model* GetModel() {return _model;};

  // stages
  UsdStageCache& GetStageCache() { return _stageCache; }

  // notices callback
  void SelectionChangedCallback(const SelectionChangedNotice& n);
  void NewSceneCallback(const NewSceneNotice& n); 
  void SceneChangedCallback(const SceneChangedNotice& n);
  void AttributeChangedCallback(const AttributeChangedNotice& n);
  void TimeChangedCallback(const TimeChangedNotice& n);
  void UndoStackNoticeCallback(const UndoStackNotice& n);

  // singleton 
  static Application *Get();

  // preferences
  static bool         PlaybackAllViews;

protected:
  static Application*               _singleton;


private:
  // windows
  RegistryWindow*                   _windows;
  Model*                            _model;
  float                             _lastTime;

  // uis
  PopupUI*                          _popup;
  View*                             _playbackView;

   // command
  std::vector<CALLBACK_FN>          _deferred;

  // stages
  UsdStageCache                     _stageCache;

};

JVR_NAMESPACE_CLOSE_SCOPE // namespace JVR

#endif // JVR_APPLICATION_APPLICATION_H

