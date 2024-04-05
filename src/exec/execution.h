#ifndef JVR_EXEC_EXECUTION_H
#define JVR_EXEC_EXECUTION_H

#include <pxr/usd/usd/stage.h>
#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Scene;
class Execution {
  friend class Scene;
  Execution(Scene* scene) : _scene(scene) {};
  virtual void InitExec(pxr::UsdStageRefPtr& stage) = 0;
  virtual void UpdateExec(pxr::UsdStageRefPtr& stage, double time) = 0;
  virtual void TerminateExec(pxr::UsdStageRefPtr& stage) = 0;

protected:
  Scene* _scene;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif //JVR_EXEC_EXECUTION_H