#include "../pbd/solver.h"
#include "../pbd/constraint.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/voxels.h"
#include "../acceleration/bvh.h"
#include "../app/application.h"
#include "../app/time.h"

#include "../utils/timer.h"

#include <iostream>

#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/pointInstancer.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/usdGeom/curves.h>



JVR_NAMESPACE_OPEN_SCOPE

static void
_SetupResults(pxr::UsdStageRefPtr& stage, std::vector<pxr::GfVec3f>& points)
{

  pxr::UsdGeomXform pntGroup =
    pxr::UsdGeomXform::Define(stage, stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("intersection")));

  size_t rayIndex = 0;
  pxr::VtArray<pxr::GfVec3f> colors(1);


  std::string name = "ray_intersection_";
  for (auto& point : points) {
    colors[0] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);

    pxr::UsdGeomSphere origin =
      pxr::UsdGeomSphere::Define(stage, pntGroup.GetPath().AppendChild(
        pxr::TfToken(name + std::to_string(rayIndex))
      ));

    origin.AddTranslateOp().Set(pxr::GfVec3d(point));
    origin.CreateRadiusAttr().Set(0.2);
    origin.CreateDisplayColorAttr().Set(colors);

    rayIndex++;

  }
}

static void 
BenchmarkParallelEvaluation(PBDSolver* solver)
{
  std::cout << "benchmark parallel evaluation" << std::endl;
  for (size_t i = 1; i <= 256; i*=2) {
    uint64_t startT = CurrentTime();
    solver->SetNumTasks(i);
    solver->Reset();
    for (size_t t = 0; t < 250; ++t) {
      solver->Step();
    }
    std::cout << "[parallel] (" << i << " threads) took " << ((CurrentTime() - startT) * 1e-9) << " seconds" << std::endl;
  }

  uint64_t startT = CurrentTime();
  PBDParticle* system = solver->GetSystem();
  size_t last = system->GetNumParticles();
  system->Reset(0, last);
  for (size_t t = 0; t < 250; ++t) {
    system->AccumulateForces(0, last, solver->GetGravity());
    system->Integrate(0, last, solver->GetTimeStep());

    for (size_t i = 0; i < last; ++i) {
      pxr::GfVec3f& p = system->GetPosition(i);

      //_position[i][0] = pxr::GfMin(pxr::GfMax(_position[i][0], -100.f), 100.f); 
      p[1] = pxr::GfMax(p[1], 0.f);
      //_position[i][2] = pxr::GfMin(pxr::GfMax(_position[i][2], 100.f), 100.f);
    }
    
    size_t numConstraints = solver->GetNumConstraints();
    for (size_t i = 0; i < numConstraints; ++i) {
      PBDConstraint* c = solver->GetConstraint(i);
      c->Solve(solver, 1);
    }
    


  }
  std::cout << "[serial] (1 thread) took " << ((CurrentTime() - startT) * 1e-9) << " seconds" << std::endl;
}

static void
_SetupBVHInstancer(pxr::UsdStageRefPtr& stage, BVH* bvh)
{
  std::vector<BVH::Cell*> cells;
  std::cout << "setup instancer " << bvh->GetRoot() << std::endl;
  bvh->GetRoot()->GetCells(cells);
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

  /*
  pxr::UsdGeomPointInstancer instancer =
    pxr::UsdGeomPointInstancer::Define(stage,
      stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("bvh_instancer")));

  pxr::UsdGeomCube proto =
    pxr::UsdGeomCube::Define(stage,
      instancer.GetPath().AppendChild(pxr::TfToken("proto_cube")));
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
  */

  pxr::UsdGeomBasisCurves curve =
    pxr::UsdGeomBasisCurves::Define(stage,
      stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("bvh_curve")));

  curve.CreatePointsAttr().Set(points);
  curve.SetWidthsInterpolation(pxr::UsdGeomTokens->constant);
  curve.CreateWidthsAttr().Set(widths);
  curve.CreateCurveVertexCountsAttr().Set(curveVertexCount);

  pxr::UsdGeomPrimvar crvColorPrimvar = curve.CreateDisplayColorPrimvar();
  crvColorPrimvar.SetInterpolation(pxr::UsdGeomTokens->vertex);
  crvColorPrimvar.SetElementSize(1);
  crvColorPrimvar.Set(colors);
  
  curve.SetWidthsInterpolation(pxr::UsdGeomTokens->vertex);
}

static void
_SetupVoxels(pxr::UsdStageRefPtr& stage, Voxels* voxels, float radius)
{

  pxr::VtArray<pxr::GfVec3f> positions;
  voxels->Init();
  voxels->Trace(0);
  voxels->Trace(1);
  voxels->Trace(2);
  voxels->Build(positions);

  size_t numPoints = positions.size();
  pxr::UsdGeomPoints points =
    pxr::UsdGeomPoints::Define(
      stage,
      stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("Voxels")));

  points.CreatePointsAttr().Set(pxr::VtValue(positions));

  pxr::VtArray<float> widths(numPoints);
  pxr::VtArray<pxr::GfVec3f> colors(numPoints);
  for (size_t p = 0; p < numPoints; ++p) {
    widths[p] = radius;
    colors[p] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  }
  points.CreateWidthsAttr().Set(pxr::VtValue(widths));
  points.SetWidthsInterpolation(pxr::UsdGeomTokens->varying);
  //points.CreateNormalsAttr().Set(pxr::VtValue({pxr::GfVec3f(0, 1, 0)}));
  points.CreateDisplayColorAttr().Set(pxr::VtValue(colors));
  points.GetDisplayColorPrimvar().SetInterpolation(pxr::UsdGeomTokens->varying);

}


struct SolverTaskData : public ThreadPool::TaskData {
  PBDSolver* solver;
  size_t     startIdx;
  size_t     endIdx;
};

void _SolveCollisions(ThreadPool::TaskData* data)
{
  SolverTaskData* taskData = (SolverTaskData*)data;
  PBDSolver* solver = taskData->solver;
  PBDParticle* system = solver->GetSystem();
  for (size_t i = taskData->startIdx; i < taskData->endIdx; ++i) {
    pxr::GfVec3f& p = system->GetPosition(i);

    //_position[i][0] = pxr::GfMin(pxr::GfMax(_position[i][0], -100.f), 100.f); 
    p[1] = pxr::GfMax(p[1], 0.f);
    //_position[i][2] = pxr::GfMin(pxr::GfMax(_position[i][2], 100.f), 100.f);
  }
}

void _SatisfyConstraints(ThreadPool::TaskData* data)
{
  SolverTaskData* taskData = (SolverTaskData*)data;
  PBDSolver* solver = taskData->solver;
  PBDParticle* system = solver->GetSystem();
  for (size_t i = taskData->startIdx; i < taskData->endIdx; ++i) {
    PBDConstraint* c = solver->GetConstraint(i);
    c->Solve(solver, 1);
  }
}

void _Integrate(ThreadPool::TaskData* data)
{
  SolverTaskData* taskData = (SolverTaskData*)data;
  PBDSolver* solver = taskData->solver;
  PBDParticle* system = solver->GetSystem();
  system->AccumulateForces(taskData->startIdx, taskData->endIdx, solver->GetGravity());
  system->Integrate(taskData->startIdx, taskData->endIdx, solver->GetTimeStep());
}

void _Reset(ThreadPool::TaskData* data)
{
  SolverTaskData* taskData = (SolverTaskData*)data;
  PBDSolver* solver = taskData->solver;
  PBDParticle* system = solver->GetSystem();
  system->Reset(taskData->startIdx, taskData->endIdx);
}


PBDSolver::PBDSolver()
  : _gravity(0, -1, 0)
  , _timeStep(1.f / 60.f)
  , _substeps(15)
  , _paused(true)
{
  _pool.Init();
}


PBDSolver::~PBDSolver()
{

}

void
PBDSolver::_ParallelEvaluation(ThreadPool::TaskFn fn, size_t numElements, size_t numTasks)
{
  _pool.BeginTasks();
  std::vector<SolverTaskData> datas(numTasks);

  size_t chunkSize = (numElements + numTasks - (numElements % numTasks)) / numTasks;
  for (size_t t = 0; t < numTasks; ++t) {
    datas[t].solver = this;
    datas[t].startIdx = t * chunkSize;
    datas[t].endIdx = pxr::GfMin((t + 1) * chunkSize, numElements);
    _pool.AddTask(fn, &datas[t]);
  }
  _pool.EndTasks();
}

void PBDSolver::Reset()
{
  UpdateColliders();
  
  // reset
  _ParallelEvaluation(_Reset, _system.GetNumParticles(), _numTasks);
}

void PBDSolver::AddGeometry(Geometry* geom, const pxr::GfMatrix4f& m)
{
  if (_geometries.find(geom) == _geometries.end()) {
    _geometries[geom] = PBDGeometry({ geom, _system.GetNumParticles(), m, m.GetInverse() });
    size_t offset = _system.AddGeometry(&_geometries[geom]);
    std::cout << "[solver] added geometry : " << offset << std::endl;
    AddConstraints(geom, offset);
    std::cout << "[solver] added constraints : " << _constraints.size() << std::endl;
  }
  
}

void PBDSolver::RemoveGeometry(Geometry* geom)
{
  if (_geometries.find(geom) != _geometries.end()) {
    _system.RemoveGeometry(&_geometries[geom]);
    _geometries.erase(geom);
  }
}

void PBDSolver::AddColliders(std::vector<Geometry*>& colliders)
{
  pxr::UsdStageRefPtr stage = GetApplication()->GetWorkspace()->GetExecStage();

  _colliders = colliders;
  float radius = 0.2f;
  Voxels voxels(colliders[0], radius);
  _SetupVoxels(stage, &voxels, radius);
  std::cout << "[solver] added colliders " << std::endl;

  BenchmarkParallelEvaluation(this);
  /*
  size_t numRays = 2048;
  std::vector<pxr::GfRay> rays(numRays);
  for (size_t r = 0; r < numRays; ++r) {
    rays[r] = pxr::GfRay(pxr::GfVec3f(0.f), 
      pxr::GfVec3f(RANDOM_LO_HI(-1, 1), RANDOM_LO_HI(-1, 1), RANDOM_LO_HI(-1, 1)).GetNormalized());
  }

  std::vector<pxr::GfVec3f> result;


  _colliders = colliders;

  std::cout << "### build bvh (mortom) : " << std::endl;
  for(auto& collider: _colliders) {
    std::cout << " collider : " << collider << std::endl;
  }
  uint64_t T = CurrentTime();
  BVH bvh;
  bvh.Init(_colliders);
  std::cout << "   build boundary volume hierarchy : " << ((CurrentTime() - T) * 1e-9) << std::endl;

  T = CurrentTime();
  for (auto& ray : rays) {
    double minDistance = DBL_MAX;
    Hit hit;
    const pxr::GfVec3f* points = _colliders[0]->GetPositionsCPtr();
    if (bvh.Raycast(points, ray, &hit, DBL_MAX, &minDistance)) {
      result.push_back(hit.GetPosition(colliders[hit.GetGeometryIndex()]));
    }
  }
  std::cout << "   hit time (" << numRays << " rays) : " << ((CurrentTime() - T) * 1e-9) << std::endl;
  _SetupBVHInstancer(stage, &bvh);


  _SetupResults(stage, result);
  */
}

void PBDSolver::UpdateColliders()
{
  /*
  BVH bvh;
  bvh.Init(_colliders);
  
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
  
  {
    pxr::GfRay ray(pxr::GfVec3f(0.f, 5.f, 0.f), pxr::GfVec3f(0.f, -1.f, 0.f));
    double minDistance;
    Hit hit;
    const pxr::GfVec3f* points = _colliders[0]->GetPositionsCPtr();
    if (bvh.Raycast(points, ray, &hit, -1, &minDistance)) {
      pxr::GfVec3f position;
      hit.GetPosition(_colliders[0]);
    }
  }
  */
}

void PBDSolver::AddConstraints(Geometry* geom, size_t offset)
{
  std::cout << "[solver] add constraints for type " << geom->GetType() << std::endl;
  if (geom->GetType() == Geometry::MESH) {
    std::cout << "[solver] add constraints for mesh : " << std::endl;
    Mesh* mesh = (Mesh*)geom;
    std::vector<HalfEdge*> edges = mesh->GetUniqueEdges();
    std::cout << "[solver] num unique edges : " << edges.size() << std::endl;
    for (HalfEdge* edge : edges) {
      PBDDistanceConstraint* constraint = new PBDDistanceConstraint();
      constraint->Init(this, edge->vertex + offset, edge->next->vertex + offset, 0.5f);
      _constraints.push_back(constraint);
    }
  } else if (geom->GetType() == Geometry::CURVE) {
    Curve* curve = (Curve*)geom;
    curve->GetTotalNumSegments();
    for (size_t curveIdx = 0; curveIdx < curve->GetNumCurves(); ++curveIdx) {

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
      _system.GetRestPositions()[0] + 
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
  UpdateColliders();
  // integrate
  _ParallelEvaluation(_Integrate, _system.GetNumParticles(), _numTasks);
  _ParallelEvaluation(_SolveCollisions, _system.GetNumParticles(), _numTasks);
  _ParallelEvaluation(_SatisfyConstraints, GetNumConstraints(), _numTasks);

  UpdateGeometries();
}

void PBDSolver::UpdateGeometries()
{
  std::map<Geometry*, PBDGeometry>::iterator it = _geometries.begin();
  for (; it != _geometries.end(); ++it)
  {
    Geometry* geom = it->first;
    size_t numPoints = geom->GetNumPoints();
    pxr::VtArray<pxr::GfVec3f> results(numPoints);
    const auto& positions = _system.GetPositions();
    for (size_t p = 0; p < numPoints; ++p) {
      results[p] = it->second.invMatrix.Transform(positions[it->second.offset + p]);
    }
    geom->SetPositions(&results[0], numPoints);
  }
}

JVR_NAMESPACE_CLOSE_SCOPE