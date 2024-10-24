#ifndef JVR_EXEC_EXECUTION_H
#define JVR_EXEC_EXECUTION_H


#include <pxr/base/tf/hashMap.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/base/work/loops.h>
#include <pxr/imaging/hd/changeTracker.h>
#include "../geometry/scene.h"

JVR_NAMESPACE_OPEN_SCOPE

class Scene;
struct ViewEventData;

class Execution {
public:
  virtual ~Execution(){};
  typedef std::pair<pxr::SdfPath, pxr::HdDirtyBits>           _Source;
  typedef pxr::VtArray<_Source>                               _Sources;
  
  friend class Scene;

  virtual void InitExec(pxr::UsdStageRefPtr& stage) = 0;
  virtual void UpdateExec(pxr::UsdStageRefPtr& stage, float time) = 0;
  virtual void TerminateExec(pxr::UsdStageRefPtr& stage) = 0;

  virtual void ViewEvent(const ViewEventData *data);

public:
  Scene* GetScene(){return &_scene;};
  const Scene* GetScene() const {return &_scene;};

protected:
  virtual void _MouseButtonEvent(const ViewEventData *data);
  virtual void _MouseMoveEvent(const ViewEventData *data);
  virtual void _KeyboardInputEvent(const ViewEventData *data);

  void _GetRootPrim(pxr::UsdStageRefPtr& stage);

  Scene         _scene;
  pxr::SdfPath  _rootId;
  pxr::UsdPrim  _rootPrim;
  
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif //JVR_EXEC_EXECUTION_H