#include <pxr/usd/usdGeom/xform.h>

#include "../geometry/points.h"
#include "../tests/points.h"

JVR_NAMESPACE_OPEN_SCOPE


void TestPoints::InitExec(UsdStageRefPtr& stage)
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

  _points = new Points();
  _pointsId = rootId.AppendChild(TfToken("Particles"));

  size_t N = 1024;
  float length = 5.f;
  VtArray<GfVec3f> positions(N);
  VtArray<float> widths(N);
  VtArray<GfVec3f> colors(N);


  for(size_t n = 0; n < N; ++n) {
    positions[n] = GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1) * length;
    widths[n] = 0.25f;
    colors[n] = GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  }
  _points->SetPositions(positions);
  _points->SetWidths(widths);
  _points->SetColors(colors);

  _scene.AddGeometry(_pointsId, _points);
}


void TestPoints::UpdateExec(UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);

}

void TestPoints::TerminateExec(UsdStageRefPtr& stage)
{
  if (!stage) return;
}

JVR_NAMESPACE_CLOSE_SCOPE