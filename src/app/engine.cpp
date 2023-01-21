#include "pxr/imaging/garch/glApi.h"
#include "pxr/imaging/glf/contextCaps.h"
#include "pxr/imaging/glf/glContext.h"
#include "pxr/imaging/hdx/taskController.h"
#include "pxr/imaging/hd/mesh.h"
#include "pxr/usdImaging/usdImagingGL/engine.h"
#include "pxr/usdImaging/usdImaging/delegate.h"
#include "pxr/usdImaging/usdImaging/stageSceneIndex.h"
#include "pxr/imaging/hd/flatteningSceneIndex.h"
#include "pxr/imaging/hdx/pickTask.h"
#include "pxr/imaging/hdx/taskController.h"
#include "pxr/imaging/hdx/tokens.h"
#include "pxr/base/tf/envSetting.h"
#include "pxr/base/tf/diagnostic.h"
#include "pxr/base/tf/callContext.h"
#include "../app/engine.h"

#include <iostream>

JVR_NAMESPACE_OPEN_SCOPE

PXR_NAMESPACE_USING_DIRECTIVE

TF_DEFINE_ENV_SETTING(JVR_ENGINE_DEBUG_SCENE_DELEGATE_ID, "/",
  "Default Jivaro scene delegate id");

TF_DEFINE_ENV_SETTING(JVR_ENGINE_ENABLE_SCENE_INDEX, false,
  "Use Scene Index API for imaging scene input");

pxr::SdfPath const&
_GetUsdImagingDelegateId()
{
  static pxr::SdfPath const delegateId =
    pxr::SdfPath(pxr::TfGetEnvSetting(JVR_ENGINE_DEBUG_SCENE_DELEGATE_ID));

  return delegateId;
}

bool
_GetUseSceneIndices()
{
  // Use UsdImagingStageSceneIndex for input if:
  // - USDIMAGINGGL_ENGINE_ENABLE_SCENE_INDEX is true (feature flag)
  // - HdRenderIndex has scene index emulation enabled (otherwise,
  //     AddInputScene won't work).
  static bool useSceneIndices =
    HdRenderIndex::IsSceneIndexEmulationEnabled() &&
    (TfGetEnvSetting(JVR_ENGINE_ENABLE_SCENE_INDEX) == true);

  return useSceneIndices;
}

Engine::Engine(const pxr::HdDriver& driver)
  : Engine(pxr::SdfPath::AbsoluteRootPath(), {}, {}, 
    _GetUsdImagingDelegateId(), driver)
{
  //SetRendererAov()
}


Engine::Engine(
  const pxr::SdfPath& rootPath,
  const pxr::SdfPathVector& excludedPaths,
  const pxr::SdfPathVector& invisedPaths,
  const pxr::SdfPath& sceneDelegateID,
  const pxr::HdDriver& driver)
  : pxr::UsdImagingGLEngine(
    rootPath,
    excludedPaths,
    invisedPaths,
    sceneDelegateID,
    driver)
  , _dirty(true)
{
}

Engine::~Engine() = default;

//----------------------------------------------------------------------------
// Picking
//----------------------------------------------------------------------------

bool
Engine::TestIntersection(
  const pxr::GfMatrix4d& viewMatrix,
  const pxr::GfMatrix4d& projectionMatrix,
  const pxr::UsdPrim& root,
  const pxr::UsdImagingGLRenderParams& params,
  pxr::GfVec3d* outHitPoint,
  pxr::GfVec3d* outHitNormal,
  pxr::SdfPath* outHitPrimPath,
  pxr::SdfPath* outHitInstancerPath,
  int* outHitInstanceIndex,
  pxr::HdInstancerContext* outInstancerContext)
{

  if (_GetUseSceneIndices()) {
    // XXX(HYD-2299): picking support
    return false;
  }

  TF_VERIFY(_GetSceneDelegate());
  TF_VERIFY(_GetTaskController());

  PrepareBatch(root, params);

  // XXX(UsdImagingPaths): This is incorrect...  "Root" points to a USD
  // subtree, but the subtree in the hydra namespace might be very different
  // (e.g. for native instancing).  We need a translation step.
  const SdfPathVector paths = {
      root.GetPath().ReplacePrefix(
          SdfPath::AbsoluteRootPath(), _sceneDelegateId)
  };
  _UpdateHydraCollection(&_intersectCollection, paths, params);

  _PrepareRender(params);

  pxr::HdxPickHitVector allHits;
  pxr::HdxPickTaskContextParams pickParams;
  pickParams.resolveMode = pxr::HdxPickTokens->resolveNearestToCenter;
  pickParams.viewMatrix = viewMatrix;
  pickParams.projectionMatrix = projectionMatrix;
  pickParams.clipPlanes = params.clipPlanes;
  pickParams.collection = _intersectCollection;
  pickParams.outHits = &allHits;
  const pxr::VtValue vtPickParams(pickParams);

  _GetHdEngine()->SetTaskContextData(pxr::HdxPickTokens->pickParams, vtPickParams);
  _Execute(params, _taskController->GetPickingTasks());

  // Since we are in nearest-hit mode, we expect allHits to have
  // a single point in it.
  if (allHits.size() != 1) {
    return false;
  }

  pxr::HdxPickHit& hit = allHits[0];

  if (outHitPoint) {
    *outHitPoint = hit.worldSpaceHitPoint;
  }

  if (outHitNormal) {
    *outHitNormal = hit.worldSpaceHitNormal;
  }

  hit.objectId = _GetSceneDelegate()->GetScenePrimPath(
    hit.objectId, hit.instanceIndex, outInstancerContext);
  hit.instancerId = _GetSceneDelegate()->ConvertIndexPathToCachePath(
    hit.instancerId).GetAbsoluteRootOrPrimPath();

  if (outHitPrimPath) {
    *outHitPrimPath = hit.objectId;
  }
  if (outHitInstancerPath) {
    *outHitInstancerPath = hit.instancerId;
  }
  if (outHitInstanceIndex) {
    *outHitInstanceIndex = hit.instanceIndex;
  }

  return true;
}

bool
Engine::DecodeIntersection(
  unsigned char const primIdColor[4],
  unsigned char const instanceIdColor[4],
  pxr::SdfPath* outHitPrimPath,
  pxr::SdfPath* outHitInstancerPath,
  int* outHitInstanceIndex,
  pxr::HdInstancerContext* outInstancerContext)
{

  if (_GetUseSceneIndices()) {
    // XXX(HYD-2299): picking
    return false;
  }

  TF_VERIFY(_GetSceneDelegate());

  const int primId = pxr::HdxPickTask::DecodeIDRenderColor(primIdColor);
  const int instanceIdx = pxr::HdxPickTask::DecodeIDRenderColor(instanceIdColor);
  pxr::SdfPath primPath =
    _GetSceneDelegate()->GetRenderIndex().GetRprimPathFromPrimId(primId);

  if (primPath.IsEmpty()) {
    return false;
  }

  pxr::SdfPath delegateId, instancerId;
  _GetSceneDelegate()->GetRenderIndex().GetSceneDelegateAndInstancerIds(
    primPath, &delegateId, &instancerId);

  primPath = _GetSceneDelegate()->GetScenePrimPath(
    primPath, instanceIdx, outInstancerContext);
  instancerId = _GetSceneDelegate()->ConvertIndexPathToCachePath(instancerId)
    .GetAbsoluteRootOrPrimPath();

  if (outHitPrimPath) {
    *outHitPrimPath = primPath;
  }
  if (outHitInstancerPath) {
    *outHitInstancerPath = instancerId;
  }
  if (outHitInstanceIndex) {
    *outHitInstanceIndex = instanceIdx;
  }

  return true;
}


JVR_NAMESPACE_CLOSE_SCOPE
