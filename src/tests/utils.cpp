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
#include "../geometry/implicit.h"
#include "../geometry/mesh.h"
#include "../geometry/points.h"
#include "../geometry/implicit.h"
#include "../geometry/instancer.h"
#include "../acceleration/bvh.h"
#include "../acceleration/grid3d.h"
#include "../pbd/tokens.h"
#include "../pbd/solver.h"
#include "../pbd/force.h"

JVR_NAMESPACE_OPEN_SCOPE

Plane* _GenerateCollidePlane(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path,
  float friction, float restitution)
{
  const double width = 100;
  const double length = 100;
  pxr::UsdGeomPlane usdGround = pxr::UsdGeomPlane::Define(stage, path);

  usdGround.CreateWidthAttr().Set(width);
  usdGround.CreateLengthAttr().Set(length);
  usdGround.CreateAxisAttr().Set(pxr::UsdGeomTokens->y);

  pxr::UsdPrim usdPrim = usdGround.GetPrim();
  usdPrim.CreateAttribute(PBDTokens->restitution, pxr::SdfValueTypeNames->Float).Set(restitution);
  usdPrim.CreateAttribute(PBDTokens->friction, pxr::SdfValueTypeNames->Float).Set(friction);

  return new Plane(usdGround, pxr::GfMatrix4d(1.0));
}

Solver* _GenerateSolver(Scene* scene, pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path,
  int subSteps, float sleepThreshold)
{
  pxr::UsdGeomXform usdXform = pxr::UsdGeomXform::Define(stage, path);

  pxr::UsdPrim usdPrim = usdXform.GetPrim();
  usdPrim.CreateAttribute(PBDTokens->substeps, pxr::SdfValueTypeNames->Int).Set(subSteps);
  usdPrim.CreateAttribute(PBDTokens->sleepThreshold, pxr::SdfValueTypeNames->Float).Set(sleepThreshold);

  usdPrim.CreateAttribute(PBDTokens->gravity, pxr::SdfValueTypeNames->Float3).Set(pxr::GfVec3f(0.f, -9.81f, 0.f));
  pxr::UsdAttribute gravityAttr = usdPrim.GetAttribute(PBDTokens->gravity);
    
  usdPrim.CreateAttribute(PBDTokens->damp, pxr::SdfValueTypeNames->Float).Set(0.1f);
  pxr::UsdAttribute dampAttr = usdPrim.GetAttribute(PBDTokens->damp);

  Solver* solver = new Solver(scene, usdXform, pxr::GfMatrix4d(1.f));

  Force* gravity = new GravityForce(gravityAttr);
  solver->AddElement(gravity, NULL, path.AppendChild(PBDTokens->gravity));

  Force* damp = new DampForce(dampAttr);
  solver->AddElement(damp, NULL, path.AppendChild(PBDTokens->damp));

  return solver;
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
  double radius, const pxr::GfMatrix4d& m, float friction, float restitution)
{
  
  pxr::UsdGeomSphere usdSphere = pxr::UsdGeomSphere::Define(stage, path);

  usdSphere.CreateRadiusAttr().Set(radius);

  double real;
  usdSphere.GetRadiusAttr().Get(&real);

  pxr::UsdPrim usdPrim = usdSphere.GetPrim();
  usdPrim.CreateAttribute(PBDTokens->restitution, pxr::SdfValueTypeNames->Float).Set(restitution);
  usdPrim.CreateAttribute(PBDTokens->friction, pxr::SdfValueTypeNames->Float).Set(friction);


  pxr::UsdGeomXformOp op = usdSphere.MakeMatrixXform();
  op.Set(pxr::GfMatrix4d(m));

  Sphere* sphere = new Sphere(usdSphere, m);
  return sphere;
}



Instancer* _SetupBVHInstancer(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path, BVH* bvh)
{
  pxr::VtArray<pxr::GfVec3f> points;
  pxr::VtArray<pxr::GfVec3f> scales;
  pxr::VtArray<pxr::GfVec3f> colors;

  bvh->GetCells(points, scales, colors);
  size_t numPoints = points.size();

  pxr::VtArray<int64_t> indices(numPoints);
  pxr::VtArray<int> protoIndices(numPoints);
  pxr::VtArray<pxr::GfQuath> rotations(numPoints);

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    protoIndices[pointIdx] = 0;
    indices[pointIdx] = pointIdx;
    rotations[pointIdx] = pxr::GfQuath::GetIdentity();
  }

  pxr::UsdGeomPointInstancer instancer =
    pxr::UsdGeomPointInstancer::Define(stage, path);

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

  return new Instancer(instancer.GetPrim(), pxr::GfMatrix4d());
}

void _UpdateBVHInstancer(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path, BVH* bvh, float time)
{
  std::vector<BVH::Cell*> cells;
  bvh->GetRoot()->GetCells(cells);
  size_t numPoints = cells.size();

  pxr::VtArray<pxr::GfVec3f> points(numPoints);
  pxr::VtArray<pxr::GfVec3f> scales(numPoints);
  pxr::VtArray<pxr::GfQuath> rotations(numPoints);

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    points[pointIdx] = pxr::GfVec3f(cells[pointIdx]->GetMidpoint());
    scales[pointIdx] = pxr::GfVec3f(cells[pointIdx]->GetSize());
    rotations[pointIdx] = pxr::GfQuath::GetIdentity();
  }

  pxr::UsdPrim prim = stage->GetPrimAtPath(path);
  pxr::UsdGeomPointInstancer instancer(prim);

  instancer.GetPositionsAttr().Set(points);
  instancer.GetScalesAttr().Set(scales);
  instancer.GetOrientationsAttr().Set(rotations);

}


Instancer* _SetupGridInstancer(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path, Grid3D* grid)
{
  pxr::VtArray<pxr::GfVec3f> points;
  pxr::VtArray<pxr::GfVec3f> scales;
  pxr::VtArray<pxr::GfVec3f> colors;

  grid->GetCells(points, scales, colors);
  size_t numPoints = points.size();

  pxr::VtArray<int64_t> indices(numPoints);
  pxr::VtArray<int> protoIndices(numPoints);
  pxr::VtArray<pxr::GfQuath> rotations(numPoints);

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    protoIndices[pointIdx] = 0;
    indices[pointIdx] = pointIdx;
    rotations[pointIdx] = pxr::GfQuath::GetIdentity();
  }

  pxr::UsdGeomPointInstancer instancer =
    pxr::UsdGeomPointInstancer::Define(stage, path);

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

  return new Instancer(instancer.GetPrim(), pxr::GfMatrix4d());
}

void _UpdateGridInstancer(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path, Grid3D* grid, float time)
{
  /*
  std::vector<BVH::Cell*> cells;
  bvh->GetRoot()->GetCells(cells);
  size_t numPoints = cells.size();

  pxr::VtArray<pxr::GfVec3f> points(numPoints);
  pxr::VtArray<pxr::GfVec3f> scales(numPoints);
  pxr::VtArray<pxr::GfQuath> rotations(numPoints);

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    points[pointIdx] = pxr::GfVec3f(cells[pointIdx]->GetMidpoint());
    scales[pointIdx] = pxr::GfVec3f(cells[pointIdx]->GetSize());
    rotations[pointIdx] = pxr::GfQuath::GetIdentity();
  }

  pxr::UsdPrim prim = stage->GetPrimAtPath(path);
  pxr::UsdGeomPointInstancer instancer(prim);

  instancer.GetPositionsAttr().Set(points);
  instancer.GetScalesAttr().Set(scales);
  instancer.GetOrientationsAttr().Set(rotations);
  */
}



JVR_NAMESPACE_CLOSE_SCOPE