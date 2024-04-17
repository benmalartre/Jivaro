#include <pxr/base/vt/array.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/plane.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformOp.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/imaging/hd/changeTracker.h>

#include "../tests/utils.h"
#include "../geometry/mesh.h"
#include "../geometry/points.h"
#include "../geometry/implicit.h"
#include "../pbd/solver.h"

JVR_NAMESPACE_OPEN_SCOPE

Plane* _GenerateCollidePlane(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path)
{
  const double width = 100;
  const double length = 100;
  pxr::UsdGeomPlane usdGround =
    pxr::UsdGeomPlane::Define(stage, path);

  usdGround.CreateWidthAttr().Set(width);
  usdGround.CreateLengthAttr().Set(length);
  usdGround.CreateAxisAttr().Set(pxr::UsdGeomTokens->y);

  pxr::UsdPrim usdPrim = usdGround.GetPrim();
  usdPrim.CreateAttribute(pxr::TfToken("Restitution"), pxr::SdfValueTypeNames->Float).Set(0.5f);
  usdPrim.CreateAttribute(pxr::TfToken("Friction"), pxr::SdfValueTypeNames->Float).Set(0.5f);
  usdPrim.CreateAttribute(pxr::TfToken("Serial"), pxr::SdfValueTypeNames->Bool).Set(false);

  Plane* ground = new Plane();
  ground->SetMatrix(
    pxr::GfMatrix4d().SetTranslate(pxr::GfVec3f(0.f, -0.5f, 0.f)));

  return ground;
}

Solver* _GenerateSolver(Scene* scene, pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path)
{
  pxr::UsdGeomXform usdXform = pxr::UsdGeomXform::Define(stage, path);

  pxr::UsdPrim usdPrim = usdXform.GetPrim();
  usdPrim.CreateAttribute(pxr::TfToken("SubSteps"), pxr::SdfValueTypeNames->Int).Set(20);
  usdPrim.CreateAttribute(pxr::TfToken("SleepThreshold"), pxr::SdfValueTypeNames->Float).Set(0.01f);

  return new Solver(scene, usdXform, pxr::GfMatrix4d(1.f));
}

Points* _GeneratePoints(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path)
{
  Points* points = new Points();

  pxr::UsdGeomPoints usdPoints = pxr::UsdGeomPoints::Define(stage, path);

  pxr::VtArray<pxr::GfVec3f> position;
  pxr::VtArray<float> width;
  pxr::VtArray<pxr::GfVec3f> color;

  for(size_t x = 0; x < 10; ++x)
    for(size_t y = 0; y < 10; ++y) {
    position.push_back(pxr::GfVec3f(x-5, 7.f, y-5));
    width.push_back(0.005f);
    color.push_back(pxr::GfVec3f(RANDOM_0_1, 1.f, 0.75f));
  }

  usdPoints.CreatePointsAttr().Set(pxr::VtValue(position));
  usdPoints.CreateWidthsAttr().Set(pxr::VtValue(width));
  //points.CreateNormalsAttr().Set(pxr::VtValue({pxr::GfVec3f(0, 1, 0)}));
  usdPoints.CreateDisplayColorAttr(pxr::VtValue(color));
  usdPoints.GetDisplayColorPrimvar().SetInterpolation(pxr::UsdGeomTokens->varying);



  return points;
}

Mesh* _GenerateClothMesh(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  float size, const pxr::GfMatrix4d& m)
{
  Mesh* mesh = new Mesh(m);
  mesh->TriangularGrid2D(10.f, 10.f, pxr::GfMatrix4f(m), size);
  //mesh.Randomize(0.1f);
  pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh::Define(stage, path);

  usdMesh.CreatePointsAttr().Set(mesh->GetPositions());
  usdMesh.CreateFaceVertexCountsAttr().Set(mesh->GetFaceCounts());
  usdMesh.CreateFaceVertexIndicesAttr().Set(mesh->GetFaceConnects());

  pxr::UsdPrim usdPrim = usdMesh.GetPrim();
  usdPrim.CreateAttribute(pxr::TfToken("StretchStiffness"), pxr::SdfValueTypeNames->Float).Set(RANDOM_0_1);
  usdPrim.CreateAttribute(pxr::TfToken("BendStiffness"), pxr::SdfValueTypeNames->Float).Set(RANDOM_0_1);
  usdPrim.CreateAttribute(pxr::TfToken("Restitution"), pxr::SdfValueTypeNames->Float).Set(RANDOM_0_1);
  usdPrim.CreateAttribute(pxr::TfToken("Friction"), pxr::SdfValueTypeNames->Float).Set(RANDOM_0_1);
  usdPrim.CreateAttribute(pxr::TfToken("Serial"), pxr::SdfValueTypeNames->Bool).Set(false);

  return mesh;
}

Sphere* _GenerateCollideSphere(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  double radius, const pxr::GfMatrix4d& m)
{
  Sphere* sphere = new Sphere(m);
  pxr::UsdGeomSphere usdSphere = pxr::UsdGeomSphere::Define(stage, path);

  usdSphere.CreateRadiusAttr().Set(radius);

  double real;
  usdSphere.GetRadiusAttr().Get(&real);

  pxr::UsdGeomXformOp op = usdSphere.MakeMatrixXform();
  op.Set(pxr::GfMatrix4d(m));

  return sphere;
}
JVR_NAMESPACE_CLOSE_SCOPE