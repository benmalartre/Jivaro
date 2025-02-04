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
class Index;
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


  // model
  Model* GetModel() {return _model;};
  Index* GetIndex(){return _index;};

  // stages
  UsdStageCache& GetStageCache() { return _stageCache; }

  // notices callback
  void SelectionChangedCallback(const SelectionChangedNotice& n);
  void NewSceneCallback(const NewSceneNotice& n); 
  void SceneChangedCallback(const SceneChangedNotice& n);
  void AttributeChangedCallback(const AttributeChangedNotice& n);
  void TimeChangedCallback(const TimeChangedNotice& n);
  void ToolChangedCallback(const ToolChangedNotice& n);
  void UndoStackNoticeCallback(const UndoStackNotice& n);

  // singleton 
  static Application *Get();

protected:
  static Application*               _singleton;

private:
  WindowRegistry*                   _windows;
  UIRegistry*                       _uis;
  Index*                            _index;
  Model*                            _model;
  float                             _lastTime;

  // stages
  UsdStageCache                     _stageCache;

};

JVR_NAMESPACE_CLOSE_SCOPE // namespace JVR

#endif // JVR_APPLICATION_APPLICATION_H

