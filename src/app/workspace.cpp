
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usd/primRange.h>


#include "../utils/strings.h"
#include "../utils/files.h"
#include "../app/workspace.h"
#include "../app/application.h"
#include "../app/time.h"
#include "../command/router.h"
#include "../command/block.h"

PXR_NAMESPACE_OPEN_SCOPE

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
  ClearAllStages();
  _workStage = nullptr;
}

void Workspace::ClearAllStages() 
{
  _allStages.clear();
}

void Workspace::RemoveStage(const std::string& name)
{
  pxr::SdfPath path(name);
  RemoveStage(path);
}


void Workspace::RemoveStage(const pxr::SdfPath& path)
{
  if(_allStages.find(path) != _allStages.end()) {
    _allStages.erase(path);
  }
}

void
Workspace::OpenStage(const std::string& filename)
{
  _workStage = pxr::UsdStage::Open(filename);
  _allStages.clear();
}

void
Workspace::OpenStage(const pxr::UsdStageRefPtr& stage)
{
  _workStage = stage;
}

pxr::UsdStageRefPtr&
Workspace::AddStageFromMemory(const std::string& name)
{
  pxr::SdfPath path(name);
  _allStages[path] = pxr::UsdStage::CreateInMemory(name);
  return  _allStages[path];
}

pxr::UsdStageRefPtr&
Workspace::AddStageFromDisk(const std::string& filename)
{
  
  std::vector<std::string> tokens = SplitString(GetFileName(filename), ".");
  std::string name = tokens.front();
  pxr::SdfPath path("/" + name);
  UndoBlock editBlock;
  pxr::UsdStageRefPtr& stage = pxr::UsdStage::Open(filename);

  _allStages[path] = stage;
  pxr::SdfLayerHandle layer = stage->GetRootLayer();
  _workStage->GetRootLayer()->InsertSubLayerPath(layer->GetIdentifier());
  _workStage->SetDefaultPrim(stage->GetDefaultPrim());
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
    _execStage = UsdStage::CreateInMemory("EXEC");
    _execScene = new Scene(_execStage);
    
    _execStage->GetRootLayer()->TransferContent(
      _workStage->GetRootLayer());
    pxr::UsdPrimRange primRange = _execStage->TraverseAll();
    for (pxr::UsdPrim prim : primRange) {
      if (prim.IsA<pxr::UsdGeomMesh>()) {
        _execScene->AddMesh(prim.GetPath());
      }
    }
    _execStage->SetDefaultPrim(_execStage->GetPrimAtPath(
      _workStage->GetDefaultPrim().GetPath()));

    _execInitialized = true;
  }
}

void 
Workspace::UpdateExec(double time)
{
  for (auto& meshMapIt : _execScene->GetMeshes()) {
    pxr::SdfPath path = meshMapIt.first;
    Mesh* mesh = &meshMapIt.second;
    pxr::VtArray<pxr::GfVec3f> positions;
    pxr::UsdGeomMesh input(_workStage->GetPrimAtPath(path));

    input.GetPointsAttr().Get(&positions, time);
    for (auto& position : positions) {
      position += pxr::GfVec3f(
        RANDOM_0_1,
        RANDOM_0_1,
        RANDOM_0_1
      ) * pxr::GfSin(GetApplication()->GetTime().GetActiveTime() * 100);
    }
    mesh->Update(positions);
  }
  _execScene->Update(time);
}

void 
Workspace::TerminateExec()
{
  std::cout << "TERMINATE EXEC " << std::endl;
}

PXR_NAMESPACE_CLOSE_SCOPE