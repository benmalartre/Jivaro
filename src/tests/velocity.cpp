#include <pxr/usd/usdGeom/xform.h>

#include "../geometry/points.h"
#include "../pbd/solver.h"
#include "../pbd/collision.h"
#include "../tests/utils.h"
#include "../tests/velocity.h"

JVR_NAMESPACE_OPEN_SCOPE


Points* _CreateGridPoint(const GfVec3f& center, size_t N) 
{
  size_t numPoints = N * N;

  VtArray<GfVec3f> positions(numPoints);
  VtArray<float> widths(numPoints);
  VtArray<GfVec3f> colors(numPoints);

  for(size_t z = 0; z < N; ++z) 
    for(size_t y = 0; y< N; ++y) {
      positions[z * N + y] = GfVec3f(center[0]+RANDOM_LO_HI(-0.01,0.01), center[1] + y+RANDOM_LO_HI(-0.01,0.01), center[2] + z+RANDOM_LO_HI(-0.01,0.01));
      widths[z * N + y] = 0.5f;
      colors[z * N + y] = GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    }

  Points* points= new Points();

  points->SetPositions(positions);
  points->SetWidths(widths);
  points->SetColors(colors);
  //points->SetInputOnly();

  return points;
}

void TestVelocity::InitExec(UsdStageRefPtr& stage)
{
  if (!stage) return;

  // get root prim
  UsdPrim rootPrim = stage->GetDefaultPrim();
  if(!rootPrim.IsValid()) {
    UsdGeomXform root = UsdGeomXform::Define(stage, SdfPath("/Root"));
    rootPrim = root.GetPrim();
    stage->SetDefaultPrim(rootPrim);
  }
  const SdfPath  rootId = rootPrim.GetPath();

  {
    _points0 = _CreateGridPoint(GfVec3f(-1.f, 2.f, 0.f), 16);
    _points0Id = rootId.AppendChild(TfToken("Points0"));
    _scene.InjectGeometry(stage, _points0Id, _points0, 1.f);
    _scene.AddGeometry(_points0Id, _points0);
  }

  {
    _points1 = _CreateGridPoint(GfVec3f(1.f, 2.f, 0.f), 16);
    _points1Id = rootId.AppendChild(TfToken("Points1"));
    _scene.InjectGeometry(stage, _points1Id, _points1, 1.f);
    _scene.AddGeometry(_points1Id, _points1);
  }

  _solverId =  rootId.AppendChild(TfToken("Solver"));  
  _solver = _CreateSolver(&_scene, stage, _solverId);


  Body* body0 = _solver->CreateBody(_points0, GfMatrix4d(1.0), 1.f, 0.25f, 0.1f, false);
  _solver->SetBodyVelocity(body0, GfVec3f(1.f, 0.f, 0.f));
  _solver->AddElement(body0, _points0, _points0Id);
  Body* body1 = _solver->CreateBody(_points1, GfMatrix4d(1.0), 1.f, 0.25f, 0.1f, false);
  _solver->SetBodyVelocity(body1, GfVec3f(-1.f, 0.f, 0.f));
  _solver->AddElement(body1, _points1, _points1Id);

  bool createGroundCollision = true;
  if(createGroundCollision) {
    // create collide ground
    _groundId = rootId.AppendChild(TfToken("Ground"));
    _ground = _CreateCollidePlane(stage, _groundId, 0.5f, 0.f);
    _ground->SetMatrix(
      GfMatrix4d().SetTranslate(GfVec3f(0.f, -0.5f, 0.f)));
    //_AddAnimationSamples(stage, _groundId);
    _scene.AddGeometry(_groundId, _ground);

    Collision* planeCollide = new PlaneCollision(_ground, _groundId, 0.5, 0.5);
    _solver->AddElement(planeCollide, _ground, _groundId);

    std::cout << "added ground collision" << std::endl;
  }

   bool createSelfCollision = true;
  if (createSelfCollision) {
    SdfPath selfCollideId = _solverId.AppendChild(TfToken("SelfCollision"));
    Collision* selfCollide = new SelfCollision(_solver->GetParticles(), selfCollideId, 0.f, 1.f);
    _solver->AddElement(selfCollide, NULL, selfCollideId);

  }
  

  _solver->Update(stage, _solver->GetStartTime());
}


void TestVelocity::UpdateExec(UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  _solver->Update(stage, time);

}

void TestVelocity::TerminateExec(UsdStageRefPtr& stage)
{
  if (!stage) return;
}

JVR_NAMESPACE_CLOSE_SCOPE