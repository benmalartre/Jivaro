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

#include <usdPbd/collisionAPI.h>
#include <usdPbd/constraintAPI.h>
#include <usdPbd/bodyAPI.h>
#include <usdPbd/solver.h>

#include "../tests/utils.h"
#include "../geometry/implicit.h"
#include "../geometry/mesh.h"
#include "../geometry/points.h"
#include "../geometry/implicit.h"
#include "../geometry/instancer.h"
#include "../acceleration/bvh.h"
#include "../acceleration/grid3d.h"
#include "../pbd/solver.h"
#include "../pbd/force.h"

JVR_NAMESPACE_OPEN_SCOPE

Plane* _CreateCollidePlane(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path,
  float friction, float restitution)
{
  const double width = 100;
  const double length = 100;
  pxr::UsdGeomPlane usdGround = pxr::UsdGeomPlane::Define(stage, path);

  usdGround.CreateWidthAttr().Set(width);
  usdGround.CreateLengthAttr().Set(length);
  usdGround.CreateAxisAttr().Set(pxr::UsdGeomTokens->y);

  pxr::UsdPbdCollisionAPI api = pxr::UsdPbdCollisionAPI::Apply(usdGround.GetPrim());
  api.GetRestitutionAttr().Set(restitution);
  api.GetFrictionAttr().Set(friction);

  return new Plane(usdGround, pxr::GfMatrix4d(1.0));
}

Solver* _CreateSolver(Scene* scene, pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path,
  int subSteps, float sleepThreshold)
{
  pxr::UsdPrim prim = stage->GetPrimAtPath(path);
  if(prim.IsValid())
    return new Solver(scene, pxr::UsdPbdSolver(prim), pxr::GfMatrix4d(1.0));
  
  return new Solver(scene, pxr::UsdPbdSolver::Define(stage, path), pxr::GfMatrix4d(1.0));
}


Mesh* _CreateClothMesh(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  float spacing, const pxr::GfMatrix4d& m, float mass, float damp)
{
  Mesh* mesh = new Mesh(m);
  mesh->TriangularGrid2D(spacing);
  //mesh->RegularGrid2D(spacing);
  //mesh.Randomize(0.1f);
  
  pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh::Define(stage, path);
  usdMesh.GetPointsAttr().Set(mesh->GetPositions(), pxr::UsdTimeCode::Default());
  usdMesh.GetFaceVertexCountsAttr().Set(mesh->GetFaceCounts(), pxr::UsdTimeCode::Default());
  usdMesh.GetFaceVertexIndicesAttr().Set(mesh->GetFaceConnects(), pxr::UsdTimeCode::Default());

  usdMesh.MakeMatrixXform().Set(m);

  pxr::UsdPrim prim = usdMesh.GetPrim();
  mesh->SetPrim(prim);
  mesh->SetInputOutput();
  pxr::UsdPbdBodyAPI api = pxr::UsdPbdBodyAPI::Apply(prim);

  api.GetMassAttr().Set(mass);
  api.GetDampAttr().Set(damp);
  api.GetRadiusAttr().Set(spacing * 0.95f);

  pxr::UsdPbdConstraintAPI::Apply(prim, pxr::TfToken("attach"));
  pxr::UsdPbdConstraintAPI::Apply(prim, pxr::TfToken("stretch"));
  pxr::UsdPbdConstraintAPI::Apply(prim, pxr::TfToken("shear"));
  pxr::UsdPbdConstraintAPI::Apply(prim, pxr::TfToken("bend"));
  
  //api.GetGravityAttr().Set(gravity);

  /*/
  usdPrim.CreateAttribute(PBDTokens->mass, pxr::SdfValueTypeNames->Float).Set(mass);
  usdPrim.CreateAttribute(PBDTokens->damp, pxr::SdfValueTypeNames->Float).Set(damp);
  usdPrim.CreateAttribute(PBDTokens->velocity, pxr::SdfValueTypeNames->Float3).Set(pxr::GfVec3f(0.f));
  
  pxr::TfToken stretchStiffness(PBDTokens->stretch.GetString()+":"+PBDTokens->stiffness.GetString());
  usdPrim.CreateAttribute(stretchStiffness, pxr::SdfValueTypeNames->Float).Set(100000.f);
  pxr::TfToken stretchDamp(PBDTokens->stretch.GetString()+":"+PBDTokens->damp.GetString());
  usdPrim.CreateAttribute(stretchDamp, pxr::SdfValueTypeNames->Float).Set(0.1f);

  pxr::TfToken shearStiffness(PBDTokens->shear.GetString()+":"+PBDTokens->stiffness.GetString());
  usdPrim.CreateAttribute(shearStiffness, pxr::SdfValueTypeNames->Float).Set(100000.f);
  pxr::TfToken shearDamp(PBDTokens->shear.GetString()+":"+PBDTokens->damp.GetString());
  usdPrim.CreateAttribute(shearDamp, pxr::SdfValueTypeNames->Float).Set(0.1f);

  pxr::TfToken bendStiffness(PBDTokens->bend.GetString() + ":" + PBDTokens->stiffness.GetString());
  usdPrim.CreateAttribute(bendStiffness, pxr::SdfValueTypeNames->Float).Set(20000.f);
  pxr::TfToken bendDamp(PBDTokens->bend.GetString() + ":" + PBDTokens->damp.GetString());
  usdPrim.CreateAttribute(bendDamp, pxr::SdfValueTypeNames->Float).Set(0.1f);
  */
 



  return mesh;

}

Mesh* _CreateMeshGrid(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  size_t subd, const pxr::GfMatrix4d& m)
{
  Mesh* mesh = new Mesh(m);
  mesh->RegularGrid2D(1.f/subd);
  //mesh.Randomize(0.1f);
  pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh::Define(stage, path);

  usdMesh.GetPointsAttr().Set(mesh->GetPositions());
  usdMesh.GetFaceVertexCountsAttr().Set(mesh->GetFaceCounts());
  usdMesh.GetFaceVertexIndicesAttr().Set(mesh->GetFaceConnects());

/*
  pxr::VtVec3fArray extentArray(2);
  extentArray[0] = pxr::GfVec3f(-0.5f,0.f,-0.5f);
  extentArray[1] = pxr::GfVec3f(0.5f,0.f,0.5f);
  usdMesh.GetExtentAttr().Set(extentArray);
*/
  pxr::UsdGeomXformOp op = usdMesh.AddTransformOp();
  op.Set(m, pxr::UsdTimeCode::Default());

  mesh->SetPrim(usdMesh.GetPrim());
  mesh->SetInputOutput();

  return mesh;
}

Sphere* _CreateCollideSphere(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  double radius, const pxr::GfMatrix4d& m, float friction, float restitution)
{
  
  pxr::UsdGeomSphere usdSphere = pxr::UsdGeomSphere::Define(stage, path);

  usdSphere.CreateRadiusAttr().Set(radius);

  double real;
  usdSphere.GetRadiusAttr().Get(&real);

  pxr::UsdPbdCollisionAPI api = pxr::UsdPbdCollisionAPI::Apply(usdSphere.GetPrim());

  api.GetRestitutionAttr().Set(restitution);
  api.GetFrictionAttr().Set(friction);

  pxr::UsdGeomXformOp op = usdSphere.MakeMatrixXform();
  op.Set(pxr::GfMatrix4d(m));

  Sphere* sphere = new Sphere(usdSphere, m);
  return sphere;
}


Instancer* _SetupBVHInstancer(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path, BVH* bvh, bool branchOrLeaf)
{
  pxr::VtArray<pxr::GfVec3f> points;
  pxr::VtArray<pxr::GfVec3f> scales;
  pxr::VtArray<pxr::GfVec3f> colors;

  bvh->GetCells(points, scales, colors, branchOrLeaf);
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

  return new Instancer(instancer.GetPrim(), pxr::GfMatrix4d(1.0));
}

void _UpdateBVHInstancer(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path, BVH* bvh, float time)
{
  std::vector<const BVH::Cell*> cells;
  bvh->GetLeaves(bvh->GetRoot(), cells);
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


Instancer* _SetupPointsInstancer(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path, Points* points)
{
  size_t numPoints = points->GetNumPoints();

  pxr::VtArray<pxr::GfVec3f> positions(numPoints);
  pxr::VtArray<pxr::GfVec3f> scales(numPoints);
  pxr::VtArray<pxr::GfVec3f> colors(numPoints);

  pxr::VtArray<int64_t> indices(numPoints);
  pxr::VtArray<int> protoIndices(numPoints);
  pxr::VtArray<pxr::GfQuath> rotations(numPoints);

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    positions[pointIdx] = points->GetPosition(pointIdx);
    scales[pointIdx] = pxr::GfVec3f(points->GetWidth(pointIdx));
    colors[pointIdx] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
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

  instancer.CreatePositionsAttr().Set(positions);
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

  return new Instancer(instancer.GetPrim(), pxr::GfMatrix4d(1.0));
}

void _UpdatePointsInstancer(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path, Points* points, float time)
{

  size_t numPoints = points->GetNumPoints();

  pxr::VtArray<pxr::GfVec3f> positions(numPoints);
  pxr::VtArray<pxr::GfVec3f> scales(numPoints);
  pxr::VtArray<pxr::GfQuath> rotations(numPoints);

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    positions[pointIdx] = points->GetPosition(pointIdx);
    scales[pointIdx] = pxr::GfVec3f(1.f);
    rotations[pointIdx] = pxr::GfQuath::GetIdentity();
  }

  pxr::UsdPrim prim = stage->GetPrimAtPath(path);
  pxr::UsdGeomPointInstancer instancer(prim);

  instancer.GetPositionsAttr().Set(positions);
  instancer.GetScalesAttr().Set(scales);
  instancer.GetOrientationsAttr().Set(rotations);

}

Instancer* _SetupGridInstancer(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path, Grid3D* grid)
{
  pxr::VtArray<pxr::GfVec3f> points;
  pxr::VtArray<pxr::GfVec3f> scales;
  pxr::VtArray<pxr::GfVec3f> colors;

  grid->GetCells(points, scales, colors, false);
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

  return new Instancer(instancer.GetPrim(), pxr::GfMatrix4d(1.0));
}

void _UpdateGridInstancer(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path, Grid3D* grid, float time)
{
  pxr::VtArray<pxr::GfVec3f> points;
  pxr::VtArray<pxr::GfVec3f> scales;
  pxr::VtArray<pxr::GfVec3f> colors;

  grid->GetCells(points, scales, colors, false);

  pxr::UsdPrim prim = stage->GetPrimAtPath(path);
  pxr::UsdGeomPointInstancer instancer(prim);

  instancer.GetPositionsAttr().Set(points);
  instancer.GetScalesAttr().Set(scales);

}



JVR_NAMESPACE_CLOSE_SCOPE