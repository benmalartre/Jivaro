#include "scene.h"
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>

AMN_NAMESPACE_OPEN_SCOPE

Scene::Scene()
{
  _stage = pxr::UsdStage::CreateInMemory();
}

Scene::~Scene()
{
  ClearAllStages();
  _stage = nullptr;
}

void Scene::ClearAllStages() 
{
  _childrens.clear();
}

void Scene::RemoveStage(const std::string& name)
{
  pxr::SdfPath path(name);
  RemoveStage(path);
}


void Scene::RemoveStage(const pxr::SdfPath& path)
{
  if(_childrens.find(path) != _childrens.end()) {

  }
  /*
  for (size_t i = 0; i < _childrens.size(); ++i) {
    if(_childrens[i] == stage) {
      _childrens.erase(_childrens.begin() + i);
      break;
    }
  }
  */
}


pxr::UsdStageRefPtr& Scene::AddStageFromMemory(const std::string& name)
{
  pxr::SdfPath path(name);
  _childrens[path] = pxr::UsdStage::CreateInMemory(name);
  return  _childrens[path];
}

pxr::UsdStageRefPtr& Scene::AddStageFromDisk(const std::string& name, const std::string& filename)
{
  pxr::SdfPath path("/" + name);
  _childrens[path] = pxr::UsdStage::CreateInMemory(name);
  pxr::UsdStageRefPtr& stage = _childrens[path];
  //pxr::UsdPrim root = pxr::UsdGeomXform::Define(_stage, pxr::SdfPath("/root")).GetPrim();
  pxr::UsdPrim ref = stage->OverridePrim(path);
  ref.GetReferences().AddReference(filename);
  stage->SetDefaultPrim(ref);
  _stage->GetLayerStack().push_back(stage->GetRootLayer());
  return stage;
}

AMN_NAMESPACE_CLOSE_SCOPE