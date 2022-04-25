
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
{
  _workStage = pxr::UsdStage::CreateInMemory("root");
  _currentStage = _workStage;
  UndoRouter::Get().TrackLayer(_workStage->GetRootLayer());
}

Workspace::~Workspace()
{
  if(_execScene)delete _execScene;
  ClearAllStages();
  _currentStage = nullptr;
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
  _currentStage = _workStage;
  _allStages.clear();
}

void
Workspace::OpenStage(const pxr::UsdStageRefPtr& stage)
{
  _currentStage = stage;
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
  _allStages[path] = pxr::UsdStage::CreateInMemory(name);

  pxr::UsdStageRefPtr& stage = _allStages[path];
  pxr::UsdPrim ref = stage->DefinePrim(path);
  ref.GetReferences().AddReference(filename);
  stage->SetDefaultPrim(ref);

  pxr::SdfLayerHandle layer = stage->GetRootLayer();
  _workStage->GetRootLayer()->InsertSubLayerPath(layer->GetIdentifier());
  return _allStages[path];
  /*
  std::vector<std::string> tokens = SplitString(GetFileName(filename), ".");
  std::string name = tokens.front();
  pxr::SdfPath path("/" + name);
  pxr::SdfLayerRefPtr layer = pxr::SdfLayer::FindOrOpen(filename);
  pxr::SdfLayerRefPtr sessionLayer = pxr::SdfLayer::CreateAnonymous();
  _allStages[path] = pxr::UsdStage::Open(layer, sessionLayer);
  _rootStage->GetRootLayer()->InsertSubLayerPath(layer->GetIdentifier());
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
  std::cout << "CREATE EXECUTION STAGE..." << std::endl;
  _execStage = NULL;
  _execInitialized = false;

  InitExec();

  std::cout << "TRANSFERED CONTENT..." << std::endl;
}

void 
Workspace::InitExec()
{
  if (!_execInitialized) {
    _execScene = new Scene();
    _execStage = _execScene->GetStage();
    _execStage->GetRootLayer()->TransferContent(
      _workStage->GetRootLayer());
    pxr::UsdPrimRange primRange = _workStage->TraverseAll();
    for (pxr::UsdPrim prim : primRange) {
      if (prim.IsA<pxr::UsdGeomMesh>()) {
        std::cout << "## ADD MESH IN MEMORY : " << prim.GetPath() << std::endl;
        _execScene->AddMesh(prim.GetPath());
      }
    }
    
    _execInitialized = true;
  }
}

void 
Workspace::UpdateExec(double time)
{
  for (auto& meshMapIt : _execScene->GetMeshes()) {
    pxr::SdfPath path = meshMapIt.first;
    Mesh* mesh = &meshMapIt.second;
    pxr::VtArray<pxr::GfVec3f> positions = mesh->GetPositions();
    for (auto& position : positions) {
      position += pxr::GfVec3f(
        RANDOM_0_1,
        RANDOM_0_1,
        RANDOM_0_1
      ) * 10.0;
    }
    mesh->Update(positions);
  }
  _execScene->Update(time);
}

void 
Workspace::TerminateExec()
{

}

PXR_NAMESPACE_CLOSE_SCOPE