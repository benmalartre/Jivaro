#include <pxr/usd/usdGeom/xform.h>

#include "../geometry/points.h"
#include "../pbd/solver.h"
#include "../tests/utils.h"
#include "../tests/velocity.h"

JVR_NAMESPACE_OPEN_SCOPE


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
    _points0 = new Points();
    _points0Id = rootId.AppendChild(pxr::TfToken("Points0"));

    pxr::VtArray<pxr::GfVec3f> positions(1);
    pxr::VtArray<float> radius(1);
    pxr::VtArray<pxr::GfVec3f> colors(1);

    positions[0] = pxr::GfVec3f(-1.f, 0.f, 0.f);

    radius[0] = 0.25f;

    colors[0] = pxr::GfVec3f(1.f, 0.f, 0.f);

    _points0->SetPositions(positions);
    _points0->SetRadii(radius);
    _points0->SetColors(colors);

    _points0->SetInputOnly();

    _scene.AddGeometry(_points0Id, _points0);
  }

  {
    _points1 = new Points();
    _points1Id = rootId.AppendChild(pxr::TfToken("Points0"));

    pxr::VtArray<pxr::GfVec3f> positions(1);
    pxr::VtArray<float> radius(1);
    pxr::VtArray<pxr::GfVec3f> colors(1);

    positions[0] = pxr::GfVec3f(1.f, 0.f, 0.f);

    radius[0] = 0.5f;

    colors[0] = pxr::GfVec3f(0.f, 1.f, 0.f);

    _points1->SetPositions(positions);
    _points1->SetRadii(radius);
    _points1->SetColors(colors);

    _points1->SetInputOnly();

    _scene.AddGeometry(_points1Id, _points1);
  }

  _solver = _GenerateSolver(&_scene, stage, rootId.AppendChild(pxr::TfToken("Solver")));


  Body* body0 = _solver->CreateBody(_points0, pxr::GfMatrix4d(1.0), 0.25f, 0.25f, 0.1f);
  _solver->SetBodyVelocity(body0, pxr::GfVec3f(1.f, 0.f, 0.f));
  _solver->AddElement(body0, _points0, _points0Id);
  Body* body1 = _solver->CreateBody(_points1, pxr::GfMatrix4d(1.0), 0.5f, 0.5f, 0.1f);
  _solver->SetBodyVelocity(body1, pxr::GfVec3f(-1.f, 0.f, 0.f));
  _solver->AddElement(body1, _points1, _points1Id);

  _solver->SetGravity(pxr::GfVec3f(0.f));
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