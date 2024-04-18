#include "../geometry/sampler.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/voxels.h"
#include "../geometry/points.h"
#include "../geometry/implicit.h"
#include "../geometry/triangle.h"
#include "../geometry/utils.h"
#include "../pbd/particle.h"
#include "../pbd/force.h"
#include "../pbd/constraint.h"
#include "../pbd/collision.h"
#include "../pbd/solver.h"
#include "../utils/timer.h"

#include "../app/scene.h"
#include "../app/application.h"
#include "../command/block.h"

#include "../tests/utils.h"
#include "../tests/raycast.h"

JVR_NAMESPACE_OPEN_SCOPE


void TestRaycast::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  const pxr::SdfPath  rootId = rootPrim.GetPath();
  
  // create mesh that will be source of rays
  pxr::GfMatrix4d matrix;
  matrix.SetTranslate(pxr::GfVec3f(0.f, 5.f, 0.f));

  _meshId = rootId.AppendChild(pxr::TfToken("Emitter"));
  _mesh = _GenerateMeshGrid(stage, _meshId, 32, 32, matrix);
  _scene->AddGeometry(_meshId, _mesh);

  _scene->Update(stage, 1);


}


void TestRaycast::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene->Update(stage, time);

}

void TestRaycast::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

}

JVR_NAMESPACE_CLOSE_SCOPE