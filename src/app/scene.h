#pragma once

#include "../common.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/base/tf/hashmap.h>


AMN_NAMESPACE_OPEN_SCOPE

typedef pxr::TfHashMap< pxr::SdfPath, pxr::UsdStageRefPtr, pxr::SdfPath::Hash > _StageCacheMap;

class Scene {
public:
  Scene();
  ~Scene();

  void ClearAllStages();
  void RemoveStage(const std::string& name);
  void RemoveStage(const pxr::SdfPath& path);
  pxr::UsdStageRefPtr& AddStageFromMemory(const std::string& name);
  pxr::UsdStageRefPtr& AddStageFromDisk(const std::string& name, const std::string& filename);
  pxr::UsdStageRefPtr& GetRoot(){return _stage;};

private:
  pxr::UsdStageRefPtr _stage;
  _StageCacheMap  _childrens;
};


AMN_NAMESPACE_CLOSE_SCOPE