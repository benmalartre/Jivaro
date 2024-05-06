#include <pxr/usd/usdGeom/xform.h>

#include "../geometry/points.h"
#include "../pbd/solver.h"
#include "../pbd/collision.h"
#include "../tests/utils.h"
#include "../tests/velocity.h"

JVR_NAMESPACE_OPEN_SCOPE


Points* _CreateGridPoint(const pxr::GfVec3f& center, size_t N) 
{
  size_t numPoints = N * N;

  pxr::VtArray<pxr::GfVec3f> positions(numPoints);
  pxr::VtArray<float> widths(numPoints);
  pxr::VtArray<pxr::GfVec3f> colors(numPoints);

  for(size_t z = 0; z < N; ++z) 
    for(size_t y = 0; y< N; ++y) {
      positions[z * N + y] = pxr::GfVec3f(center[0], center[1] + y, center[2] + z);
      widths[z * N + y] = 0.25f;
      colors[z * N + y] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    }

  Points* points= new Points();

  points->SetPositions(positions);
  points->SetWidths(widths);
  points->SetColors(colors);
  points->SetInputOnly();

  return points;
}

void TestVelocity::InitExec(pxr::UsdStageRefPtr& stage)
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

  {
    _points0 = _CreateGridPoint(pxr::GfVec3f(-1.f, 2.f, 0.f), 16);
    _points0Id = rootId.AppendChild(pxr::TfToken("Points0"));

    _scene.AddGeometry(_points0Id, _points0);
  }

  {
    _points1 = _CreateGridPoint(pxr::GfVec3f(1.f, 2.f, 0.f), 16);
    _points1Id = rootId.AppendChild(pxr::TfToken("Points1"));

    _scene.AddGeometry(_points1Id, _points1);
  }

  _solverId =  rootId.AppendChild(pxr::TfToken("Solver"));
  _solver = _GenerateSolver(&_scene, stage, _solverId);


  Body* body0 = _solver->CreateBody(_points0, pxr::GfMatrix4d(1.0), 0.1f, 0.25f, 0.1f);
  _solver->SetBodyVelocity(body0, pxr::GfVec3f(1.f, 0.f, 0.f));
  _solver->AddElement(body0, _points0, _points0Id);
  Body* body1 = _solver->CreateBody(_points1, pxr::GfMatrix4d(1.0), 1.f, 0.25f, 0.1f);
  _solver->SetBodyVelocity(body1, pxr::GfVec3f(-1.f, 0.f, 0.f));
  _solver->AddElement(body1, _points1, _points1Id);

  bool createGroundCollision = true;
  if(createGroundCollision) {
    // create collide ground
    _groundId = rootId.AppendChild(pxr::TfToken("Ground"));
    _ground = _GenerateCollidePlane(stage, _groundId, 0.5, 0.5);
    _ground->SetMatrix(
      pxr::GfMatrix4d().SetTranslate(pxr::GfVec3f(0.f, -0.5f, 0.f)));
    //_AddAnimationSamples(stage, _groundId);
    _scene.AddGeometry(_groundId, _ground);

    Collision* planeCollide = new PlaneCollision(_ground, _groundId, 0.5, 0.5);
    _solver->AddElement(planeCollide, _ground, _groundId);

    std::cout << "added ground collision" << std::endl;
  }

   bool createSelfCollision = false;
  if (createSelfCollision) {
    pxr::SdfPath selfCollideId = _solverId.AppendChild(pxr::TfToken("SelfCollision"));
    Collision* selfCollide = new SelfCollision(_solver->GetParticles(), selfCollideId, 0.25f, 0.25f);
    _solver->AddElement(selfCollide, NULL, selfCollideId);

  }
  

  _solver->Update(stage, _solver->GetStartTime());
}


void TestVelocity::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  _solver->Update(stage, time);

}

void TestVelocity::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;
}

JVR_NAMESPACE_CLOSE_SCOPE