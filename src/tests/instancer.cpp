#include "../geometry/instancer.h"
#include "../tests/instancer.h"
#include "../tests/utils.h"


JVR_NAMESPACE_OPEN_SCOPE

class Curve;
class Mesh;
class Points;
class BVH;


class TestInstancer : public Execution {
public:
  friend class Scene;
  TestInstancer() : Execution(){};
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;
  void _TraverseStageFindingMeshes(pxr::UsdStageRefPtr& stage);
  void _AddAnimationSamples(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path);

protected:
  void _UpdateRays();
  void _FindHits(size_t begin, size_t end, const pxr::GfVec3f* positions, pxr::GfVec3f* results, bool* hits);
  void _UpdateHits();

private:
  Mesh*                     _proto1;
  Mesh*                     _proto2;
  Instancer*                _instancer;
  pxr::SdfPath              _proto1Id;
  pxr::SdfPath              _proto2Id;
  pxr::SdfPath              _instancerId;

};

Execution* CreateTestInstancer()
{
  return new TestInstancer();
}

void TestInstancer::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  const pxr::SdfPath  rootId = rootPrim.GetPath();

  _proto1Id = rootId.AppendChild(pxr::TfToken("proto1"));
  _proto2Id = rootId.AppendChild(pxr::TfToken("proto2"));
  _instancerId = root.AppendChild(pxr::TfToken("instancer"));

  
  // find meshes in the scene
  std::vector<Geometry*> meshes;
  _TraverseStageFindingMeshes(stage);

  // create bvh
  if (_meshes.size()) {
    _bvh.Init(_meshes);

    for(size_t m = 0; m < _meshes.size();++m) {
      _scene.AddGeometry(_meshesId[m], _meshes[m]);
      _meshes[m]->SetInputOnly();
    }

    _bvhId = rootId.AppendChild(pxr::TfToken("bvh"));
    _leaves = _SetupBVHInstancer(stage, _bvhId, &_bvh);
    _scene.AddGeometry(_bvhId, (Geometry*)_leaves );
    _scene.MarkPrimDirty(_bvhId, pxr::HdChangeTracker::DirtyInstancer);
  }
  
  // create mesh that will be source of rays
  pxr::GfRotation rotation(pxr::GfVec3f(0.f, 0.f, 1.f), 180.f);

  pxr::GfMatrix4d scale = pxr::GfMatrix4d(1.f).SetScale(pxr::GfVec3f(10.f, 10.f, 10.f));
  pxr::GfMatrix4d rotate = pxr::GfMatrix4d(1.f).SetRotate(rotation);
  pxr::GfMatrix4d translate = pxr::GfMatrix4d(1.f).SetTranslate(pxr::GfVec3f(0.f, 20.f, 0.f));

  const size_t n = 32;
  _meshId = rootId.AppendChild(pxr::TfToken("emitter"));
  _mesh = _GenerateMeshGrid(stage, _meshId, n, scale * rotate * translate);
  _scene.AddGeometry(_meshId, _mesh);

  //_AddAnimationSamples(stage, _meshId);

  // create rays
  _raysId = rootId.AppendChild(pxr::TfToken("rays"));
  _rays = (Curve*)_scene.AddGeometry(_raysId, Geometry::CURVE, pxr::GfMatrix4d(1.0));

  _UpdateRays();
  _scene.MarkPrimDirty(_raysId, pxr::HdChangeTracker ::AllDirty);

  // create hits
  _hitsId = rootId.AppendChild(pxr::TfToken("hits"));
  _hits = (Points*)_scene.AddGeometry(_hitsId, Geometry::POINT, pxr::GfMatrix4d(1.0));

  _UpdateHits();

}

void TestInstancer::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  
  if (_meshes.size()) {
    _bvh.Update();
    _UpdateBVHInstancer(stage, &_bvh, _leaves);
    _scene.MarkPrimDirty(_bvhId, pxr::HdChangeTracker::DirtyInstancer);
  }

  _UpdateRays();
  _scene.MarkPrimDirty(_raysId, pxr::HdChangeTracker::DirtyPoints);
  _scene.MarkPrimDirty(_meshId, pxr::HdChangeTracker::AllDirty);

  _UpdateHits();
  _scene.MarkPrimDirty(_hitsId, pxr::HdChangeTracker::AllDirty);
}

void TestInstancer::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  stage->RemovePrim(_meshId);
  stage->RemovePrim(_bvhId);

}

JVR_NAMESPACE_CLOSE_SCOPE