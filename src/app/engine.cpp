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

void Engine::InitExec()
{
  if (!_scene)_scene = new Scene(_GetRenderIndex(), _GetUsdImagingDelegateId());
  Application* app = GetApplication();
  pxr::UsdStageWeakPtr stage = app->GetStage();
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  pxr::SdfPath rootId = rootPrim.GetPath().AppendChild(pxr::TfToken("test"));
  

  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());

  pxr::UsdPrimRange primRange = stage->TraverseAll();
  for (pxr::UsdPrim prim : primRange) {
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::TfToken meshName(prim.GetName().GetString() + "RT");
      pxr::SdfPath meshPath(rootId.AppendChild(meshName));
      _srcMap[meshPath] = prim.GetPath();
      Mesh* mesh = _scene->AddMesh(meshPath);
      pxr::UsdGeomMesh usdMesh(prim);
      pxr::VtArray<pxr::GfVec3f> positions;
      usdMesh.GetPointsAttr().Get(&positions);

      pxr::VtArray<int> counts;
      pxr::VtArray<int> indices;
      usdMesh.GetFaceVertexCountsAttr().Get(&counts);
      usdMesh.GetFaceVertexIndicesAttr().Get(&indices);
      pxr::VtArray<Triangle> triangles;
      TriangulateMesh(counts, indices, triangles);
      pxr::VtArray<pxr::GfVec3f> normals;
      ComputeVertexNormals(positions, counts, indices, triangles, normals);
      
      Sampler::PoissonSampling(0.01, 64000, positions, normals, triangles, _samples);

      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
     
      pxr::VtArray<pxr::GfVec3f> points(_samples.size());
      for (size_t sampleIdx = 0; sampleIdx < _samples.size(); ++sampleIdx) {
        points[sampleIdx] = xform.Transform(_samples[sampleIdx].GetPosition(&positions[0]));
      }
      mesh->MaterializeSamples(points, 2.f);
      pxr::HdChangeTracker& tracker = _scene->GetRenderIndex().GetChangeTracker();
      tracker.MarkRprimDirty(meshPath, pxr::HdChangeTracker::DirtyTopology);
    }
  }
}


static VtVec3fArray _AnimatePositions(VtVec3fArray const& positions, float time)
{
  VtVec3fArray result = positions;
  for (size_t i = 0; i < result.size(); ++i) {
    result[i] += GfVec3f((float)(0.5 * sin(0.5 * i + time)), 0, 0);
  }
  return result;
}

void Engine::UpdateExec(double time)
{
  pxr::UsdStageRefPtr stage = GetApplication()->GetStage();
  pxr::UsdGeomXformCache xformCache(time);
  for (auto& execPrim : _scene->GetPrims()) {
    pxr::UsdPrim usdPrim = stage->GetPrimAtPath(_srcMap[execPrim.first]);
    pxr::UsdGeomMesh usdMesh(usdPrim);
    pxr::VtArray<pxr::GfVec3f> positions;
    usdMesh.GetPointsAttr().Get(&positions, pxr::UsdTimeCode(time));

    pxr::VtArray<int> counts;
    pxr::VtArray<int> indices;
    usdMesh.GetFaceVertexCountsAttr().Get(&counts);
    usdMesh.GetFaceVertexIndicesAttr().Get(&indices);
    pxr::VtArray<Triangle> triangles;
    TriangulateMesh(counts, indices, triangles);
    pxr::VtArray<pxr::GfVec3f> normals;
    ComputeVertexNormals(positions, counts, indices, triangles, normals);

    pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(usdPrim);

    pxr::VtArray<pxr::GfVec3f>& points = execPrim.second->GetPositions();
    for (size_t sampleIdx = 0; sampleIdx < _samples.size(); ++sampleIdx) {
      const pxr::GfVec3f& normal = _samples[sampleIdx].GetNormal(&normals[0]);
      const pxr::GfVec3f& tangent = _samples[sampleIdx].GetTangent(&positions[0], &normals[0]);
      const pxr::GfVec3f bitangent = (normal ^ tangent).GetNormalized();
      const pxr::GfVec3f& position = _samples[sampleIdx].GetPosition(&positions[0]);
      points[sampleIdx * 3] = xform.Transform(position - tangent * 0.02f);
      points[sampleIdx * 3 + 1] = xform.Transform(position + bitangent + normal * 0.4f * (1.5f + pxr::GfSin(position[2]*0.4 + time*0.2) * 0.5f));
      points[sampleIdx * 3 + 2] = xform.Transform(position + tangent * 0.02f);
    }

    //mesh.second.Randomize(0.01);
    HdChangeTracker& tracker = _scene->GetRenderIndex().GetChangeTracker();
    tracker.MarkRprimDirty(execPrim.first, HdChangeTracker::DirtyPoints);
  }
}

void Engine::TerminateExec()
{
  Application* app = GetApplication();
  pxr::UsdStageWeakPtr stage = app->GetStage();
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  pxr::SdfPath rootId = rootPrim.GetPath().AppendChild(pxr::TfToken("test"));

  pxr::UsdPrimRange primRange = stage->TraverseAll();

  for (pxr::UsdPrim prim : primRange) {
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::TfToken meshName(prim.GetName().GetString() + "RT");
      pxr::SdfPath meshPath(rootId.AppendChild(meshName));
      _scene->Remove(meshPath);
    }
  }
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
  , _scene(NULL)
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
