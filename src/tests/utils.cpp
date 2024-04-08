#include <pxr/base/vt/array.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformOp.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/imaging/hd/changeTracker.h>

#include "../tests/utils.h"
#include "../geometry/mesh.h"


JVR_NAMESPACE_OPEN_SCOPE

pxr::UsdPrim _GenerateCollideGround(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path)
{

  pxr::UsdGeomCube ground =
    pxr::UsdGeomCube::Define(stage, path.AppendChild(pxr::TfToken("Ground")));

  ground.CreateSizeAttr().Set(1.0);

  ground.AddScaleOp(pxr::UsdGeomXformOp::PrecisionFloat).Set(pxr::GfVec3f(100.f, 1.f, 100.f));
  ground.AddTranslateOp(pxr::UsdGeomXformOp::PrecisionFloat).Set(pxr::GfVec3f(0.f, -0.5f, 0.f));


  pxr::UsdPrim usdPrim = ground.GetPrim();
  usdPrim.CreateAttribute(pxr::TfToken("Restitution"), pxr::SdfValueTypeNames->Float).Set(RANDOM_0_1);
  usdPrim.CreateAttribute(pxr::TfToken("Friction"), pxr::SdfValueTypeNames->Float).Set(RANDOM_0_1);

  return ground.GetPrim();
}

pxr::UsdPrim _GenerateSolver(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, const pxr::TfToken& name)
{

  pxr::UsdGeomXform usdXform = pxr::UsdGeomXform::Define(stage, path.AppendChild(name));


  pxr::UsdPrim usdPrim = usdXform.GetPrim();
  usdPrim.CreateAttribute(pxr::TfToken("SubSteps"), pxr::SdfValueTypeNames->Int).Set(20);
  usdPrim.CreateAttribute(pxr::TfToken("SleepThreshold"), pxr::SdfValueTypeNames->Float).Set(0.01f);
  usdPrim.CreateAttribute(pxr::TfToken("Gravity"), pxr::SdfValueTypeNames->Vector3f).Set(pxr::GfVec3f(0.f, -9.8f, 0.f));

  return usdPrim;
}

pxr::UsdPrim _GenerateClothMesh(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  float size, const pxr::GfMatrix4f& m)
{
  Mesh mesh;
  mesh.TriangularGrid2D(10.f, 10.f, m, size);
  //mesh.Randomize(0.1f);
  pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh::Define(stage, path);

  usdMesh.CreatePointsAttr().Set(mesh.GetPositions());
  usdMesh.CreateFaceVertexCountsAttr().Set(mesh.GetFaceCounts());
  usdMesh.CreateFaceVertexIndicesAttr().Set(mesh.GetFaceConnects());

  pxr::UsdPrim usdPrim = usdMesh.GetPrim();
  usdPrim.CreateAttribute(pxr::TfToken("StretchStiffness"), pxr::SdfValueTypeNames->Float).Set(RANDOM_0_1);
  usdPrim.CreateAttribute(pxr::TfToken("BendStiffness"), pxr::SdfValueTypeNames->Float).Set(RANDOM_0_1);
  usdPrim.CreateAttribute(pxr::TfToken("Restitution"), pxr::SdfValueTypeNames->Float).Set(RANDOM_0_1);
  usdPrim.CreateAttribute(pxr::TfToken("Friction"), pxr::SdfValueTypeNames->Float).Set(RANDOM_0_1);
  usdPrim.CreateAttribute(pxr::TfToken("Serial"), pxr::SdfValueTypeNames->Bool).Set(false);

  return usdMesh.GetPrim();
}

pxr::UsdPrim _GenerateCollideSphere(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  double radius, const pxr::GfMatrix4f& m)
{
  pxr::UsdGeomSphere usdSphere = pxr::UsdGeomSphere::Define(stage, path);

  usdSphere.CreateRadiusAttr().Set(radius);

  double real;
  usdSphere.GetRadiusAttr().Get(&real);

  pxr::UsdGeomXformOp op = usdSphere.MakeMatrixXform();
  op.Set(pxr::GfMatrix4d(m));

  return usdSphere.GetPrim();
}
JVR_NAMESPACE_CLOSE_SCOPE