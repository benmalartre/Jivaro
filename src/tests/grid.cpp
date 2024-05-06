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



void TestGrid::_UpdateRays() 
{
  const double time = Time::Get()->GetActiveTime();
  const size_t numRays = _mesh->GetNumPoints();
  /*
  const size_t num = std::sqrt(numRays);
  pxr::VtArray<pxr::GfVec3f> deformed(numRays);

  const float speed = 0.25f;
  const float amplitude = 0.1f;
  const float frequency = 5.f;
  float xf, yf, zf;
  for(size_t z = 0; z < num; ++z) {
    for (size_t x = 0; x < num; ++x) {
      xf = (float)x/(float)num - 0.5;
      zf = (float)z/(float)num - 0.5;
      yf = pxr::GfSin(zf*frequency + time*speed) * amplitude;
      
      deformed[x + z * num] = pxr::GfVec3f(xf, yf, zf);
    }
  }
  _mesh->SetPositions(deformed);
  */

  const pxr::GfVec3f* positions = _mesh->GetPositionsCPtr();
  const pxr::GfVec3f* normals = _mesh->GetNormalsCPtr();
  const pxr::GfMatrix4d& matrix = _mesh->GetMatrix();

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
    points[r*2+1] = matrix.Transform(positions[r] + normals[r] * 0.2);
    colors[r*2]   = pxr::GfVec3f(0.66f,0.66f,0.66f);
    colors[r*2+1] = pxr::GfVec3f(0.66f,0.66f,0.66f);
  }

  _rays->SetTopology(points, radiis, counts); 
  _rays->SetColors(colors);
}

// thread task
void TestGrid::_FindHits(size_t begin, size_t end, const pxr::GfVec3f* positions, 
  pxr::GfVec3f* results, bool* hits, Intersector* intersector)
{
  for (size_t index = begin; index < end; ++index) {
    pxr::GfRay ray(positions[index*2], positions[index*2+1] - positions[index*2]);
    double minDistance = DBL_MAX;
    Location hit;
    if (intersector->Raycast(ray, &hit, DBL_MAX, &minDistance)) {
      Geometry* collided = intersector->GetGeometry(hit.GetGeometryIndex());
      const pxr::GfMatrix4d& matrix = collided->GetMatrix();
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
void TestGrid::_UpdateHits()
{
  const pxr::GfVec3f* positions = _rays->GetPositionsCPtr();  
  size_t numRays = _rays->GetNumPoints() >> 1;

  pxr::VtArray<pxr::GfVec3f> points(numRays * 2);
  pxr::VtArray<bool> hits(numRays * 2, false);


  pxr::WorkParallelForN(_rays->GetNumCurves(),
    std::bind(&TestGrid::_FindHits, this, std::placeholders::_1, 
      std::placeholders::_2, positions, &points[0], &hits[0], &_grid));

  pxr::WorkParallelForN(_rays->GetNumCurves(),
    std::bind(&TestGrid::_FindHits, this, std::placeholders::_1, 
      std::placeholders::_2, positions, &points[numRays], &hits[numRays], &_bvh));

  // need accumulate result
  pxr::VtArray<pxr::GfVec3f> result;
  pxr::VtArray<pxr::GfVec3f> colors;
  pxr::VtArray<float> widths;
  result.reserve(numRays * 2);
  colors.reserve(numRays * 2);
  widths.reserve(numRays * 2);
  size_t bvhHits = 0, gridHits = 0;
  for(size_t r = 0; r < numRays; ++r) {
    if(hits[r]) {
      result.push_back(points[r]+pxr::GfVec3f(RANDOM_0_1*0.05));
      colors.push_back({1.f, 0.f, 0.f});
      widths.push_back( 0.25f);
      bvhHits++;
    }
    if(hits[r+numRays]) {
      result.push_back(points[r+numRays]+pxr::GfVec3f(RANDOM_0_1*0.05));
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

bool TestGrid::_CompareHits(const pxr::VtArray<bool>& hits, const  pxr::VtArray<pxr::GfVec3f> points)
{
  size_t numRays = hits.size() / 2;
  size_t cntError = 0;
  for(size_t r = 0; r < numRays; ++r) {
    if(hits[r] != hits[r+numRays]) {
      cntError++;
      std::cout << r << "hit diverge between grid and bvh" << std::endl;
    } else {
      if((points[r]-points[r+numRays]).GetLength() > 0.001f){
         cntError++;
        std::cout << r << "coordinates diverge between grid and bvh" << std::endl;
        std::cout << points[r] << std::endl;
        std::cout << points[r+numRays] << std::endl;
      }
    }
  }
  return cntError == 0;
}

void TestGrid::_TraverseStageFindingMeshes(pxr::UsdStageRefPtr& stage)
{
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  for (pxr::UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      _meshes.push_back(new Mesh(pxr::UsdGeomMesh(prim), 
        xformCache.GetLocalToWorldTransform(prim)));
      _meshesId.push_back(prim.GetPath());
    }
      
}

void TestGrid::_AddAnimationSamples(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path)
{
  pxr::UsdPrim prim = stage->GetPrimAtPath(path);
  if(prim.IsValid()) {
    pxr::UsdGeomXformable xformable(prim);
    pxr::GfRotation rotation(pxr::GfVec3f(0.f, 0.f, 1.f), 0.f);
    pxr::GfMatrix4d scale = pxr::GfMatrix4d().SetScale(pxr::GfVec3f(10.f, 10.f, 10.f));
    pxr::GfMatrix4d rotate = pxr::GfMatrix4d().SetRotate(rotation);
    pxr::GfMatrix4d translate1 = pxr::GfMatrix4d().SetTranslate(pxr::GfVec3f(0.f, 0.f, -10.f));
    pxr::GfMatrix4d translate2 = pxr::GfMatrix4d().SetTranslate(pxr::GfVec3f(0.f, 0.f, 10.f));
    auto op = xformable.GetTransformOp();
    op.Set(scale * rotate * translate1, 1);
    op.Set(scale * rotate * translate2, 101);
  }
}

void TestGrid::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  const pxr::SdfPath  rootId = rootPrim.GetPath();

  // find meshes in the scene
  _TraverseStageFindingMeshes(stage);

  // create bvh
  if (_meshes.size()) {

    for(size_t m = 0; m < _meshes.size();++m) {
      _scene.AddGeometry(_meshesId[m], _meshes[m]);
      _meshes[m]->SetInputOnly();
    }

    _grid.Init(_meshes);
    _bvh.Init(_meshes);
    


    _gridId = rootId.AppendChild(pxr::TfToken("grid"));
    //_leaves = _SetupGridInstancer(stage, _gridId, &_grid);
    //_scene.AddGeometry(_gridId, (Geometry*)_leaves );
    //_scene.MarkPrimDirty(_gridId, pxr::HdChangeTracker::DirtyInstancer);
  }
  
  // create mesh that will be source of rays
  pxr::GfRotation rotation(pxr::GfVec3f(0.f, 0.f, 1.f), 180.f);

  pxr::GfMatrix4d scale = pxr::GfMatrix4d().SetScale(pxr::GfVec3f(10.f, 10.f, 10.f));
  pxr::GfMatrix4d rotate = pxr::GfMatrix4d().SetRotate(rotation);
  pxr::GfMatrix4d translate = pxr::GfMatrix4d().SetTranslate(pxr::GfVec3f(0.f, 20.f, 0.f));

  const size_t n = 8;
  _meshId = rootId.AppendChild(pxr::TfToken("emitter"));
  _mesh = _GenerateMeshGrid(stage, _meshId, n, scale * rotate * translate);
  _scene.AddGeometry(_meshId, _mesh);

  _AddAnimationSamples(stage, _meshId);

  // create rays
  _raysId = rootId.AppendChild(pxr::TfToken("rays"));
  _rays = (Curve*)_scene.AddGeometry(_raysId, Geometry::CURVE, pxr::GfMatrix4d(1.0));

  _UpdateRays();
  _scene.MarkPrimDirty(_raysId, pxr::HdChangeTracker ::AllDirty);

  // create hits
  _hitsId = rootId.AppendChild(pxr::TfToken("hits"));
  _hits = (Points*)_scene.AddGeometry(_hitsId, Geometry::POINT, pxr::GfMatrix4d(1.0));

  UpdateExec(stage, 1);

}

void TestGrid::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  
  if (_meshes.size()) {
    _grid.Update();
    _bvh.Update();
    _UpdateGridInstancer(stage, _gridId, &_grid, time);
    _scene.MarkPrimDirty(_gridId, pxr::HdChangeTracker::DirtyInstancer);
  }

  _UpdateRays();
  _scene.MarkPrimDirty(_raysId, 
    pxr::HdChangeTracker::DirtyPoints);

  _scene.MarkPrimDirty(_meshId, 
    pxr::HdChangeTracker::DirtyPoints|
    pxr::HdChangeTracker::DirtyTransform);

  _UpdateHits();
  _scene.MarkPrimDirty(_hitsId, pxr::HdChangeTracker::AllDirty);
}

void TestGrid::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  stage->RemovePrim(_meshId);
  stage->RemovePrim(_gridId);

}

JVR_NAMESPACE_CLOSE_SCOPE