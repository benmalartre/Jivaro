#ifndef JVR_APPLICATION_MODEL_H
#define JVR_APPLICATION_MODEL_H

#include <pxr/base/gf/vec3d.h>
#include <pxr/imaging/hd/mergingSceneIndex.h>
#include <pxr/imaging/hd/sceneIndex.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/metrics.h>
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

class Model : public TfWeakBase
{
public:
  // constructor
  Model();

  // destructor
  ~Model();

  // selection
  Selection* GetSelection(){return &_selection;};
  void SetSelection(const SdfPathVector& selection);
  void ToggleSelection(const SdfPathVector& selection);
  void AddToSelection(const SdfPathVector& path);
  void RemoveFromSelection(const SdfPathVector& path);
  void ClearSelection();
  GfBBox3d GetSelectionBoundingBox();
  GfBBox3d GetStageBoundingBox();

  // stage 
  UsdStageRefPtr& GetStage(){return _stage;};
  void SetStage(UsdStageRefPtr& stage);

  // layer
  SdfLayerRefPtr GetSessionLayer();
  SdfLayerRefPtr GetRootLayer();

  UsdPrim GetUsdPrim(SdfPath primPath);
  UsdPrimRange GetAllPrims();

  void SetEmptyStage();
  void LoadUsdStage(const std::string usdFilePath);


private:

  // filename
  std::string                       _filename;

  // selection 
  Selection                         _selection;

  // stage
  UsdStageRefPtr                    _stage;

  // layer
  SdfLayerRefPtr                    _rootLayer;
  SdfLayerRefPtr                    _sessionLayer;
};

JVR_NAMESPACE_CLOSE_SCOPE // namespace JVR

#endif // JVR_APPLICATION_APPLICATION_H

