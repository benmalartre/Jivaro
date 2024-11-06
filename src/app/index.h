#ifndef JVR_APPLICATION_INDEX_H
#define JVR_APPLICATION_INDEX_H

#include <pxr/base/gf/vec3d.h>
#include <pxr/imaging/hd/mergingSceneIndex.h>
#include <pxr/imaging/hd/sceneIndex.h>
#include <pxr/usdImaging/usdImaging/sceneIndices.h>
#include <pxr/usdImaging/usdImaging/stageSceneIndex.h>

#include "../common.h"
#include "../app/time.h"
#include "../app/notice.h"
#include "../app/selection.h"
#include "../exec/execution.h"
#include "../exec/sceneIndex.h"
#include "../command/manager.h"


JVR_NAMESPACE_OPEN_SCOPE

class Engine;
class Scene;
class Execution;

class Index
{
public:
  // constructor
  Index();

  // destructor
  ~Index();

  // update
  void Update(const float time);

  void AddSceneIndexBase(HdSceneIndexBaseRefPtr sceneIndex);
  HdSceneIndexBaseRefPtr GetEditableSceneIndex();

  void SetCurrentSceneIndex(HdSceneIndexBaseRefPtr sceneIndex);
  HdSceneIndexBaseRefPtr GetFinalSceneIndex();
  HdSceneIndexPrim GetPrim(SdfPath primPath);

  void SetStage(UsdStageRefPtr& stage);

  // execution
  void ToggleExec();
  void SetExec(bool state);
  bool GetExec();

  virtual void InitExec(Execution* exec);
  virtual void UpdateExec(float time);
  virtual void TerminateExec();
  virtual void SendExecViewEvent(const ViewEventData &data);

private:

  UsdStageRefPtr                    _stage;
  UsdImagingStageSceneIndexRefPtr   _stageSceneIndex;
  HdSceneIndexBaseRefPtr            _editableSceneIndex;
  HdMergingSceneIndexRefPtr         _sceneIndexBases;
  HdMergingSceneIndexRefPtr         _finalSceneIndex;

  // execution
  bool                              _execute;
  Execution*                        _exec;
  ExecSceneIndexRefPtr              _execSceneIndex;
  float                             _lastTime;


};

JVR_NAMESPACE_CLOSE_SCOPE // namespace JVR

#endif // JVR_APPLICATION_INDEX_H

