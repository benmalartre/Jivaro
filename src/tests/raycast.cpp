#include "../geometry/sampler.h"
#include "../geometry/location.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/voxels.h"
#include "../geometry/points.h"
#include "../geometry/implicit.h"
#include "../geometry/triangle.h"
#include "../geometry/edge.h"
#include "../geometry/utils.h"
#include "../geometry/scene.h"

#include "../pbd/particle.h"
#include "../pbd/force.h"
#include "../pbd/constraint.h"
#include "../pbd/collision.h"
#include "../pbd/solver.h"
#include "../utils/timer.h"

#include "../tests/raycast.h"

#include "../app/application.h"

#include "../tests/utils.h"

JVR_NAMESPACE_OPEN_SCOPE

void TestRaycast::_UpdateRays() 
{
  const double time = Time::Get()->GetActiveTime();
  const size_t numRays = _mesh->GetNumPoints();

  const pxr::GfVec3f* positions = _mesh->GetPositionsCPtr();
  const pxr::GfVec3f* normals = _mesh->GetNormalsCPtr();
  const pxr::GfMatrix4f matrix(_mesh->GetMatrix());

  pxr::VtArray<pxr::GfVec3f> points;
  pxr::VtArray<float> radiis;
  pxr::VtArray<int> counts;
  pxr::VtArray<pxr::GfVec3f> colors;

  points.resize(numRays * 2);
  radiis.resize(numRays * 2);
  colors.resize(numRays * 2);
  counts.resize(numRays);

  for(size_t r = 0; r < numRays; ++r) {
    counts[r] = 2;
    radiis[r*2]   = 0.01f;
    radiis[r*2+1]   = 0.01f;
    points[r*2]   = matrix.Transform(positions[r]);
    points[r*2+1] = matrix.Transform(positions[r] + normals[r]);
    colors[r*2]   = pxr::GfVec3f(0.25f);
    colors[r*2+1] = pxr::GfVec3f(0.25f);
  }

  _rays->SetTopology(points, radiis, counts); 
  _rays->SetColors(colors);
}

// thread task
void TestRaycast::_FindHits(size_t begin, size_t end, const pxr::GfVec3f* positions, 
  pxr::GfVec3f* results, bool* hits)
{
  for (size_t index = begin; index < end; ++index) {
    pxr::GfRay ray(positions[index*2], positions[index*2+1] - positions[index*2]);
    double minDistance = DBL_MAX;
    Location hit;
    Deformable* deformable;
    if (_bvh.Raycast(ray, &hit, DBL_MAX, &minDistance)) {
      Geometry* collided = _bvh.GetGeometry(hit.GetGeometryIndex());
      switch (collided->GetType()) {
        case Geometry::MESH:
        {
          Mesh* mesh = (Mesh*)collided;
          Triangle* triangle = mesh->GetTriangle(hit.GetElementIndex());
          results[index] = hit.GetPosition(mesh->GetPositionsCPtr(), &triangle->vertices[0], 3, mesh->GetMatrix());
          break;
        }
        case Geometry::CURVE:
        {
          //Curve* curve = (Curve*)collided;
          //Edge* edge = curve->GetEdge(hit.GetElementIndex());
          //results[index] = hit.GetPosition(collided->GetPositionsCPtr(), &edge->vertices[0], 2, curve->GetMatrix());
          break;
        }
      default:
        continue;
      }
      
      hits[index] = true;
    } else {
      hits[index] = false;
    }
  }
}

// parallelize raycast
void TestRaycast::_UpdateHits()
{
  const pxr::GfVec3f* positions = _rays->GetPositionsCPtr();
  size_t numRays = _rays->GetNumPoints() >> 1;

  pxr::VtArray<pxr::GfVec3f> points(numRays);
  pxr::VtArray<bool> hits(numRays, false);

  hits.resize(numRays, false);

   pxr::WorkParallelForN(_rays->GetNumCurves(),
    std::bind(&TestRaycast::_FindHits, this, std::placeholders::_1, 
      std::placeholders::_2, positions, &points[0], &hits[0]));

  // need accumulate result
  pxr::VtArray<pxr::GfVec3f> result;
  result.reserve(numRays);
  for(size_t r = 0; r < numRays; ++r) {
    if(hits[r])result.push_back(points[r]);
  }

  _hits->SetPositions(result);
  
  pxr::VtArray<float> radiis(result.size(), 0.2);
  _hits->SetRadii(radiis);

  pxr::VtArray<pxr::GfVec3f> colors(result.size(), pxr::GfVec3f(1.f, 0.5f, 0.0f));
  _hits->SetColors(colors);

}

void TestRaycast::_TraverseStageFindingMeshes(pxr::UsdStageRefPtr& stage)
{
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  for (pxr::UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      _subjects.push_back(new Mesh(pxr::UsdGeomMesh(prim), xformCache.GetLocalToWorldTransform(prim)));
      _subjectsId.push_back(prim.GetPath());
      _subjects.back()->SetInputOnly();
    }
      
}

void TestRaycast::_AddAnimationSamples(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path)
{
  pxr::UsdPrim prim = stage->GetPrimAtPath(path);
  if(prim.IsValid()) {
    pxr::UsdGeomXformable xformable(prim);
    pxr::GfRotation rotation(pxr::GfVec3f(0.f, 0.f, 1.f), 0.f);
    pxr::GfMatrix4d scale = pxr::GfMatrix4d(1.f).SetScale(pxr::GfVec3f(10.f, 10.f, 10.f));
    pxr::GfMatrix4d rotate = pxr::GfMatrix4d(1.f).SetRotate(rotation);
    pxr::GfMatrix4d translate1 = pxr::GfMatrix4d(1.f).SetTranslate(pxr::GfVec3f(0.f, 0.f, -10.f));
    pxr::GfMatrix4d translate2 = pxr::GfMatrix4d(1.f).SetTranslate(pxr::GfVec3f(0.f, 0.f, 10.f));
    auto op = xformable.GetTransformOp();
    op.Set(scale * rotate * translate1, 1);
    op.Set(scale * rotate * translate2, 101);
  }
}

void TestRaycast::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  const pxr::SdfPath  rootId = rootPrim.GetPath();

  // find meshes in the scene
  _TraverseStageFindingMeshes(stage);

  // create bvh
  if (_subjects.size()) {

    _bvh.Init(_subjects);

    for(size_t s = 0; s < _subjects.size();++s) {
      _scene.AddGeometry(_subjectsId[s], _subjects[s]);
    }

  }
  
  // create mesh that will be source of rays
  pxr::GfRotation rotation(pxr::GfVec3f(0.f, 0.f, 1.f), 0.f);

  pxr::GfMatrix4d scale = pxr::GfMatrix4d(1.f).SetScale(pxr::GfVec3f(10.f, 10.f, 10.f));
  pxr::GfMatrix4d rotate = pxr::GfMatrix4d(1.f).SetRotate(rotation);
  pxr::GfMatrix4d translate = pxr::GfMatrix4d(1.f).SetTranslate(pxr::GfVec3f(0.f, 0.f, 0.f));

  const size_t n = 64;
  _meshId = rootId.AppendChild(pxr::TfToken("emitter"));
  _mesh = _GenerateMeshGrid(stage, _meshId, n, scale * rotate * translate);
  _scene.AddGeometry(_meshId, _mesh);

  std::cout << "num rays : " << pxr::GfPow(n, 2.f) << std::endl;

  _AddAnimationSamples(stage, _meshId);

  // create rays
  _raysId = rootId.AppendChild(pxr::TfToken("rays"));
  _rays = (Curve*)_scene.AddGeometry(_raysId, Geometry::CURVE, pxr::GfMatrix4d(1.0));

  _UpdateRays();
  _scene.MarkPrimDirty(_raysId, pxr::HdChangeTracker ::AllDirty);

  // create hits
  _hitsId = rootId.AppendChild(pxr::TfToken("hits"));
  _hits = (Points*)_scene.AddGeometry(_hitsId, Geometry::POINT, pxr::GfMatrix4d(1.0));

  _UpdateHits();
  UpdateExec(stage, Time::Get()->GetActiveTime());

}

void TestRaycast::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
   _bvh.Update();
  _UpdateRays();
  _scene.MarkPrimDirty(_raysId, pxr::HdChangeTracker::DirtyPoints);
  _scene.MarkPrimDirty(_meshId, pxr::HdChangeTracker::AllDirty);

  _UpdateHits();
  _scene.MarkPrimDirty(_hitsId, pxr::HdChangeTracker::AllDirty);

}

void TestRaycast::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  stage->RemovePrim(_meshId);
  //stage->RemovePrim(_bvhId);

}

JVR_NAMESPACE_CLOSE_SCOPE