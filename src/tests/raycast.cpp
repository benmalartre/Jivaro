#include "../geometry/sampler.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/voxels.h"
#include "../geometry/points.h"
#include "../geometry/implicit.h"
#include "../geometry/triangle.h"
#include "../geometry/utils.h"
#include "../geometry/scene.h"

#include "../pbd/particle.h"
#include "../pbd/force.h"
#include "../pbd/constraint.h"
#include "../pbd/collision.h"
#include "../pbd/solver.h"
#include "../utils/timer.h"

#include "../app/application.h"
#include "../command/block.h"

#include "../tests/utils.h"
#include "../tests/raycast.h"

JVR_NAMESPACE_OPEN_SCOPE


void _UpdateRays(Mesh* mesh, Curve* rays) 
{
  const size_t numRays = mesh->GetNumPoints();
  const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
  const pxr::GfVec3f* normals = mesh->GetNormalsCPtr();
  const pxr::GfMatrix4f matrix(mesh->GetMatrix());

  pxr::VtArray<pxr::GfVec3f> points;
  pxr::VtArray<float> radiis;
  pxr::VtArray<int> counts;

  points.resize(numRays * 2);
  radiis.resize(numRays * 2);
  counts.resize(numRays);
  for(size_t r = 0; r < numRays; ++r) {
    counts[r] = 2;
    radiis[r*2]   = 0.05f;
    radiis[r*2+1]   = 0.025f;
    points[r*2]   = matrix.Transform(positions[r]);
    points[r*2+1] = matrix.Transform(positions[r]) + matrix.TransformDir(normals[r]);
  }

  rays->SetTopology(points, radiis, counts);
  
}

void TestRaycast::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  const pxr::SdfPath  rootId = rootPrim.GetPath();
  
  // create mesh that will be source of rays
  pxr::GfQuatf rotation(180.f * DEGREES_TO_RADIANS, pxr::GfVec3f(0.f, 0.f, 1.f));
  rotation.Normalize();

  pxr::GfMatrix4d scale = pxr::GfMatrix4d(1.f).SetScale(pxr::GfVec3f(10.f, 10.f, 10.f));
  pxr::GfMatrix4d rotate = pxr::GfMatrix4d(1.f).SetRotate(rotation);
  pxr::GfMatrix4d translate = pxr::GfMatrix4d(1.f).SetTranslate(pxr::GfVec3f(0.f, 10.f, 0.f));

  _meshId = rootId.AppendChild(pxr::TfToken("Emitter"));
  _mesh = _GenerateMeshGrid(stage, _meshId, 32, 32, scale * rotate * translate);
  _scene.AddGeometry(_meshId, _mesh);


  _raysId = rootId.AppendChild(pxr::TfToken("Rays"));
  _rays = new Curve();
  _scene.AddGeometry(_raysId, _rays);

  _UpdateRays(_mesh, _rays);

  Scene::_Prim* prim = _scene.GetPrim(_raysId);
  prim->bits = pxr::HdChangeTracker::AllDirty;


  _scene.Update(stage, 1);


}


void TestRaycast::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Update(stage, time);

  _UpdateRays(_mesh, _rays);
  Scene::_Prim* prim = _scene.GetPrim(_raysId);
  prim->bits = pxr::HdChangeTracker::AllDirty;
}

void TestRaycast::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;
  stage->RemovePrim(_meshId);
  
  _scene.RemoveGeometry(_meshId);
  _scene.RemoveGeometry(_raysId);
  delete _mesh;
  delete _rays;

}

JVR_NAMESPACE_CLOSE_SCOPE