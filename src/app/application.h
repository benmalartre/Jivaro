#ifndef JVR_APPLICATION_APPLICATION_H
#define JVR_APPLICATION_APPLICATION_H
#include <map>

#include "../common.h"
#include "../exec/execution.h"
#include "../app/model.h"
#include "../command/manager.h"


JVR_NAMESPACE_OPEN_SCOPE

class PopupUI;
class Engine;
class Window;
class Scene;

class Application : public TfWeakBase
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
  static Window* CreateStandardWindow(const std::string& name, const GfVec4i& dimension);

  // create a child window
  static Window* CreateChildWindow(const std::string& name, const GfVec4i& dimension, Window* parent);

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
  bool IsToolInteracting();

  void AddDeferredCommand(CALLBACK_FN fn);
  void ExecuteDeferredCommands();

  // model
  Model* GetModel() {return _model;};

  // engines
  /*
  void AddEngine(Engine* engine);
  void RemoveEngine(Engine* engine);
  void SetActiveEngine(Engine* engine);
  Engine* GetActiveEngine();
  std::vector<Engine*> GetEngines() { return _engines; };
  */
  void DirtyAllEngines();

  // notices callback
  void SelectionChangedCallback(const SelectionChangedNotice& n);
  void NewSceneCallback(const NewSceneNotice& n); 
  void SceneChangedCallback(const SceneChangedNotice& n);
  void AttributeChangedCallback(const AttributeChangedNotice& n);
  void TimeChangedCallback(const TimeChangedNotice& n);
  void UndoStackNoticeCallback(const UndoStackNotice& n);

  // singleton 
  static Application *Get();

protected:
  static Application*               _singleton;

private:
  // windows
  Window*                           _mainWindow;
  std::vector<Window*>              _childWindows;
  Window*                           _activeWindow;
  Window*                           _focusWindow;
  Model*                            _model;
  float                             _lastTime;

  // uis
  PopupUI*                          _popup;

   // command
  std::vector<CALLBACK_FN>          _deferred;

};

JVR_NAMESPACE_CLOSE_SCOPE // namespace JVR

#endif // JVR_APPLICATION_APPLICATION_H

