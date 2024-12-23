#include <algorithm>
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

Plane* _CreateCollidePlane(UsdStageRefPtr& stage, const SdfPath& path,
  float friction, float restitution)
{
  const double width = 100;
  const double length = 100;
  UsdGeomPlane usdGround = UsdGeomPlane::Define(stage, path);

  usdGround.CreateWidthAttr().Set(width);
  usdGround.CreateLengthAttr().Set(length);
  usdGround.CreateAxisAttr().Set(UsdGeomTokens->y);

  UsdPbdCollisionAPI api = UsdPbdCollisionAPI::Apply(usdGround.GetPrim());
  api.GetRestitutionAttr().Set(restitution);
  api.GetFrictionAttr().Set(friction);

  return new Plane(usdGround, GfMatrix4d(1.0));
}

Solver* _CreateSolver(Scene* scene, UsdStageRefPtr& stage, const SdfPath& path,
  int subSteps, float sleepThreshold)
{
  UsdPrim prim = stage->GetPrimAtPath(path);
  if(prim.IsValid())
    return new Solver(scene, prim);
  
  prim = UsdPbdSolver::Define(stage, path).GetPrim();
  return new Solver(scene, prim);
}


Mesh* _CreateClothMesh(UsdStageRefPtr& stage, const SdfPath& path, 
  float spacing, const GfMatrix4d& m, float mass, float damp)
{
  Mesh* mesh = new Mesh(m);
  mesh->TriangularGrid2D(spacing);
  //mesh->RegularGrid2D(spacing);
  //mesh.Randomize(0.1f);
  
  UsdGeomMesh usdMesh = UsdGeomMesh::Define(stage, path);
  usdMesh.GetPointsAttr().Set(mesh->GetPositions(), UsdTimeCode::Default());
  usdMesh.GetFaceVertexCountsAttr().Set(mesh->GetFaceCounts(), UsdTimeCode::Default());
  usdMesh.GetFaceVertexIndicesAttr().Set(mesh->GetFaceConnects(), UsdTimeCode::Default());

  usdMesh.MakeMatrixXform().Set(m);

  UsdPrim prim = usdMesh.GetPrim();
  mesh->SetPrim(prim);
  mesh->SetInputOutput();
  UsdPbdBodyAPI api = UsdPbdBodyAPI::Apply(prim);

  api.GetMassAttr().Set(mass);
  api.GetDampAttr().Set(damp);
  api.GetRadiusAttr().Set(spacing * 0.95f);

  UsdPbdConstraintAPI::Apply(prim, TfToken("attach"));
  UsdPbdConstraintAPI::Apply(prim, TfToken("stretch"));
  UsdPbdConstraintAPI::Apply(prim, TfToken("shear"));
  UsdPbdConstraintAPI::Apply(prim, TfToken("dihedral"));

  return mesh;

}

Mesh* _CreateMeshGrid(UsdStageRefPtr& stage, const SdfPath& path, 
  size_t subd, const GfMatrix4d& m)
{
  Mesh* mesh = new Mesh(m);
  mesh->RegularGrid2D(1.f/subd);
  //mesh.Randomize(0.1f);
  UsdGeomMesh usdMesh = UsdGeomMesh::Define(stage, path);

  usdMesh.GetPointsAttr().Set(mesh->GetPositions());
  usdMesh.GetFaceVertexCountsAttr().Set(mesh->GetFaceCounts());
  usdMesh.GetFaceVertexIndicesAttr().Set(mesh->GetFaceConnects());

/*
  VtVec3fArray extentArray(2);
  extentArray[0] = GfVec3f(-0.5f,0.f,-0.5f);
  extentArray[1] = GfVec3f(0.5f,0.f,0.5f);
  usdMesh.GetExtentAttr().Set(extentArray);
*/

  UsdGeomXformOp op = usdMesh.AddTransformOp();
  op.Set(m, UsdTimeCode::Default());

  mesh->SetPrim(usdMesh.GetPrim());
  mesh->SetInputOutput();

  return mesh;
}

Mesh* _CreateTexturedMeshGrid(UsdStageRefPtr& stage, const SdfPath& path, 
  size_t subdivX, size_t subdivY, float sizeX, float sizeY, const std::string& filename,
  const GfMatrix4d& m)
{
  Mesh* mesh = new Mesh(m);
  mesh->RegularGrid2D(subdivX, subdivY, sizeX, sizeY, pxr::GfMatrix4f(m));
  //mesh.Randomize(0.1f);
  UsdGeomMesh usdMesh = UsdGeomMesh::Define(stage, path);

  usdMesh.GetPointsAttr().Set(mesh->GetPositions());
  usdMesh.GetFaceVertexCountsAttr().Set(mesh->GetFaceCounts());
  usdMesh.GetFaceVertexIndicesAttr().Set(mesh->GetFaceConnects());
/*
billboard.CreateExtentAttr([(-430, -145, 0), (430, 145, 0)])
texCoords = UsdGeom.PrimvarsAPI(billboard).CreatePrimvar("st",
                                    Sdf.ValueTypeNames.TexCoord2fArray,
                                    UsdGeom.Tokens.varying)
texCoords.Set([(0, 0), (1, 0), (1,1), (0, 1)])
*/
return nullptr;
}

Sphere* _CreateCollideSphere(UsdStageRefPtr& stage, const SdfPath& path, 
  double radius, const GfMatrix4d& m, float friction, float restitution)
{
  
  UsdGeomSphere usdSphere = UsdGeomSphere::Define(stage, path);

  usdSphere.CreateRadiusAttr().Set(radius);

  double real;
  usdSphere.GetRadiusAttr().Get(&real);

  UsdPbdCollisionAPI api = UsdPbdCollisionAPI::Apply(usdSphere.GetPrim());

  api.GetRestitutionAttr().Set(restitution);
  api.GetFrictionAttr().Set(friction);

  UsdGeomXformOp op = usdSphere.MakeMatrixXform();
  op.Set(GfMatrix4d(m));

  Sphere* sphere = new Sphere(usdSphere, m);
  return sphere;
}


Instancer* _SetupBVHInstancer(UsdStageRefPtr& stage, SdfPath& path, BVH* bvh, bool branchOrLeaf)
{
  VtArray<GfVec3f> points;
  VtArray<GfVec3f> scales;
  VtArray<GfVec3f> colors;

  bvh->GetCells(points, scales, colors, branchOrLeaf);
  size_t numPoints = points.size();

  VtArray<int64_t> indices(numPoints);
  VtArray<int> protoIndices(numPoints);
  VtArray<GfQuath> rotations(numPoints);

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    protoIndices[pointIdx] = 0;
    indices[pointIdx] = pointIdx;
    rotations[pointIdx] = GfQuath::GetIdentity();
  }

  UsdGeomPointInstancer instancer =
    UsdGeomPointInstancer::Define(stage, path);

  UsdGeomCube proto =
    UsdGeomCube::Define(stage,
      path.AppendChild(TfToken("proto_cube")));
  proto.CreateSizeAttr().Set(1.0);

  instancer.CreatePositionsAttr().Set(points);
  instancer.CreateProtoIndicesAttr().Set(protoIndices);
  instancer.CreateScalesAttr().Set(scales);
  instancer.CreateIdsAttr().Set(indices);
  instancer.CreateOrientationsAttr().Set(rotations);
  instancer.CreatePrototypesRel().AddTarget(proto.GetPath());
  UsdGeomPrimvarsAPI primvarsApi(instancer);
  UsdGeomPrimvar colorPrimvar =
    primvarsApi.CreatePrimvar(UsdGeomTokens->primvarsDisplayColor, SdfValueTypeNames->Color3fArray);
  colorPrimvar.SetInterpolation(UsdGeomTokens->varying);
  colorPrimvar.SetElementSize(1);
  colorPrimvar.Set(colors);

  return new Instancer(instancer.GetPrim(), GfMatrix4d(1.0));
}

void _UpdateBVHInstancer(UsdStageRefPtr& stage, SdfPath& path, BVH* bvh, float time)
{
  std::vector<const BVH::Cell*> cells;
  bvh->GetLeaves(bvh->GetRoot(), cells);
  size_t numPoints = cells.size();

  VtArray<GfVec3f> points(numPoints);
  VtArray<GfVec3f> scales(numPoints);
  VtArray<GfQuath> rotations(numPoints);

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    points[pointIdx] = GfVec3f(cells[pointIdx]->GetMidpoint());
    scales[pointIdx] = GfVec3f(cells[pointIdx]->GetSize());
    rotations[pointIdx] = GfQuath::GetIdentity();
  }

  UsdPrim prim = stage->GetPrimAtPath(path);
  UsdGeomPointInstancer instancer(prim);

  instancer.GetPositionsAttr().Set(points);
  instancer.GetScalesAttr().Set(scales);
  instancer.GetOrientationsAttr().Set(rotations);

}


Instancer* _SetupPointsInstancer(UsdStageRefPtr& stage, SdfPath& path, Points* points)
{
  size_t numPoints = points->GetNumPoints();

  VtArray<GfVec3f> positions(numPoints);
  VtArray<GfVec3f> scales(numPoints);
  VtArray<GfVec3f> colors(numPoints);

  VtArray<int64_t> indices(numPoints);
  VtArray<int> protoIndices(numPoints);
  VtArray<GfQuath> rotations(numPoints);

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    positions[pointIdx] = points->GetPosition(pointIdx);
    scales[pointIdx] = GfVec3f(points->GetWidth(pointIdx));
    colors[pointIdx] = GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    protoIndices[pointIdx] = 0;
    indices[pointIdx] = pointIdx;
    rotations[pointIdx] = GfQuath::GetIdentity();
  }

  UsdGeomPointInstancer instancer =
    UsdGeomPointInstancer::Define(stage, path);

  UsdGeomCube proto =
    UsdGeomCube::Define(stage,
      path.AppendChild(TfToken("proto_cube")));
  proto.CreateSizeAttr().Set(1.0);

  instancer.CreatePositionsAttr().Set(positions);
  instancer.CreateProtoIndicesAttr().Set(protoIndices);
  instancer.CreateScalesAttr().Set(scales);
  instancer.CreateIdsAttr().Set(indices);
  instancer.CreateOrientationsAttr().Set(rotations);
  instancer.CreatePrototypesRel().AddTarget(proto.GetPath());
  UsdGeomPrimvarsAPI primvarsApi(instancer);
  UsdGeomPrimvar colorPrimvar =
    primvarsApi.CreatePrimvar(UsdGeomTokens->primvarsDisplayColor, SdfValueTypeNames->Color3fArray);
  colorPrimvar.SetInterpolation(UsdGeomTokens->varying);
  colorPrimvar.SetElementSize(1);
  colorPrimvar.Set(colors);

  return new Instancer(instancer.GetPrim(), GfMatrix4d(1.0));
}

void _UpdatePointsInstancer(UsdStageRefPtr& stage, SdfPath& path, Points* points, float time)
{

  size_t numPoints = points->GetNumPoints();

  VtArray<GfVec3f> positions(numPoints);
  VtArray<GfVec3f> scales(numPoints);
  VtArray<GfQuath> rotations(numPoints);

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    positions[pointIdx] = points->GetPosition(pointIdx);
    scales[pointIdx] = GfVec3f(1.f);
    rotations[pointIdx] = GfQuath::GetIdentity();
  }

  UsdPrim prim = stage->GetPrimAtPath(path);
  UsdGeomPointInstancer instancer(prim);

  instancer.GetPositionsAttr().Set(positions);
  instancer.GetScalesAttr().Set(scales);
  instancer.GetOrientationsAttr().Set(rotations);

}

Instancer* _SetupGridInstancer(UsdStageRefPtr& stage, SdfPath& path, Grid3D* grid)
{
  VtArray<GfVec3f> points;
  VtArray<GfVec3f> scales;
  VtArray<GfVec3f> colors;

  grid->GetCells(points, scales, colors, false);
  size_t numPoints = points.size();

  VtArray<int64_t> indices(numPoints);
  VtArray<int> protoIndices(numPoints);
  VtArray<GfQuath> rotations(numPoints);

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    protoIndices[pointIdx] = 0;
    indices[pointIdx] = pointIdx;
    rotations[pointIdx] = GfQuath::GetIdentity();
  }

  UsdGeomPointInstancer instancer =
    UsdGeomPointInstancer::Define(stage, path);

  UsdGeomCube proto =
    UsdGeomCube::Define(stage,
      path.AppendChild(TfToken("proto_cube")));
  proto.CreateSizeAttr().Set(1.0);

  instancer.CreatePositionsAttr().Set(points);
  instancer.CreateProtoIndicesAttr().Set(protoIndices);
  instancer.CreateScalesAttr().Set(scales);
  instancer.CreateIdsAttr().Set(indices);
  instancer.CreateOrientationsAttr().Set(rotations);
  instancer.CreatePrototypesRel().AddTarget(proto.GetPath());
  UsdGeomPrimvarsAPI primvarsApi(instancer);
  UsdGeomPrimvar colorPrimvar =
    primvarsApi.CreatePrimvar(UsdGeomTokens->primvarsDisplayColor, SdfValueTypeNames->Color3fArray);
  colorPrimvar.SetInterpolation(UsdGeomTokens->varying);
  colorPrimvar.SetElementSize(1);
  colorPrimvar.Set(colors);

  return new Instancer(instancer.GetPrim(), GfMatrix4d(1.0));
}

void _UpdateGridInstancer(UsdStageRefPtr& stage, SdfPath& path, Grid3D* grid, float time)
{
  VtArray<GfVec3f> points;
  VtArray<GfVec3f> scales;
  VtArray<GfVec3f> colors;

  grid->GetCells(points, scales, colors, false);

  UsdPrim prim = stage->GetPrimAtPath(path);
  UsdGeomPointInstancer instancer(prim);

  instancer.GetPositionsAttr().Set(points);
  instancer.GetScalesAttr().Set(scales);

}



JVR_NAMESPACE_CLOSE_SCOPE