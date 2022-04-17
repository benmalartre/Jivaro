
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/points.h>


#include "../utils/strings.h"
#include "../utils/files.h"
#include "../app/workspace.h"
#include "../command/router.h"
#include "../command/block.h"

PXR_NAMESPACE_OPEN_SCOPE

Workspace::Workspace()
{
  _rootStage = pxr::UsdStage::CreateInMemory("root");
  _currentStage = _rootStage;
  UndoRouter::Get().TrackLayer(_rootStage->GetRootLayer());
}

Workspace::~Workspace()
{
  ClearAllStages();
  _currentStage = nullptr;
  _rootStage = nullptr;
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
  _rootStage = pxr::UsdStage::Open(filename);
  _currentStage = _rootStage;
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
  _rootStage->GetRootLayer()->InsertSubLayerPath(layer->GetIdentifier());
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

PXR_NAMESPACE_CLOSE_SCOPE