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

  //void AddDeferredCommand(CALLBACK_FN fn);
  //void ExecuteDeferredCommands();

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

protected:
  static Application*               _singleton;

private:
  WindowRegistry*                   _windows;
  Model*                            _model;
  float                             _lastTime;

   // command
  std::vector<CALLBACK_FN>          _deferred;

  // stages
  UsdStageCache                     _stageCache;

};

JVR_NAMESPACE_CLOSE_SCOPE // namespace JVR

#endif // JVR_APPLICATION_APPLICATION_H

