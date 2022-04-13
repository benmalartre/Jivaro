#ifndef JVR_APPLICATION_WORKSPACE_H
#define JVR_APPLICATION_WORKSPACE_H

#include "../common.h"
#include "../app/scene.h"
#include <pxr/usd/usd/stage.h>
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
  void SetCurrentStage(pxr::UsdStageRefPtr& stage) { _currentStage = stage; };
  void ClearAllStages();
  void RemoveStage(const std::string& name);
  void RemoveStage(const pxr::SdfPath& path);
  pxr::UsdStageRefPtr& AddStageFromMemory(const std::string& name);
  pxr::UsdStageRefPtr& AddStageFromDisk(const std::string& filename);
  pxr::UsdStageRefPtr& GetRootStage() { return _rootStage; };
  pxr::UsdStageRefPtr& GetCurrentStage() { return _currentStage; };

private:
  pxr::UsdStageRefPtr _currentStage;
  pxr::UsdStageRefPtr _rootStage;
  _StageCacheMap      _allStages;

  Scene               _scene;
};


PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_Workspace_H