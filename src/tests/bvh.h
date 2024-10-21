#ifndef JVR_TEST_BVH_H
#define JVR_TEST_BVH_H

#include "../acceleration/bvh.h"
#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class Curve;
class Mesh;
class Points;
class BVH;
class Instancer;
class Geometry;

class TestBVH : public Execution {
public:
  enum Method{
    RAYCAST,
    CLOSEST
  };
  
  friend class Scene;
  TestBVH() : Execution(){};
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
  Mesh*                     _mesh;
  Curve*                    _rays;
  Points*                   _hits;
  BVH                       _bvh;
  Instancer*                _leaves;
  pxr::SdfPath              _meshId;
  pxr::SdfPath              _raysId;
  pxr::SdfPath              _hitsId;
  pxr::SdfPath              _bvhId;
  std::vector<Geometry*>    _meshes;
  std::vector<pxr::SdfPath> _meshesId;

  bool                      _method;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_BVH_H