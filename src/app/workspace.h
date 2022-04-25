#ifndef JVR_APPLICATION_WORKSPACE_H
#define JVR_APPLICATION_WORKSPACE_H

#include "../common.h"
#include "../app/scene.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/base/tf/hashmap.h>


PXR_NAMESPACE_OPEN_SCOPE

typedef pxr::TfHashMap< pxr::SdfPath, pxr::UsdStageRefPtr, pxr::SdfPath::Hash > 
  _StageCacheMap;


class Workspace {

public:
  enum STAGE_TYPE {
    INPUT,
    EXEC,
    OUTPUT
  };

  Workspace();
  ~Workspace();

  void OpenStage(const std::string& filename);
  void OpenStage(const pxr::UsdStageRefPtr& stage);
  void ClearAllStages();
  void RemoveStage(const std::string& name);
  void RemoveStage(const pxr::SdfPath& path);

  void InitExec();
  void UpdateExec(double time);
  void TerminateExec();
 
  pxr::SdfLayerHandle AddLayerFromDisk(const std::string& filename);
  pxr::UsdStageRefPtr& AddExecStage();
  void RemoveExecStage();
  pxr::UsdStageRefPtr& AddStageFromMemory(const std::string& name);
  pxr::UsdStageRefPtr& AddStageFromDisk(const std::string& filename);
  pxr::UsdStageRefPtr& GetWorkStage() { return _workStage; };
  pxr::UsdStageRefPtr& GetExecStage() { return _execStage; };
  pxr::UsdStageRefPtr& GetDisplayStage() { 
    return _execInitialized ? _execStage : _workStage; 
  };
  
private:
  pxr::UsdStageRefPtr _workStage;
  pxr::UsdStageRefPtr _execStage;
  _StageCacheMap      _allStages;

  Scene*              _execScene;
  bool                _execInitialized;
};


PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_Workspace_H