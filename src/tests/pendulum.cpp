#include <pxr/usd/usdGeom/xform.h>

#include "../geometry/points.h"
#include "../pbd/solver.h"
#include "../pbd/collision.h"
#include "../pbd/constraint.h"
#include "../tests/utils.h"
#include "../tests/pendulum.h"

JVR_NAMESPACE_OPEN_SCOPE


Points* _CreatePendulum(size_t N) 
{
  size_t numPoints = N + 1;

  pxr::VtArray<pxr::GfVec3f> positions(numPoints);
  pxr::VtArray<float> widths(numPoints);
  pxr::VtArray<pxr::GfVec3f> colors(numPoints);

  for(size_t x = 0; x < numPoints; ++x) {
    positions[x] = pxr::GfVec3f(x *0.25f, 10, 0);
    widths[x] = 0.25f;
    colors[x] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  }

  Points* points= new Points();

  points->SetPositions(positions);
  points->SetWidths(widths);
  points->SetColors(colors);
  //points->SetInputOnly();

  return points;
}

void TestPendulum::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  // get root prim
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  if(!rootPrim.IsValid()) {
    pxr::UsdGeomXform root = pxr::UsdGeomXform::Define(stage, pxr::SdfPath("/Root"));
    rootPrim = root.GetPrim();
    stage->SetDefaultPrim(rootPrim);
  }
  const pxr::SdfPath  rootId = rootPrim.GetPath();
  const size_t N = 16;
  {
    _points = _CreatePendulum(N);
    _pointsId = rootId.AppendChild(pxr::TfToken("pendulum"));

    _scene.AddGeometry(_pointsId, _points);
    _scene.InjectGeometry(stage, _pointsId, _points, 1.f);
  }

  _solverId =  rootId.AppendChild(pxr::TfToken("Solver"));
  _solver = _CreateSolver(&_scene, stage, _solverId);


  Body* body = _solver->CreateBody(_points, pxr::GfMatrix4d(1.0), 1.f, 0.25f, 0.f);

  //_solver->SetBodyVelocity(body, pxr::GfVec3f(0.01f, 0.f, 0.f));
  _solver->AddElement(body, _points, _pointsId);

  pxr::VtArray<int> elements(2 * N);
  for(size_t i = 0; i < N; ++i) {
    elements[i * 2] = i;
    elements[i * 2 + 1] = i+1;
  }

  StretchConstraint* stretch = new StretchConstraint(body, elements, 200000.f, 0.5f);
  _solver->AddConstraint(stretch);

  bool createSelfCollision = false;
  if (createSelfCollision) {
    pxr::SdfPath selfCollideId = _solverId.AppendChild(pxr::TfToken("SelfCollision"));
    Collision* selfCollide = new SelfCollision(_solver->GetParticles(), selfCollideId, 0.f, 0.25f);
    _solver->AddElement(selfCollide, NULL, selfCollideId);

  }
  
  _solver->Update(stage, _solver->GetStartTime());
}


void TestPendulum::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);

  float* mass = &_solver->GetParticles()->mass[0];
  float* invMass = &_solver->GetParticles()->invMass[0];
  mass[0] = 0.f;
  invMass[0] = 0.f;
  _solver->Update(stage, time);

}

void TestPendulum::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;
}

JVR_NAMESPACE_CLOSE_SCOPE