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

  const GfVec3f* positions = _mesh->GetPositionsCPtr();
  const GfVec3f* normals = _mesh->GetNormalsCPtr();
  const GfMatrix4d matrix = _mesh->GetMatrix();

  VtArray<GfVec3f> points;
  VtArray<float> radiis;
  VtArray<int> counts;
  VtArray<GfVec3f> colors;

  points.resize(numRays * 2);
  radiis.resize(numRays * 2);
  colors.resize(numRays * 2);
  counts.resize(numRays);

  for(size_t r = 0; r < numRays; ++r) {
    counts[r] = 2;
    radiis[r*2]   = 0.01f;
    radiis[r*2+1]   = 0.01f;
    points[r*2]   = GfVec3f(matrix.Transform(positions[r]));
    points[r*2+1] = GfVec3f(matrix.Transform(positions[r] + normals[r] * 0.2f));
    colors[r*2]   = GfVec3f(0.25f);
    colors[r*2+1] = GfVec3f(0.25f);
  }

  _rays->SetTopology(points, radiis, counts); 
  _rays->SetColors(colors);
}

// thread task
void TestRaycast::_FindHits(size_t begin, size_t end, const GfVec3f* positions, 
  GfVec3f* results, bool* hits)
{
  for (size_t index = begin; index < end; ++index) {
    GfRay ray(positions[index*2], positions[index*2+1] - positions[index*2]);
    double minDistance = DBL_MAX;
    Location hit;
    Deformable* deformable;
    hits[index] = false;

    if (_bvh.Raycast(ray, &hit, DBL_MAX, &minDistance)) {

      const Geometry* collided = _bvh.GetGeometry(hit.GetGeometryIndex());
      const GfMatrix4d& matrix = collided->GetMatrix();
      switch (collided->GetType()) {
        case Geometry::MESH:
        {
          Mesh* mesh = (Mesh*)collided;
          Triangle* triangle = mesh->GetTriangle(hit.GetComponentIndex());
          results[index] = hit.ComputePosition(mesh->GetPositionsCPtr(), 
            &triangle->vertices[0], 3, &matrix);
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
    } 
  }
}

// parallelize raycast
void TestRaycast::_UpdateHits()
{
  const GfVec3f* positions = _rays->GetPositionsCPtr();
  size_t numRays = _rays->GetNumPoints() >> 1;

  VtArray<GfVec3f> points(numRays);
  VtArray<bool> hits(numRays, false);

  hits.resize(numRays, false);

  WorkParallelForN(_rays->GetNumCurves(),
    std::bind(&TestRaycast::_FindHits, this, std::placeholders::_1, 
      std::placeholders::_2, positions, &points[0], &hits[0]), 32);

  // need accumulate result
  VtArray<GfVec3f> result;
  result.reserve(numRays);
  for(size_t r = 0; r < numRays; ++r) {
    if(hits[r])result.push_back(points[r]);
  }

  _hits->SetPositions(result);
  
  VtArray<float> widths(result.size(), 0.2f);
  _hits->SetWidths(widths);

  VtArray<GfVec3f> colors(result.size(), GfVec3f(1.f, 0.5f, 0.0f));
  _hits->SetColors(colors);

}

void TestRaycast::_TraverseStageFindingMeshes(UsdStageRefPtr& stage)
{
  UsdGeomXformCache xformCache(UsdTimeCode::Default());
  for (UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<UsdGeomMesh>()) {
      _subjects.push_back(new Mesh(UsdGeomMesh(prim), xformCache.GetLocalToWorldTransform(prim)));
      _subjectsId.push_back(prim.GetPath());
      //_subjects.back()->SetInputOnly();
    } 
}

void TestRaycast::_AddAnimationSamples(UsdStageRefPtr& stage, SdfPath& path)
{
  UsdPrim prim = stage->GetPrimAtPath(path);
  if(prim.IsValid()) {
    UsdGeomXformable xformable(prim);
    GfRotation rotation(GfVec3f(0.f, 0.f, 1.f), 180.f);
    GfMatrix4d scale = GfMatrix4d().SetScale(GfVec3f(10.f, 10.f, 10.f));
    GfMatrix4d rotate = GfMatrix4d().SetRotate(rotation);
    GfMatrix4d translate1 = GfMatrix4d().SetTranslate(GfVec3f(0.f, 12.f, -10.f));
    GfMatrix4d translate2 = GfMatrix4d().SetTranslate(GfVec3f(0.f, 12.f, 10.f));
    auto op = xformable.GetTransformOp();
    op.Set(scale * rotate * translate1, 1);
    op.Set(scale * rotate * translate2, 101);
  }
}

void TestRaycast::InitExec(UsdStageRefPtr& stage)
{
  if (!stage) return;

  UsdPrim rootPrim = stage->GetDefaultPrim();
  const SdfPath  rootId = rootPrim.GetPath();

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
  GfRotation rotation(GfVec3f(0.f, 0.f, 1.f), 0.f);

  GfMatrix4d scale = GfMatrix4d().SetScale(GfVec3f(10.f, 10.f, 10.f));
  GfMatrix4d rotate = GfMatrix4d().SetRotate(rotation);
  GfMatrix4d translate = GfMatrix4d().SetTranslate(GfVec3f(0.f, 0.f, 0.f));

  const size_t n = 1024;
  _meshId = rootId.AppendChild(TfToken("emitter"));
  _mesh = _CreateMeshGrid(stage, _meshId, n, scale * rotate * translate);
  _scene.AddGeometry(_meshId, _mesh);

  _AddAnimationSamples(stage, _meshId);

  // create rays
  _raysId = rootId.AppendChild(TfToken("rays"));
  _rays = (Curve*)_scene.AddGeometry(_raysId, Geometry::CURVE, GfMatrix4d(1.0));
  //_scene.InjectGeometry(stage, _raysId, _rays);

  _UpdateRays();
  _scene.MarkPrimDirty(_raysId, HdChangeTracker ::AllDirty);

  // create hits
  _hitsId = rootId.AppendChild(TfToken("hits"));
  _hits = (Points*)_scene.AddGeometry(_hitsId, Geometry::POINT, GfMatrix4d(1.0));
  //_scene.InjectGeometry(stage, _hitsId, _hits);

  _UpdateHits();
  UpdateExec(stage, Time::Get()->GetActiveTime());

}

void TestRaycast::UpdateExec(UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  _bvh.Update();
  _UpdateRays();
  _scene.MarkPrimDirty(_raysId, HdChangeTracker::DirtyPoints);
  _scene.MarkPrimDirty(_meshId, HdChangeTracker::AllDirty);

  _UpdateHits();
  _scene.MarkPrimDirty(_hitsId, HdChangeTracker::AllDirty);

}

void TestRaycast::TerminateExec(UsdStageRefPtr& stage)
{
  if (!stage) return;

  stage->RemovePrim(_meshId);
  //stage->RemovePrim(_bvhId);

}

JVR_NAMESPACE_CLOSE_SCOPE