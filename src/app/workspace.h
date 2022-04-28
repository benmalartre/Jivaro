#ifndef JVR_APPLICATION_WORKSPACE_H
#define JVR_APPLICATION_WORKSPACE_H

#include "../common.h"
#include "../app/scene.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stageCache.h>
#include <pxr/base/tf/hashmap.h>


PXR_NAMESPACE_OPEN_SCOPE

class Workspace {

public:
  enum STAGE_TYPE {
    INPUT,
    EXEC,
    OUTPUT
  };

  Workspace();
  ~Workspace();

  bool HasUnsavedWork();

  void OpenStage(const std::string& filename);
  void OpenStage(const pxr::UsdStageRefPtr& stage);
  void ClearStageCache();

  void SetCurrentLayer(pxr::SdfLayerRefPtr layer);
  pxr::SdfLayerRefPtr GetCurrentLayer();
  void SetPreviousLayer();
  void SetNextLayer();

  /// Create a new layer in file path
  void CreateLayer(const std::string& path);
  void ImportLayer(const std::string& path);
  void CreateStage(const std::string& path);
  void ImportStage(const std::string& path, bool openLoaded = true);
  void SaveCurrentLayerAs(const std::string& path);

  void SetWorkStage(UsdStageCache::Id current);
  void SetWorkStage(UsdStageRefPtr stage);
  void SetWorkEditTarget(SdfLayerHandle layer);

  UsdStageCache& GetStageCache() { return _stageCache; }

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
  pxr::UsdStageRefPtr       _workStage;
  pxr::UsdStageRefPtr       _execStage;
  pxr::UsdStageCache        _stageCache;

  pxr::SdfLayerRefPtrVector _layerHistory;
  size_t                    _layerHistoryPointer;

  Scene*                    _execScene;
  bool                      _execInitialized;
};


PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_Workspace_H