
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/pointInstancer.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/sdf/fileFormat.h>


#include "../utils/strings.h"
#include "../utils/files.h"
#include "../app/workspace.h"
#include "../app/application.h"
#include "../app/time.h"
#include "../command/router.h"
#include "../command/block.h"
#include "../acceleration/bvh.h"

JVR_NAMESPACE_OPEN_SCOPE

Workspace::Workspace() 
  : _execInitialized(false)
  , _execScene(NULL)
  , _execStage(NULL)
{
  _workStage = pxr::UsdStage::CreateInMemory("work");
  UndoRouter::Get().TrackLayer(_workStage->GetRootLayer());
}

Workspace::~Workspace()
{
  if(_execScene)delete _execScene;
  ClearStageCache();
  _workStage = nullptr;
}

bool 
Workspace::HasUnsavedWork()
{
  for (const auto& layer : SdfLayer::GetLoadedLayers()) {
    if (layer && layer->IsDirty() && !layer->IsAnonymous()) {
      return true;
    }
  }
  return false;
}

void 
Workspace::SetWorkStage(pxr::UsdStageCache::Id current)
{
  SetWorkStage(_stageCache.Find(current));
}

void 
Workspace::SetWorkStage(pxr::UsdStageRefPtr stage)
{
  _workStage = stage;
  UndoRouter::Get().TrackLayer(_workStage->GetRootLayer());
  // NOTE: We set the default layer to the current stage root
  // this might have side effects
  if (!GetWorkLayer() && _workStage) {
    SetWorkLayer(_workStage->GetRootLayer());
  }
}

void 
Workspace::SetWorkLayer(SdfLayerRefPtr layer) 
{
  if (!layer)
    return;
  if (!_layerHistory.empty()) {
    if (GetWorkLayer() != layer) {
      if (_layerHistoryPointer < _layerHistory.size() - 1) {
        _layerHistory.resize(_layerHistoryPointer + 1);
      }
      _layerHistory.push_back(layer);
      _layerHistoryPointer = _layerHistory.size() - 1;
    }
  }
  else {
    _layerHistory.push_back(layer);
    _layerHistoryPointer = _layerHistory.size() - 1;
  }
}

void 
Workspace::SetWorkEditTarget(SdfLayerHandle layer) 
{
  if (GetWorkStage()) {
    GetWorkStage()->SetEditTarget(UsdEditTarget(layer));
  }
}

SdfLayerRefPtr 
Workspace::GetWorkLayer() 
{
  return _layerHistory.empty() ? SdfLayerRefPtr() : _layerHistory[_layerHistoryPointer];
}

void 
Workspace::SetPreviousLayer() 
{
  if (_layerHistoryPointer > 0) {
    _layerHistoryPointer--;
  }
}


void 
Workspace::SetNextLayer() 
{
  if (_layerHistoryPointer < _layerHistory.size() - 1) {
    _layerHistoryPointer++;
  }
}


void 
Workspace::UseLayer(SdfLayerRefPtr layer)
{
  if (layer) {
    SetWorkLayer(layer);
    //_settings._showContentBrowser = true;
  }
}


void 
Workspace::CreateLayer(const std::string& path)
{
  auto newLayer = SdfLayer::CreateNew(path);
  UseLayer(newLayer);
}

void 
Workspace::ImportLayer(const std::string& path) 
{
  auto newLayer = SdfLayer::FindOrOpen(path);
  UseLayer(newLayer);
}

//
void 
Workspace::ImportStage(const std::string& path, bool openLoaded) 
{
  auto newStage = UsdStage::Open(path, openLoaded ? UsdStage::LoadAll : UsdStage::LoadNone); // TODO: as an option
  if (newStage) {
    _stageCache.Insert(newStage);
    SetWorkStage(newStage);
    //_settings._showContentBrowser = true;
    //_settings._showViewport = true;
    //UpdateRecentFiles(_settings._recentFiles, path);
  }
}

void 
Workspace::SaveCurrentLayerAs(const std::string& path)
{
  auto newLayer = SdfLayer::CreateNew(path);
  if (newLayer && GetWorkLayer()) {
    newLayer->TransferContent(GetWorkLayer());
    newLayer->Save();
    UseLayer(newLayer);
  }
}

void 
Workspace::CreateStage(const std::string& path)
{
  auto usdaFormat = pxr::SdfFileFormat::FindByExtension("usda");
  auto layer = SdfLayer::New(usdaFormat, path);
  if (layer) {
    auto newStage = UsdStage::Open(layer);
    if (newStage) {
      _stageCache.Insert(newStage);
      SetWorkStage(newStage);
      //_settings._showContentBrowser = true;
      //_settings._showViewport = true;
    }
  }
}

void Workspace::ClearStageCache() 
{
  _stageCache.Clear();
}


void
Workspace::OpenStage(const std::string& filename)
{
  _workStage = pxr::UsdStage::Open(filename);
  UndoRouter::Get().TrackLayer(_workStage->GetRootLayer());
  _stageCache.Insert(_workStage);
}

void
Workspace::OpenStage(const pxr::UsdStageRefPtr& stage)
{
  _workStage = stage;
  UndoRouter::Get().TrackLayer(_workStage->GetRootLayer());
}

pxr::UsdStageRefPtr&
Workspace::AddStageFromMemory(const std::string& name)
{
  /*
  _stageCache.Find()
  _alStages[path] = pxr::UsdStage::CreateInMemory(name);
  return  _allStages[path];
  */
  pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateInMemory(name);
  UndoRouter::Get().TrackLayer(stage->GetRootLayer());
  _stageCache.Insert(stage);
  return stage;
}

pxr::UsdStageRefPtr&
Workspace::AddStageFromDisk(const std::string& filename)
{
  
  std::vector<std::string> tokens = SplitString(GetFileName(filename), ".");
  std::string name = tokens.front();
  pxr::SdfPath path("/" + name);
  UndoBlock editBlock;
  pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(filename);
  _stageCache.Insert(stage);
  pxr::SdfLayerHandle layer = stage->GetRootLayer();
  _workStage = stage;
  UndoRouter::Get().TrackLayer(_workStage->GetRootLayer());
  //_workStage->GetRootLayer()->InsertSubLayerPath(layer->GetIdentifier());
  //_workStage->SetDefaultPrim(stage->GetDefaultPrim());
  return stage;
  /*
  std::vector<std::string> tokens = SplitString(GetFileName(filename), ".");
  std::string name = tokens.front();
  pxr::SdfPath path("/" + name);
  pxr::SdfLayerRefPtr layer = pxr::SdfLayer::FindOrOpen(filename);
  pxr::SdfLayerRefPtr sessionLayer = pxr::SdfLayer::CreateAnonymous();
  _allStages[path] = pxr::UsdStage::Open(layer, sessionLayer);
  _workStage->GetRootLayer()->InsertSubLayerPath(layer->GetIdentifier());
  return _allStages[path];
 */
}

pxr::SdfLayerHandle 
Workspace::AddLayerFromDisk(const std::string& filename)
{
  pxr::SdfLayerHandle handle = pxr::SdfLayer::FindOrOpen(filename);
  return handle;
}

pxr::UsdStageRefPtr&
Workspace::AddExecStage()
{
  _execStage = NULL;
  _execInitialized = false;

  InitExec();
  return _execStage;
}

void
Workspace::RemoveExecStage()
{
  _execStage = NULL;
  if (_execScene) {
    delete _execScene;
    _execScene = NULL;
  }
  _execInitialized = false;
}

void 
Workspace::InitExec()
{

  if (!_execInitialized) {
    _execStage = UsdStage::CreateInMemory("exec");
    _execScene = new Scene(_execStage);
   
    Time& time = GetApplication()->GetTime();
    _startFrame = time.GetStartTime();
    _lastFrame = time.GetActiveTime();
    
    _execStage->GetRootLayer()->TransferContent(_workStage->GetRootLayer());

    pxr::UsdGeomXformCache xformCache(_startFrame);

    pxr::UsdPrimRange primRange = _execStage->TraverseAll();
    for (pxr::UsdPrim prim : primRange) {
      if (prim.IsA<pxr::UsdGeomMesh>()) {
        _execScene->AddMesh(prim.GetPath());
        _solver.AddGeometry(&_execScene->GetMeshes()[prim.GetPath()], 
          pxr::GfMatrix4f(xformCache.GetLocalToWorldTransform(prim)));
      }
    }

    std::vector<Geometry*> colliders;
    for (auto& mesh : _execScene->GetMeshes()) {
      colliders.push_back(&mesh.second);
    }
    _solver.AddColliders(colliders);
    _execStage->SetDefaultPrim(_execStage->GetPrimAtPath(
      _workStage->GetDefaultPrim().GetPath()));

    pxr::UsdGeomPoints points =
      pxr::UsdGeomPoints::Define(
        _execStage, 
        _execStage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("System")));

    points.CreatePointsAttr().Set(pxr::VtValue(_solver.GetSystem().GetPositions()));

    size_t numParticles = _solver.GetSystem().GetNumParticles();
    
    pxr::VtArray<float> widths(numParticles);
    pxr::VtArray<pxr::GfVec3f> colors(numParticles);
    for(size_t p = 0; p < numParticles; ++p) {
      widths[p] = 0.02;
      colors[p] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    }
    points.CreateWidthsAttr().Set(pxr::VtValue(widths));
    points.SetWidthsInterpolation(pxr::UsdGeomTokens->varying);
    points.CreateDisplayColorAttr().Set(pxr::VtValue(colors));
    points.GetDisplayColorPrimvar().SetInterpolation(pxr::UsdGeomTokens->varying);
    
    _execInitialized = true;
  }
}

void 
Workspace::UpdateExec(double time)
{
  /*
  for (auto& meshMapIt : _execScene->GetMeshes()) {
    pxr::SdfPath path = meshMapIt.first;
    Mesh* mesh = &meshMapIt.second;
    pxr::VtArray<pxr::GfVec3f> positions;
    pxr::UsdGeomMesh input(_workStage->GetPrimAtPath(path));

    double t = pxr::GfSin(GetApplication()->GetTime().GetActiveTime() * 100);

    input.GetPointsAttr().Get(&positions, time);
    for (auto& position : positions) {
      position += pxr::GfVec3f(
        pxr::GfSin(position[0] + t),
        pxr::GfCos(position[0] + t) * 5.0,
        RANDOM_0_1 * 0.05
      );
    }
    mesh->Update(positions);
  }
  */
  if (time <= _startFrame) {
    _solver.Reset();
  }
  if(time > _lastFrame) {
    _solver.Step();
    _execScene->Update(time);
  }
  _lastFrame = (float)time;
}

void 
Workspace::TerminateExec()
{
  std::cout << "TERMINATE EXEC " << std::endl;
}

JVR_NAMESPACE_CLOSE_SCOPE