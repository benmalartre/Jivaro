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
#include "../app/application.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/sampler.h"

#include <iostream>

JVR_NAMESPACE_OPEN_SCOPE

PXR_NAMESPACE_USING_DIRECTIVE

TF_DEFINE_PRIVATE_TOKENS(
  _tokens,
  (meshPoints)
  (pickables)
);

TF_DEFINE_ENV_SETTING(JVR_ENGINE_EXEC_SCENE_DELEGATE_ID, "/",
  "Default Jivaro scene delegate id");

TF_DEFINE_ENV_SETTING(JVR_ENGINE_ENABLE_SCENE_INDEX, false,
  "Use Scene Index API for imaging scene input");

SdfPath const&
_GetUsdImagingDelegateId()
{
  static SdfPath const delegateId =
    SdfPath(TfGetEnvSetting(JVR_ENGINE_EXEC_SCENE_DELEGATE_ID));

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

Engine::Engine(const HdDriver& driver)
  : Engine(SdfPath::AbsoluteRootPath(), {}, {}, 
    _GetUsdImagingDelegateId(), driver)
{
  //SetRendererAov()
}

void Engine::InitExec(Scene* scene)
{
  _delegate = new Delegate(_GetRenderIndex(), _GetUsdImagingDelegateId());
  _delegate->SetScene(scene);
  // Add a meshPoints repr since it isn't populated in 
    // HdRenderIndex::_ConfigureReprs
    HdMesh::ConfigureRepr(_tokens->meshPoints,
                          HdMeshReprDesc(HdMeshGeomStylePoints,
                                         HdCullStyleNothing,
                                         HdMeshReprDescTokens->pointColor,
                                         /*flatShadingEnabled=*/true,
                                         /*blendWireframeColor=*/false));
}


void Engine::UpdateExec(double time)
{
  _delegate->UpdateScene();
}

void Engine::TerminateExec()
{
  _delegate->RemoveScene();
  delete(_delegate);
  _delegate = NULL;
}


Engine::Engine(
  const SdfPath& rootPath,
  const SdfPathVector& excludedPaths,
  const SdfPathVector& invisedPaths,
  const SdfPath& sceneDelegateID,
  const HdDriver& driver)
  : UsdImagingGLEngine(
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
  const GfMatrix4d& viewMatrix,
  const GfMatrix4d& projectionMatrix,
  const UsdPrim& root,
  const UsdImagingGLRenderParams& params,
  GfVec3d* outHitPoint,
  GfVec3d* outHitNormal,
  SdfPath* outHitPrimPath,
  SdfPath* outHitInstancerPath,
  int* outHitInstanceIndex,
  HdInstancerContext* outInstancerContext)
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



  /*
  
  HdxPickHitVector allHits;
    HdxPickTaskContextParams p;
    p.resolution = HdxUnitTestUtils::CalculatePickResolution(
        startPos, endPos, GfVec2i(4,4));
    p.pickTarget = pickTarget;
    p.resolveMode = HdxPickTokens->resolveUnique;
    p.viewMatrix = viewMatrix;
    p.projectionMatrix = HdxUnitTestUtils::ComputePickingProjectionMatrix(
        startPos, endPos, GfVec2i(width, height), frustum);
    p.collection = _pickablesCol;
    p.outHits = &allHits;

    HdTaskSharedPtrVector tasks;
    tasks.push_back(GetDelegate().GetRenderIndex().GetTask(
        SdfPath("/pickTask")));
    VtValue pickParams(p);
    _GetEngine()->SetTaskContextData(HdxPickTokens->pickParams, pickParams);
    _GetEngine()->Execute(&GetDelegate().GetRenderIndex(), &tasks);

    return HdxUnitTestUtils::TranslateHitsToSelection(
        p.pickTarget, HdSelection::HighlightModeSelect, allHits);
  
  */

  HdxPickHitVector allHits;
  HdxPickTaskContextParams pickParams;
  pickParams.resolveMode = HdxPickTokens->resolveNearestToCenter;
  pickParams.viewMatrix = viewMatrix;
  pickParams.projectionMatrix = projectionMatrix;
  pickParams.clipPlanes = params.clipPlanes;
  pickParams.collection = _intersectCollection;
  pickParams.outHits = &allHits;
  const VtValue vtPickParams(pickParams);

  _GetHdEngine()->SetTaskContextData(HdxPickTokens->pickParams, vtPickParams);
  _Execute(params, _taskController->GetPickingTasks());

  // Since we are in nearest-hit mode, we expect allHits to have
  // a single point in it.
  if (allHits.size() != 1) {
    return false;
  }

  HdxPickHit& hit = allHits[0];

  if (outHitPoint) {
    *outHitPoint = hit.worldSpaceHitPoint;
  }

  if (outHitNormal) {
    *outHitNormal = hit.worldSpaceHitNormal;
  }

  if(!hit.objectId.IsEmpty() && !_CheckPrimSelectable(hit.objectId)) return false;

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
  SdfPath* outHitPrimPath,
  SdfPath* outHitInstancerPath,
  int* outHitInstanceIndex,
  HdInstancerContext* outInstancerContext)
{

  if (_GetUseSceneIndices()) {
    // XXX(HYD-2299): picking
    return false;
  }

  TF_VERIFY(_GetSceneDelegate());

  const int primId = HdxPickTask::DecodeIDRenderColor(primIdColor);
  const int instanceIdx = HdxPickTask::DecodeIDRenderColor(instanceIdColor);
  SdfPath primPath =
    _GetSceneDelegate()->GetRenderIndex().GetRprimPathFromPrimId(primId);

  if (primPath.IsEmpty()) {
    return false;
  }

  SdfPath delegateId, instancerId;
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

bool
Engine::_CheckPrimSelectable(const SdfPath &path)
{
  Scene* scene = _delegate->GetScene();
  if(!scene->GetPrim(path)->geom->IsInput())return false;
  return true;
}


JVR_NAMESPACE_CLOSE_SCOPE
