#include "scene.h"
#include "../utils/strings.h"
#include "../utils/files.h"
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>

AMN_NAMESPACE_OPEN_SCOPE

Scene::Scene()
{
  _rootStage = pxr::UsdStage::CreateInMemory("root");
  _currentStage = _rootStage;
}

Scene::~Scene()
{
  ClearAllStages();
  _currentStage = nullptr;
  _rootStage = nullptr;
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
    _childrens.erase(path);
  }
}


pxr::UsdStageRefPtr& Scene::AddStageFromMemory(const std::string& name)
{
  pxr::SdfPath path(name);
  _childrens[path] = pxr::UsdStage::CreateInMemory(name);
  return  _childrens[path];
}

pxr::UsdStageRefPtr& Scene::AddStageFromDisk(const std::string& filename)
{
  std::vector<std::string> tokens = SplitString(GetFileName(filename), ".");
  std::string name = tokens.front();
  pxr::SdfPath path("/" + name);
  _childrens[path] = pxr::UsdStage::CreateInMemory(name);

  pxr::UsdStageRefPtr& stage = _childrens[path];
  pxr::UsdPrim ref = stage->OverridePrim(path);
  ref.GetReferences().AddReference(filename);
  stage->SetDefaultPrim(ref);

  _rootStage->GetRootLayer()->InsertSubLayerPath(stage->GetRootLayer()->GetIdentifier());
}

void Scene::TestVoronoi()
{
  pxr::SdfPath path("/Voronoi");
  pxr::UsdStageRefPtr stage = AddStageFromMemory("Voronoi");
  _childrens[path] = stage;
  Mesh mesh;
  std::vector<pxr::GfVec3f> points(1024);
  for (auto& point : points) {
    point[0] = (float)rand() / (float)RAND_MAX;
    point[1] = 0.f;
    point[2] = (float)rand() / (float)RAND_MAX;
  }
  mesh.VoronoiDiagram(points);
  pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh::Define(stage, path);
  usdMesh.CreatePointsAttr(pxr::VtValue(mesh.GetPositions()));
  usdMesh.CreateNormalsAttr(pxr::VtValue(mesh.GetNormals()));
  usdMesh.CreateFaceVertexIndicesAttr(pxr::VtValue(mesh.GetFaceConnects()));
  usdMesh.CreateFaceVertexCountsAttr(pxr::VtValue(mesh.GetFaceCounts()));

  usdMesh.CreateDisplayColorAttr(pxr::VtValue(mesh.GetDisplayColor()));
  pxr::UsdGeomPrimvar displayColorPrimvar = usdMesh.GetDisplayColorPrimvar();
  GeomInterpolation colorInterpolation = mesh.GetDisplayColorInterpolation();

  switch (colorInterpolation) {
  case GeomInterpolationConstant:
    displayColorPrimvar.SetInterpolation(pxr::UsdGeomTokens->constant);
    break;
  case GeomInterpolationUniform:
    displayColorPrimvar.SetInterpolation(pxr::UsdGeomTokens->uniform);
    break;
  case GeomInterpolationVertex:
    displayColorPrimvar.SetInterpolation(pxr::UsdGeomTokens->vertex);
    break;
  case GeomInterpolationVarying:
    displayColorPrimvar.SetInterpolation(pxr::UsdGeomTokens->varying);
    break;
  case GeomInterpolationFaceVarying:
    displayColorPrimvar.SetInterpolation(pxr::UsdGeomTokens->faceVarying);
    break;
  default:
    break;
  }

  usdMesh.CreateSubdivisionSchemeAttr(pxr::VtValue(pxr::UsdGeomTokens->none));

  stage->SetDefaultPrim(usdMesh.GetPrim());
  _rootStage->GetRootLayer()->InsertSubLayerPath(stage->GetRootLayer()->GetIdentifier());
}

AMN_NAMESPACE_CLOSE_SCOPE