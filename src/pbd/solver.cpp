#include "../pbd/solver.h"
#include "../pbd/constraint.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../acceleration/bvh.h"
#include "../app/application.h"
#include "../app/time.h"

#include "../utils/timer.h"

#include <iostream>

#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/pointInstancer.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/usdGeom/curves.h>



JVR_NAMESPACE_OPEN_SCOPE

static void
_SetupBVHInstancer(pxr::UsdStageRefPtr& stage, BVH* bvh)
{
  pxr::UsdGeomPointInstancer instancer =
    pxr::UsdGeomPointInstancer::Define(stage,
      stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("bvh_instancer")));

  pxr::UsdGeomCube proto =
    pxr::UsdGeomCube::Define(stage,
      instancer.GetPath().AppendChild(pxr::TfToken("proto_cube")));
  proto.CreateSizeAttr().Set(1.f);

  pxr::UsdGeomBasisCurves curve =
    pxr::UsdGeomBasisCurves::Define(stage,
      stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("bvh_curve")));


  std::vector<BVH*> cells;
  bvh->GetCells(cells);
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
      pxr::GfVec3f(bvh->ComputeCodeAsColor(
        pxr::GfVec3f(cells[pointIdx]->GetMidpoint())));
    rotations[pointIdx] = pxr::GfQuath::GetIdentity();
    widths[pointIdx] = RANDOM_0_1;
  }
  
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

  curve.CreatePointsAttr().Set(points);
  curve.SetWidthsInterpolation(pxr::UsdGeomTokens->constant);
  curve.CreateWidthsAttr().Set(widths);
  curve.CreateCurveVertexCountsAttr().Set(curveVertexCount);

  colorPrimvar = curve.CreateDisplayColorPrimvar();
  colorPrimvar.SetInterpolation(pxr::UsdGeomTokens->vertex);
  colorPrimvar.SetElementSize(1);
  colorPrimvar.Set(colors);
  
  curve.SetWidthsInterpolation(pxr::UsdGeomTokens->vertex);
}


PBDSolver::PBDSolver() 
  : _gravity(0,-1,0)
  , _timeStep(0.05)
{
}


PBDSolver::~PBDSolver()
{

}

void PBDSolver::Reset()
{
  _system.Reset();
}

void PBDSolver::AddGeometry(Geometry* geom, const pxr::GfMatrix4f& m)
{
  size_t offset = _system.AddGeometry(geom, m);
  AddConstraints(geom, offset);
}

void PBDSolver::RemoveGeometry(Geometry* geom)
{
  _system.RemoveGeometry(geom);
}

void PBDSolver::AddColliders(std::vector<Geometry*>& colliders)
{
  _colliders = colliders;
  {
    uint64_t T = CurrentTime();
    BVH bvh;
    bvh.Init(_colliders);
    std::cout << "build bvh no mortom : " << ((CurrentTime() - T) * 1e-9) << std::endl;

    T = CurrentTime();
    pxr::GfRay ray(pxr::GfVec3f(0.f, 5.f, 0.f), pxr::GfVec3f(0.f, -1.f, 0.f));
    double minDistance;
    Hit hit;
    if (bvh.Raycast(ray, &hit, -1, &minDistance)) {
      pxr::GfVec3f position;
      hit.GetPosition(&position);
      std::cout << "hit position : " << position << std::endl;
    }
    std::cout << "hit time : " << ((CurrentTime() - T) * 1e-9) << std::endl;
    _SetupBVHInstancer(GetApplication()->GetWorkspace()->GetExecStage(), &bvh);
    BVH::EchoNumHits();
   

  }
  
  {
    uint64_t T = CurrentTime();
    BVH bvh;
    bvh.Init(_colliders, true);
    std::cout << "build bvh with mortom : " << ((CurrentTime() - T) * 1e-9) << std::endl;

    T = CurrentTime();
    pxr::GfRay ray(pxr::GfVec3f(0.f, 5.f, 0.f), pxr::GfVec3f(0.f, -1.f, 0.f));
    double minDistance;
    Hit hit;
    if (bvh.Raycast(ray, &hit, -1, &minDistance)) {
      pxr::GfVec3f position;
      hit.GetPosition(&position);
      std::cout << "hit position : " << position << std::endl;
    }
    std::cout << "hit time : " << ((CurrentTime() - T) * 1e-9) << std::endl;
    _SetupBVHInstancer(GetApplication()->GetWorkspace()->GetExecStage(), &bvh);
    BVH::EchoNumHits();
    
  }
}

void PBDSolver::UpdateColliders()
{
  BVH bvh;
  bvh.Init(_colliders);
  /*
  {
    double minDistance;
    Hit hit;
    if (bvh.Closest(pxr::GfVec3f(0.f), &hit, -1, &minDistance)) {
      std::cout << "CLOSEST HIT :" << std::endl;
      pxr::GfVec3f position;
      hit.GetPosition(&position);
      std::cout << "   pos : " << position << std::endl;
      std::cout << "   tri : " << hit.GetElementIndex() << std::endl;
    }
  }
  */
  {
    pxr::GfRay ray(pxr::GfVec3f(0.f, 5.f, 0.f), pxr::GfVec3f(0.f, -1.f, 0.f));
    double minDistance;
    Hit hit;
    if (bvh.Raycast(ray, &hit, -1, &minDistance)) {
      pxr::GfVec3f position;
      hit.GetPosition(&position);
    }
  }
}

void PBDSolver::AddConstraints(Geometry* geom, size_t offset)
{
  if (geom->GetType() == Geometry::MESH) {
    Mesh* mesh = (Mesh*)geom;
    std::vector<HalfEdge*> edges = mesh->GetUniqueEdges();
    for (HalfEdge* edge : edges) {
      PBDDistanceConstraint* constraint = new PBDDistanceConstraint();
      constraint->Init(this, edge->vertex + offset, edge->next->vertex + offset, 0.5f);
      _constraints.push_back(constraint);
    }
  }
}

void PBDSolver::SatisfyConstraints()
{
  Time& time = GetApplication()->GetTime();
  for (int j = 0; j < 5; j++) {
    for (int i = 0; i < _constraints.size(); i++) {
      PBDConstraint* c = _constraints[i];
      c->Solve(this, 1);
    }
    // Constrain one particle of the cloth to origo
    _system.GetPosition(0) =
      _system.GetInitPositions()[0] + 
        pxr::GfVec3f(0, sin(time.GetActiveTime()) * 5.f + 1.f, 0.f);

    for (int i = 0; i < _system.GetNumParticles(); ++i) {
      pxr::GfVec3f& p = _system.GetPosition(i);


      //_position[i][0] = pxr::GfMin(pxr::GfMax(_position[i][0], -100.f), 100.f); 
      p[1] = pxr::GfMax(p[1], 0.f);
      //_position[i][2] = pxr::GfMin(pxr::GfMax(_position[i][2], 100.f), 100.f);
    }

  }
}

void PBDSolver::Step()
{
  _system.AccumulateForces(_gravity);
  _system.Integrate(_timeStep);
  UpdateColliders();
  SatisfyConstraints();
  
  _system.UpdateGeometries();
}

JVR_NAMESPACE_CLOSE_SCOPE