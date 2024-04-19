#include <pxr/base/vt/array.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/plane.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformOp.h>
#include <pxr/usd/usdGeom/xformCommonApi.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/pointInstancer.h>
#include <pxr/usd/usdGeom/primvarsApi.h>
#include <pxr/imaging/hd/changeTracker.h>

#include "../tests/utils.h"
#include "../geometry/mesh.h"
#include "../geometry/points.h"
#include "../geometry/implicit.h"
#include "../acceleration/bvh.h"
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

  Plane* ground = new Plane();
  ground->SetMatrix(
    pxr::GfMatrix4d().SetTranslate(pxr::GfVec3f(0.f, -0.5f, 0.f)));

  return ground;
}

Solver* _GenerateSolver(Scene* scene, pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path)
{
  pxr::UsdGeomXform usdXform = pxr::UsdGeomXform::Define(stage, path);

  pxr::UsdPrim usdPrim = usdXform.GetPrim();
  usdPrim.CreateAttribute(pxr::TfToken("SubSteps"), pxr::SdfValueTypeNames->Int).Set(5);
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
  float spacing, const pxr::GfMatrix4d& m)
{
  Mesh* mesh = new Mesh(m);
  mesh->TriangularGrid2D(spacing);
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

Mesh* _GenerateMeshGrid(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  size_t subd, const pxr::GfMatrix4d& m)
{
  Mesh* mesh = new Mesh(m);
  mesh->RegularGrid2D(1.f/subd);
  //mesh.Randomize(0.1f);
  pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh::Define(stage, path);

  usdMesh.CreatePointsAttr().Set(mesh->GetPositions());
  usdMesh.CreateFaceVertexCountsAttr().Set(mesh->GetFaceCounts());
  usdMesh.CreateFaceVertexIndicesAttr().Set(mesh->GetFaceConnects());

  pxr::UsdGeomXformOp op = usdMesh.AddTransformOp();
  op.Set(m, pxr::UsdTimeCode::Default());

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

  pxr::UsdPrim usdPrim = usdSphere.GetPrim();
  usdPrim.CreateAttribute(pxr::TfToken("Restitution"), pxr::SdfValueTypeNames->Float).Set(0.5f);
  usdPrim.CreateAttribute(pxr::TfToken("Friction"), pxr::SdfValueTypeNames->Float).Set(0.5f);


  pxr::UsdGeomXformOp op = usdSphere.MakeMatrixXform();
  op.Set(pxr::GfMatrix4d(m));

  return sphere;
}

void _SetupBVHInstancer(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path, BVH* bvh)
{
  std::vector<BVH::Cell*> cells;
  bvh->GetRoot()->GetLeaves(cells);
  size_t numPoints = cells.size();
  pxr::VtArray<pxr::GfVec3f> points(numPoints);
  pxr::VtArray<pxr::GfVec3f> scales(numPoints);
  pxr::VtArray<int64_t> indices(numPoints);
  pxr::VtArray<int> protoIndices(numPoints);
  pxr::VtArray<pxr::GfQuath> rotations(numPoints);
  pxr::VtArray<pxr::GfVec3f> colors(numPoints);
  pxr::VtArray<int> curveVertexCount(1);
  curveVertexCount[0] = numPoints;
  pxr::VtArray<float> widths(numPoints);
  widths[0] = 1.f;

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    points[pointIdx] = pxr::GfVec3f(cells[pointIdx]->GetMidpoint());
    scales[pointIdx] = pxr::GfVec3f(cells[pointIdx]->GetSize());
    protoIndices[pointIdx] = 0;
    indices[pointIdx] = pointIdx;
    colors[pointIdx] =
      pxr::GfVec3f(bvh->ComputeCodeAsColor(bvh->GetRoot(),
        pxr::GfVec3f(cells[pointIdx]->GetMidpoint())));
    rotations[pointIdx] = pxr::GfQuath::GetIdentity();
    widths[pointIdx] = RANDOM_0_1;
  }

  pxr::UsdGeomPointInstancer instancer =
    pxr::UsdGeomPointInstancer::Define(stage,path);

  pxr::UsdGeomCube proto =
    pxr::UsdGeomCube::Define(stage,
      path.AppendChild(pxr::TfToken("proto_cube")));
  proto.CreateSizeAttr().Set(1.0);

  instancer.CreatePositionsAttr().Set(points);
  instancer.CreateProtoIndicesAttr().Set(protoIndices);
  instancer.CreateScalesAttr().Set(scales);
  instancer.CreateIdsAttr().Set(indices);
  instancer.CreateOrientationsAttr().Set(rotations);
  instancer.CreatePrototypesRel().AddTarget(proto.GetPath());
  pxr::UsdGeomPrimvarsAPI primvarsApi(instancer);
  pxr::UsdGeomPrimvar colorPrimvar =
    primvarsApi.CreatePrimvar(pxr::UsdGeomTokens->primvarsDisplayColor, pxr::SdfValueTypeNames->Color3fArray);
  colorPrimvar.SetInterpolation(pxr::UsdGeomTokens->varying);
  colorPrimvar.SetElementSize(1);
  colorPrimvar.Set(colors);
}

JVR_NAMESPACE_CLOSE_SCOPE