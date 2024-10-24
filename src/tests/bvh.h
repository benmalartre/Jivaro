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
  void InitExec(UsdStageRefPtr& stage) override;
  void UpdateExec(UsdStageRefPtr& stage, float time) override;
  void TerminateExec(UsdStageRefPtr& stage) override;
  void _TraverseStageFindingMeshes(UsdStageRefPtr& stage);
  void _AddAnimationSamples(UsdStageRefPtr& stage, SdfPath& path);

protected:
  void _UpdateRays();
  void _FindHits(size_t begin, size_t end, const GfVec3f* positions, GfVec3f* results, bool* hits);
  void _UpdateHits();

private:
  Mesh*                     _mesh;
  Curve*                    _rays;
  Points*                   _hits;
  BVH                       _bvh;
  Instancer*                _leaves;
  SdfPath              _meshId;
  SdfPath              _raysId;
  SdfPath              _hitsId;
  SdfPath              _bvhId;
  std::vector<Geometry*>    _meshes;
  std::vector<SdfPath> _meshesId;

  bool                      _method;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_BVH_H