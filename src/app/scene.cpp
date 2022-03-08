
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/points.h>


#include "../utils/strings.h"
#include "../utils/files.h"
#include "../app/scene.h"
#include "../command/router.h"
#include "../command/block.h"

PXR_NAMESPACE_OPEN_SCOPE

Scene::Scene()
{
  _stage = pxr::UsdStage::CreateInMemory((char*)this);
  UndoRouter::Get().TrackLayer(_stage->GetRootLayer());
}

Scene::~Scene()
{
}

void 
Scene::Init(const pxr::UsdStageRefPtr& stage)
{

}

void
Scene::Save(const std::string& filename)
{

}

void 
Scene::Export(const std::string& filename)
{

}



Mesh* Scene::AddMesh(pxr::SdfPath& path, const pxr::GfMatrix4d& xfo)
{
  if (!_stage->GetPrimAtPath(path).IsDefined()) {
    pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh::Define(_stage, path);
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
    pxr::UsdGeomMesh usdMesh(_stage->GetPrimAtPath(path));
    _meshes[path] = Mesh(usdMesh);
  }
  return &_meshes[path];
}

  Curve* Scene::AddCurve(pxr::SdfPath & path, const pxr::GfMatrix4d & xfo)
  {
    if (!_stage->GetPrimAtPath(path).IsDefined()) {
      pxr::UsdGeomBasisCurves usdCurve = pxr::UsdGeomBasisCurves::Define(_stage, path);
      _curves[path] = Curve();
    }
    else {
      pxr::UsdGeomBasisCurves usdCurve(_stage->GetPrimAtPath(path));
     _curves[path] = Curve(usdCurve);
    }
    return &_curves[path];
  }

  Points* Scene::AddPoints(pxr::SdfPath& path, const pxr::GfMatrix4d& xfo)
  {
    if (!_stage->GetPrimAtPath(path).IsDefined()) {
      pxr::UsdGeomPoints usdPoints = pxr::UsdGeomPoints::Define(_stage, path);
      _points[path] = Points();
    }
    else {
      pxr::UsdGeomPoints usdPoints(_stage->GetPrimAtPath(path));
      _points[path] = Points(usdPoints);
    }
    return &_points[path];
  }


void Scene::TestVoronoi()
{
  pxr::SdfPath path("/Voronoi");
  Mesh mesh;
  std::vector<pxr::GfVec3f> points(1024);
  for (auto& point : points) {
    point[0] = (float)rand() / (float)RAND_MAX - 0.5f;
    point[1] = 0.f;
    point[2] = (float)rand() / (float)RAND_MAX - 0.5f;
  }
  mesh.VoronoiDiagram(points);
  pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh::Define(_stage, path);

  pxr::VtArray<pxr::GfVec3f> pivotPositions = mesh.GetPositions();
  for (auto& pos : pivotPositions) pos += pxr::GfVec3f(1.f, 0.f, 0.f);
  usdMesh.CreatePointsAttr(pxr::VtValue(mesh.GetPositions()));
  //usdMesh.CreateNormalsAttr(pxr::VtValue(mesh.GetNormals()));
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
  /*
  pxr::UsdGeomXformCommonAPI xformApi(usdMesh.GetPrim());
  xformApi.SetPivot(pxr::GfVec3f(1.f, 0.f, 0.f), pxr::UsdTimeCode::Default());
  */
  //stage->Export("C:/Users/graph/Documents/bmal/src/USD_ASSETS/tests/pivot.usda");

  //stage->SetDefaultPrim(usdMesh.GetPrim());
  //_rootStage->GetRootLayer()->InsertSubLayerPath(stage->GetRootLayer()->GetIdentifier());
}

PXR_NAMESPACE_CLOSE_SCOPE