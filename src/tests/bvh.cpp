#include "../geometry/sampler.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/voxels.h"
#include "../geometry/points.h"
#include "../geometry/implicit.h"
#include "../geometry/instancer.h"
#include "../geometry/triangle.h"
#include "../geometry/utils.h"
#include "../geometry/scene.h"

#include "../pbd/particle.h"
#include "../pbd/force.h"
#include "../pbd/constraint.h"
#include "../pbd/collision.h"
#include "../pbd/solver.h"
#include "../utils/timer.h"

#include "../tests/bvh.h"

#include "../tests/utils.h"
#include "../app/application.h"
#include "../app/window.h"

JVR_NAMESPACE_OPEN_SCOPE

void TestBVH::_UpdateRays() 
{
  const double time = Time::Get()->GetActiveTime();
  const size_t numRays = _mesh->GetNumPoints();
  /*
  const size_t num = std::sqrt(numRays);
  VtArray<GfVec3f> deformed(numRays);

  const float speed = 0.25f;
  const float amplitude = 0.1f;
  const float frequency = 5.f;
  float xf, yf, zf;
  for(size_t z = 0; z < num; ++z) {
    for (size_t x = 0; x < num; ++x) {
      xf = (float)x/(float)num - 0.5;
      zf = (float)z/(float)num - 0.5;
      yf = GfSin(zf*frequency + time*speed) * amplitude;
      
      deformed[x + z * num] = GfVec3f(xf, yf, zf);
    }
  }
  _mesh->SetPositions(deformed);
  */

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
    points[r*2+1] = GfVec3f(matrix.Transform(positions[r] + normals[r]* 0.2));
    colors[r*2]   = GfVec3f(0.66f,0.66f,0.66f);
    colors[r*2+1] = GfVec3f(0.66f,0.66f,0.66f);
  }

  _rays->SetTopology(points, radiis, counts); 
  _rays->SetColors(colors);
}

// thread task
void TestBVH::_FindHits(size_t begin, size_t end, const GfVec3f* positions, 
  GfVec3f* results, bool* hits)
{
  if(_method == RAYCAST) {
    for (size_t index = begin; index < end; ++index) {
      GfRay ray(positions[index*2], positions[index*2+1] - positions[index*2]);
      double minDistance = DBL_MAX;
      Location hit;
      if (_bvh.Raycast(ray, &hit, DBL_MAX, &minDistance)) {
        const Geometry* collided = _bvh.GetGeometry(hit.GetGeometryIndex());
        const GfMatrix4d& matrix = collided->GetMatrix();
        switch (collided->GetType()) {
        case Geometry::MESH:
        {
          Mesh* mesh = (Mesh*)collided;
          Triangle* triangle = mesh->GetTriangle(hit.GetComponentIndex());

          results[index] = hit.ComputePosition(mesh->GetPositionsCPtr(), &triangle->vertices[0], 3, &matrix);
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
  } else if(_method == CLOSEST) {
    for (size_t index = begin; index < end; ++index) {
      Location hit;
      if (_bvh.Closest(positions[index*2], &hit, DBL_MAX)) {
        const Geometry* collided = _bvh.GetGeometry(hit.GetGeometryIndex());
        const GfMatrix4d& matrix = collided->GetMatrix();
        switch (collided->GetType()) {
        case Geometry::MESH:
        {
          Mesh* mesh = (Mesh*)collided;
          Triangle* triangle = mesh->GetTriangle(hit.GetComponentIndex());

          results[index] = hit.ComputePosition(mesh->GetPositionsCPtr(), &triangle->vertices[0], 3, &matrix);
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
}

// parallelize raycast
void TestBVH::_UpdateHits()
{
  const GfVec3f* positions = _rays->GetPositionsCPtr();
  size_t numRays = _rays->GetNumPoints() >> 1;

  VtArray<GfVec3f> points(numRays);
  for(size_t i = 0; i< numRays; ++i)points[i] = positions[i*2];
  VtArray<bool> hits(numRays, false);

  WorkParallelForN(_rays->GetNumCurves(),
    std::bind(&TestBVH::_FindHits, this, std::placeholders::_1, 
      std::placeholders::_2, positions, &points[0], &hits[0]));

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

void TestBVH::_TraverseStageFindingMeshes(UsdStageRefPtr& stage)
{
  UsdGeomXformCache xformCache(UsdTimeCode::Default());
  for (UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<UsdGeomMesh>()) {
      _meshes.push_back(new Mesh(UsdGeomMesh(prim), 
        xformCache.GetLocalToWorldTransform(prim)));
      _meshesId.push_back(prim.GetPath());
    }
}

void TestBVH::_AddAnimationSamples(UsdStageRefPtr& stage, SdfPath& path)
{
  UsdPrim prim = stage->GetPrimAtPath(path);
  if(prim.IsValid()) {
    UsdGeomXformable xformable(prim);
    GfRotation rotation(GfVec3f(0.f, 0.f, 1.f), 0.f);
    GfMatrix4d scale = GfMatrix4d().SetScale(GfVec3f(10.f, 10.f, 10.f));
    GfMatrix4d rotate = GfMatrix4d().SetRotate(rotation);
    GfMatrix4d translate1 = GfMatrix4d().SetTranslate(GfVec3f(0.f, 0.f, -10.f));
    GfMatrix4d translate2 = GfMatrix4d().SetTranslate(GfVec3f(0.f, 0.f, 10.f));
    auto op = xformable.GetTransformOp();
    op.Set(scale * rotate * translate1, 1);
    op.Set(scale * rotate * translate2, 101);
  }
}

void TestBVH::InitExec(UsdStageRefPtr& stage)
{
  if (!stage) return;

  _method = CLOSEST;
  UsdPrim rootPrim = stage->GetDefaultPrim();
  const SdfPath  rootId = rootPrim.GetPath();

  // find meshes in the scene
  std::vector<Geometry*> meshes;
  _TraverseStageFindingMeshes(stage);

  // create bvh
  if (_meshes.size()) {
    _bvh.Init(_meshes);

    for(size_t m = 0; m < _meshes.size();++m) {
      _scene.AddGeometry(_meshesId[m], _meshes[m]);
      //_meshes[m]->SetInputOnly();
    }

    _bvhId = rootId.AppendChild(TfToken("bvh"));
    _leaves = _SetupBVHInstancer(stage, _bvhId, &_bvh);
    _scene.AddGeometry(_bvhId, (Geometry*)_leaves );
    _scene.MarkPrimDirty(_bvhId, HdChangeTracker::DirtyInstancer);
  }
  
  // create mesh that will be source of rays
  GfRotation rotation(GfVec3f(0.f, 0.f, 1.f), 180.f);

  GfMatrix4d scale = GfMatrix4d().SetScale(GfVec3f(10.f, 10.f, 10.f));
  GfMatrix4d rotate = GfMatrix4d().SetRotate(rotation);
  GfMatrix4d translate = GfMatrix4d().SetTranslate(GfVec3f(0.f, 20.f, 0.f));

  const size_t n = 1024;
  _meshId = rootId.AppendChild(TfToken("emitter"));
  _mesh = _CreateMeshGrid(stage, _meshId, n, scale * rotate * translate);
  _scene.AddGeometry(_meshId, _mesh);

  //_AddAnimationSamples(stage, _meshId);

  // create rays
  _raysId = rootId.AppendChild(TfToken("rays"));
  _rays = (Curve*)_scene.AddGeometry(_raysId, Geometry::CURVE, GfMatrix4d(1.0));

  _UpdateRays();
  _scene.MarkPrimDirty(_raysId, HdChangeTracker ::AllDirty);

  // create hits
  _hitsId = rootId.AppendChild(TfToken("hits"));
  _hits = (Points*)_scene.AddGeometry(_hitsId, Geometry::POINT, GfMatrix4d(1.0));

  _UpdateHits();

  UpdateExec(stage, 1);

}

void TestBVH::UpdateExec(UsdStageRefPtr& stage, float time)
{
  uint64_t startT = ArchGetTickTime();
  _scene.Sync(stage, time);
  
  if (_meshes.size()) {
    _bvh.Update();
    _UpdateBVHInstancer(stage, _bvhId, &_bvh, time);
    _scene.MarkPrimDirty(_bvhId, HdChangeTracker::DirtyInstancer);
  }

  _UpdateRays();
  _scene.MarkPrimDirty(_raysId, 
    HdChangeTracker::DirtyPoints);

  _scene.MarkPrimDirty(_meshId, 
    HdChangeTracker::DirtyPoints|
    HdChangeTracker::DirtyTransform);

  _UpdateHits();
  _scene.MarkPrimDirty(_hitsId, HdChangeTracker::AllDirty);
  double elapsedT = (double)(ArchGetTickTime() - startT)*1e-6;
  size_t numRays = _mesh->GetNumPoints();
  Window* mainWindow = WindowRegistry::Get()->GetWindow(0);
  mainWindow->SetViewportMessage("launch " + std::to_string(numRays) + " took " +std::to_string(elapsedT) + " seconds.");
}

void TestBVH::TerminateExec(UsdStageRefPtr& stage)
{
  if (!stage) return;

  stage->RemovePrim(_meshId);
  stage->RemovePrim(_bvhId);

}

JVR_NAMESPACE_CLOSE_SCOPE