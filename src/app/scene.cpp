#include "scene.h"
#include "../utils/strings.h"
#include "../utils/files.h"
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/points.h>

PXR_NAMESPACE_OPEN_SCOPE

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
  _allStages.clear();
}

void Scene::RemoveStage(const std::string& name)
{
  pxr::SdfPath path(name);
  RemoveStage(path);
}


void Scene::RemoveStage(const pxr::SdfPath& path)
{
  if(_allStages.find(path) != _allStages.end()) {
    _allStages.erase(path);
  }
}


pxr::UsdStageRefPtr& Scene::AddStageFromMemory(const std::string& name)
{
  pxr::SdfPath path(name);
  _allStages[path] = pxr::UsdStage::CreateInMemory(name);
  return  _allStages[path];
}

pxr::UsdStageRefPtr& Scene::AddStageFromDisk(const std::string& filename)
{
  std::vector<std::string> tokens = SplitString(GetFileName(filename), ".");
  std::string name = tokens.front();
  pxr::SdfPath path("/" + name);
  _allStages[path] = pxr::UsdStage::CreateInMemory(name);

  pxr::UsdStageRefPtr& stage = _allStages[path];
  pxr::UsdPrim ref = stage->DefinePrim(path);
  ref.GetReferences().AddReference(filename);
  stage->SetDefaultPrim(ref);

  _rootStage->GetRootLayer()->InsertSubLayerPath(stage->GetRootLayer()->GetIdentifier());
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

Mesh* Scene::AddMesh(pxr::SdfPath& path, const pxr::GfMatrix4d& xfo)
{
  if (!_currentStage->GetPrimAtPath(path).IsDefined()) {
    pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh::Define(_currentStage, path);
    usdMesh.CreatePointsAttr();
    usdMesh.CreateNormalsAttr();
    usdMesh.CreateFaceVertexIndicesAttr(pxr::VtValue());
    usdMesh.CreateFaceVertexCountsAttr(pxr::VtValue());

    usdMesh.CreateDisplayColorAttr();
    pxr::UsdGeomPrimvar displayColorPrimvar = usdMesh.GetDisplayColorPrimvar();
    displayColorPrimvar.SetInterpolation(pxr::UsdGeomTokens->constant);
   
    usdMesh.CreateSubdivisionSchemeAttr();
    _meshes[path] = Mesh();
  } else {
    pxr::UsdGeomMesh usdMesh(_currentStage->GetPrimAtPath(path));
    _meshes[path] = Mesh(usdMesh);
  }
  return &_meshes[path];
}

  Curve* Scene::AddCurve(pxr::SdfPath & path, const pxr::GfMatrix4d & xfo)
  {
    if (!_currentStage->GetPrimAtPath(path).IsDefined()) {
      pxr::UsdGeomBasisCurves usdCurve = pxr::UsdGeomBasisCurves::Define(_currentStage, path);
      _curves[path] = Curve();
    }
    else {
      pxr::UsdGeomBasisCurves usdCurve(_currentStage->GetPrimAtPath(path));
     _curves[path] = Curve(usdCurve);
    }
    return &_curves[path];
  }

  Points* Scene::AddPoints(pxr::SdfPath& path, const pxr::GfMatrix4d& xfo)
  {
    if (!_currentStage->GetPrimAtPath(path).IsDefined()) {
      pxr::UsdGeomPoints usdPoints = pxr::UsdGeomPoints::Define(_currentStage, path);
      _points[path] = Points();
    }
    else {
      pxr::UsdGeomPoints usdPoints(_currentStage->GetPrimAtPath(path));
      _points[path] = Points(usdPoints);
    }
    return &_points[path];
  }


void Scene::TestVoronoi()
{
  pxr::SdfPath path("/Voronoi");
  pxr::UsdStageRefPtr stage = AddStageFromMemory("Voronoi");
  _allStages[path] = stage;
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

PXR_NAMESPACE_CLOSE_SCOPE