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

#include "../tests/raycast.h"

#include "../app/application.h"

#include "../tests/utils.h"

JVR_NAMESPACE_OPEN_SCOPE

class Curve;
class Mesh;
class Points;
class BVH;

class TestRaycast : public Execution {
public:
  friend class Scene;
  TestRaycast() : Execution(){};
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;
private:
  Mesh*                                                       _mesh;
  Curve*                                                      _rays;
  Points*                                                     _hits;
  BVH                                                         _bvh;
  pxr::SdfPath                                                _meshId;
  pxr::SdfPath                                                _raysId;
  pxr::SdfPath                                                _hitsId;
  pxr::SdfPath                                                _bvhId;

};

Execution* CreateTestRaycast()
{
  return new TestRaycast();
}


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

void _TraverseStageFindingMeshes(pxr::UsdStageRefPtr& stage, std::vector<Geometry*>& meshes)
{
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  for (pxr::UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      meshes.push_back(new Mesh(pxr::UsdGeomMesh(prim), xformCache.GetLocalToWorldTransform(prim)));
    }
      
}

void TestRaycast::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  const pxr::SdfPath  rootId = rootPrim.GetPath();

  // find meshes in the scene
  std::vector<Geometry*> meshes;
  _TraverseStageFindingMeshes(stage, meshes);

  // create bvh
  if (meshes.size()) {
    std::cout << "num meshes " << meshes.size() << std::endl;
    _bvh.Init(meshes);
    _bvhId = rootId.AppendChild(pxr::TfToken("bvh"));
    std::cout << "setup bvh instancer" << std::endl;
    //_SetupBVHInstancer(stage, _bvhId, &_bvh);

  }
  
  // create mesh that will be source of rays
  pxr::GfQuatf rotation(180.f * DEGREES_TO_RADIANS, pxr::GfVec3f(0.f, 0.f, 1.f));
  rotation.Normalize();

  pxr::GfMatrix4d scale = pxr::GfMatrix4d(1.f).SetScale(pxr::GfVec3f(10.f, 10.f, 10.f));
  pxr::GfMatrix4d rotate = pxr::GfMatrix4d(1.f).SetRotate(rotation);
  pxr::GfMatrix4d translate = pxr::GfMatrix4d(1.f).SetTranslate(pxr::GfVec3f(0.f, 10.f, 0.f));

  _meshId = rootId.AppendChild(pxr::TfToken("emitter"));
  _mesh = _GenerateMeshGrid(stage, _meshId, 32, 32, scale * rotate * translate);
  _scene.AddGeometry(_meshId, _mesh);

  // create rays
  _raysId = rootId.AppendChild(pxr::TfToken("rays"));
  _rays = (Curve*)_scene.AddGeometry(_raysId, Geometry::CURVE, pxr::GfMatrix4d(1.0));

  _UpdateRays(_mesh, _rays);

  // create hits
  _hitsId = rootId.AppendChild(pxr::TfToken("hits"));
  _hits = (Points*)_scene.AddGeometry(_hitsId, Geometry::POINT, pxr::GfMatrix4d(1.0));

  _scene.Update(stage, 1);

  Scene::_Prim* prim = _scene.GetPrim(_raysId);
  prim->bits = pxr::HdChangeTracker::AllDirty;  

}

void TestRaycast::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Update(stage, time);
  _UpdateRays(_mesh, _rays);
  Scene::_Prim* prim = _scene.GetPrim(_raysId);
  prim->bits = pxr::HdChangeTracker::AllDirty;

  pxr::VtArray<pxr::GfVec3f> result;
  const pxr::GfVec3f* positions = _rays->GetPositionsCPtr();
  for (size_t c= 0; _rays->GetNumCurves(); ++c) {
    pxr::GfRay ray(positions[c*2], positions[c*2+1] - positions[c*2]);
    double minDistance = DBL_MAX;
    Location hit;
    const pxr::GfVec3f* points = _mesh->GetPositionsCPtr();
    if (_bvh.Raycast(points, ray, &hit, DBL_MAX, &minDistance)) {
      Geometry* collided = _bvh.GetGeometry(hit.GetGeometryIndex());
      result.push_back(hit.GetPosition(collided));
    }
  }

  _hits->SetPositions(result);
  pxr::VtArray<float> radiis(result.size(), 0.1);
  _hits->SetRadii(radiis);
  prim = _scene.GetPrim(_hitsId);
  prim->bits = pxr::HdChangeTracker::DirtyTopology;

}

void TestRaycast::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  stage->RemovePrim(_meshId);
  stage->RemovePrim(_bvhId);

}

JVR_NAMESPACE_CLOSE_SCOPE