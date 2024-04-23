#include "../geometry/implicit.h"
#include "../geometry/curve.h"

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
  
  // create xform
  _xfoId = rootId.AppendChild(pxr::TfToken("Sphere"));
  _xfo = (Sphere*)_scene.AddGeometry(_xfoId, Geometry::SPHERE, pxr::GfMatrix4d(1.f));
  _xfo->SetRadius(0.5f);
  _xfo->SetInputOnly();

  _scene.InjectGeometry(stage, _xfoId, _xfo, 0.f);

  // create curve
  _curveId = rootId.AppendChild(pxr::TfToken("Curve"));
  _curve = (Curve*)_scene.AddGeometry(_curveId, Geometry::CURVE, pxr::GfMatrix4d(1.f));

  pxr::VtArray<pxr::GfVec3f> positions(2);
  positions[0] = pxr::GfVec3f(0.f);
  positions[1] = pxr::GfVec3f(0.f,10.f,0.f);

  pxr::VtArray<int> cvCounts(1);
  cvCounts[0] = 2;
  _curve->Set(positions, cvCounts);
  _scene.MarkPrimDirty(_curveId, pxr::HdChangeTracker::AllDirty);

}


void TestVelocity::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  const pxr::GfVec3f pos(0.f);//(_xfo->GetMatrix().GetRow3(3));
  const pxr::GfVec3f vel = _xfo->GetVelocity();
  const pxr::GfVec3f ang = xfo->GetTorque().Transform(pxr::GfVec3f(1.f, 0f, 0.f));

  std::cout << "velocity : " << vel << std::endl;

  pxr::VtArray<pxr::GfVec3f> positions(4);
  pxr::VtArray<pxr::GfVec3f> colors(4);
  positions[0] = pos;
  positions[1] = pos + vel;
  positions[2] = pos;
  positions[3] = pos + ang;

  colors[0] = colors[1] = pxr::GfVec3f(1.f, 0.f, 0.f);
  colors[2] = colors[3] = pxr::GfVec3f(0.f, 1.f, 0.f);

  _curve->SetPositions(positions);
  _curve->SetColors(colors);
  
  _scene.MarkPrimDirty(_curveId, pxr::HdChangeTracker::DirtyPoints);
}

void TestVelocity::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  
}

JVR_NAMESPACE_CLOSE_SCOPE