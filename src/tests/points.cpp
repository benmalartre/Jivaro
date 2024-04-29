#include <pxr/usd/usdGeom/xform.h>

#include "../geometry/points.h"
#include "../tests/points.h"

JVR_NAMESPACE_OPEN_SCOPE


void TestPoints::InitExec(pxr::UsdStageRefPtr& stage)
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

  _points = new Points();
  _pointsId = rootId.AppendChild(pxr::TfToken("Particles"));

  size_t N = 1024;
  float length = 5.f;
  pxr::VtArray<pxr::GfVec3f> positions(N);
  pxr::VtArray<float> radius(N);
  pxr::VtArray<pxr::GfVec3f> colors(N);


  for(size_t n = 0; n < N; ++n) {
    positions[n] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1) * length;
    radius[n] = 0.25f;
    colors[n] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  }
  _points->SetPositions(positions);
  _points->SetRadii(radius);
  _points->SetColors(colors);

  _scene.AddGeometry(_pointsId, _points);
}


void TestPoints::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);

}

void TestPoints::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;
}

JVR_NAMESPACE_CLOSE_SCOPE