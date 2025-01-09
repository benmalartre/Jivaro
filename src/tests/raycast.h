#ifndef JVR_TEST_RAYCAST_H
#define JVR_TEST_RAYCAST_H

#include "../acceleration/bvh.h"
#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE


class Curve;
class Mesh;
class Points;
class BVH;

class TestRaycast : public Execution {
public:
  friend class Scene;
  TestRaycast() : Execution(){};
  void InitExec(UsdStageRefPtr& stage) override;
  void UpdateExec(UsdStageRefPtr& stage, float time) override;
  void TerminateExec(UsdStageRefPtr& stage) override;

protected:
  void _UpdateRays() ;
  void _FindHits(size_t begin, size_t end, const GfVec3f* positions, 
    GfVec3f* results, bool* hits);
  void _UpdateHits();
  void _TraverseStageFindingMeshes(UsdStageRefPtr& stage);
  void _AddAnimationSamples(UsdStageRefPtr& stage, SdfPath& path);

private:
  std::vector<Geometry*>    _subjects; 
  Mesh*                     _mesh;
  Curve*                    _rays;
  Points*                   _hits;
  BVH                       _bvh;

  std::vector<SdfPath> _subjectsId;
  SdfPath              _meshId;
  SdfPath              _raysId;
  SdfPath              _hitsId;
  SdfPath              _bvhId;
  

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H