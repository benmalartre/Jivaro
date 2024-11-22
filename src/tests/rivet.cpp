#include "../utils/timer.h"
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

#include "../tests/grid.h"

#include "../tests/utils.h"
#include "../app/application.h"

JVR_NAMESPACE_OPEN_SCOPE



void TestRivet::_UpdateCurves() 
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
  const GfMatrix4d& matrix = _mesh->GetMatrix();

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
    points[r*2]   = matrix.Transform(positions[r]);
    points[r*2+1] = matrix.Transform(positions[r] + normals[r] * 0.2);
    colors[r*2]   = GfVec3f(0.66f,0.66f,0.66f);
    colors[r*2+1] = GfVec3f(0.66f,0.66f,0.66f);
  }

  _rays->SetTopology(points, radiis, counts); 
  _rays->SetColors(colors);
}

// thread task
void TestRivet::_FindHits(size_t begin, size_t end, const GfVec3f* positions, 
  GfVec3f* results, bool* hits, Intersector* intersector)
{
  for (size_t index = begin; index < end; ++index) {
    GfRay ray(positions[index*2], positions[index*2+1] - positions[index*2]);
    double minDistance = DBL_MAX;
    Location hit;
    if (intersector->Raycast(ray, &hit, DBL_MAX, &minDistance)) {
      const Geometry* collided = intersector->GetGeometry(hit.GetGeometryIndex());
      const GfMatrix4d& matrix = collided->GetMatrix();
      switch (collided->GetType()) {
        case Geometry::MESH:
        {
          Mesh* mesh = (Mesh*)collided;
          Triangle* triangle = mesh->GetTriangle(hit.GetComponentIndex());
          results[index] = hit.ComputePosition(mesh->GetPositionsCPtr(), &triangle->vertices[0], 3, &matrix);
          break;
        }
      }
      hits[index] = true;
    } else {
      hits[index] = false;
    }
  }
}

// parallelize raycast
void TestRivet::_UpdateHits()
{
  const GfVec3f* positions = _rays->GetPositionsCPtr();  
  size_t numRays = _rays->GetNumPoints() >> 1;

  std::cout << "num rays : " << numRays << std::endl;

  VtArray<GfVec3f> points(numRays * 2);
  VtArray<bool> hits(numRays * 2, false);


  uint64_t startT = CurrentTime();
  WorkParallelForN(_rays->GetNumCurves(),
    std::bind(&TestRivet::_FindHits, this, std::placeholders::_1, 
      std::placeholders::_2, positions, &points[0], &hits[0], &_grid));
  uint64_t bvhRaycastT = CurrentTime() - startT;
  std::cout << "raycast bvh : " << ((double)bvhRaycastT * 1e-6) << " seconds" << std::endl;

  startT = CurrentTime();
  WorkParallelForN(_rays->GetNumCurves(),
    std::bind(&TestRivet::_FindHits, this, std::placeholders::_1, 
      std::placeholders::_2, positions, &points[numRays], &hits[numRays], &_bvh));
  uint64_t gridRaycastT = CurrentTime() - startT;
  std::cout << "raycast grid : " << ((double)gridRaycastT * 1e-6) << " seconds" << std::endl;



  // need accumulate result
  VtArray<GfVec3f> result;
  VtArray<GfVec3f> colors;
  VtArray<float> widths;
  result.reserve(numRays * 2);
  colors.reserve(numRays * 2);
  widths.reserve(numRays * 2);
  size_t bvhHits = 0, gridHits = 0;
  for(size_t r = 0; r < numRays; ++r) {
    if(hits[r]) {
      result.push_back(points[r]+GfVec3f(RANDOM_0_1*0.05));
      colors.push_back({1.f, 0.f, 0.f});
      widths.push_back( 0.25f);
      bvhHits++;
    }
    if(hits[r+numRays]) {
      result.push_back(points[r+numRays]+GfVec3f(RANDOM_0_1*0.05));
      colors.push_back({0.f, 1.f, 0.f});
      widths.push_back( 0.25f);
      gridHits++;
    }
  }
  
  std::cout << "bvh hits : " << bvhHits << std::endl;
  std::cout << "grid hits : " << gridHits << std::endl;

  _hits->SetPositions(result);
  _hits->SetWidths(widths);
  _hits->SetColors(colors);

  _CompareHits(hits, points);

}

bool TestRivet::_CompareHits(const VtArray<bool>& hits, const  VtArray<GfVec3f> points)
{
  size_t numRays = hits.size() / 2;
  size_t cntHitError = 0;
  size_t cntCoordError = 0;
  for(size_t r = 0; r < numRays; ++r) {
    if(hits[r] != hits[r+numRays]) {
      cntHitError++;
    } else {
      if((points[r]-points[r+numRays]).GetLength() > 0.001f){
         cntCoordError++;
      }
    }
  }

  std::cout << "num hit error : " << cntHitError << std::endl;
  std::cout << "num coord error : " << cntCoordError << std::endl;

  return cntHitError == 0 && cntCoordError == 0;
}

void TestRivet::_TraverseStageFindingMeshes(UsdStageRefPtr& stage)
{
  UsdGeomXformCache xformCache(UsdTimeCode::Default());
  for (UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<UsdGeomMesh>()) {
      _meshes.push_back(new Mesh(UsdGeomMesh(prim), 
        xformCache.GetLocalToWorldTransform(prim)));
      _meshesId.push_back(prim.GetPath());
    }
      
}

void TestRivet::_AddAnimationSamples(UsdStageRefPtr& stage, SdfPath& path)
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

void TestRivet::InitExec(UsdStageRefPtr& stage)
{
  if (!stage) return;

  UsdPrim rootPrim = stage->GetDefaultPrim();
  const SdfPath  rootId = rootPrim.GetPath();

  // find meshes in the scene
  _TraverseStageFindingMeshes(stage);

  // create bvh
  if (_meshes.size()) {

    for(size_t m = 0; m < _meshes.size();++m) {
      _scene.AddGeometry(_meshesId[m], _meshes[m]);
      //_meshes[m]->SetInputOnly();
    }

    uint64_t startT = CurrentTime();
    _bvh.Init(_meshes);
    uint64_t bvhBuildT = CurrentTime() - startT;
    std::cout << "build bvh : " << ((double)bvhBuildT * 1e-6) << " seconds" << std::endl;


    startT = CurrentTime();
    _grid.Init(_meshes);
    uint64_t gridBuildT = CurrentTime() - startT;
    std::cout << "build grid : " << ((double)gridBuildT * 1e-6) << " seconds" << std::endl;
    

    _gridId = rootId.AppendChild(TfToken("grid"));
    _leaves = _SetupGridInstancer(stage, _gridId, &_grid);
    _scene.AddGeometry(_gridId, (Geometry*)_leaves );
    _scene.MarkPrimDirty(_gridId, HdChangeTracker::AllDirty);
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

  _AddAnimationSamples(stage, _meshId);

  // create rays
  _raysId = rootId.AppendChild(TfToken("rays"));
  _rays = (Curve*)_scene.AddGeometry(_raysId, Geometry::CURVE, GfMatrix4d(1.0));

  _UpdateRays();
  _scene.MarkPrimDirty(_raysId, HdChangeTracker ::AllDirty);

  // create hits
  _hitsId = rootId.AppendChild(TfToken("hits"));
  _hits = (Points*)_scene.AddGeometry(_hitsId, Geometry::POINT, GfMatrix4d(1.0));

  UpdateExec(stage, 1);

}

void TestRivet::UpdateExec(UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  
  if (_meshes.size()) {
    uint64_t startT = CurrentTime();
    _bvh.Update();
    uint64_t bvhUpdateT = CurrentTime() - startT;
    std::cout << "update bvh : " << ((double)bvhUpdateT * 1e-6) << " seconds" << std::endl;

     startT = CurrentTime();
    _grid.Update();
    uint64_t gridUpdateT = CurrentTime() - startT;
    std::cout << "update grid : " << ((double)gridUpdateT * 1e-6) << " seconds" << std::endl;
    _UpdateGridInstancer(stage, _gridId, &_grid, time);
    _scene.MarkPrimDirty(_gridId, HdChangeTracker::DirtyInstancer);
  }

  _UpdateRays();
  _scene.MarkPrimDirty(_raysId, 
    HdChangeTracker::DirtyPoints);

  _scene.MarkPrimDirty(_meshId, 
    HdChangeTracker::DirtyPoints|
    HdChangeTracker::DirtyTransform);

  _UpdateHits();
  _scene.MarkPrimDirty(_hitsId, HdChangeTracker::AllDirty);
}

void TestRivet::TerminateExec(UsdStageRefPtr& stage)
{
  if (!stage) return;

  stage->RemovePrim(_meshId);
  stage->RemovePrim(_gridId);

}

JVR_NAMESPACE_CLOSE_SCOPE