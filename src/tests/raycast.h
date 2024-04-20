#ifndef JVR_TEST_PBD_H
#define JVR_TEST_PBD_H

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
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;

protected:
  void _UpdateRays() ;
  void _FindHits(size_t begin, size_t end, const pxr::GfVec3f* positions, 
    pxr::GfVec3f* results, bool* hits);
  void _UpdateHits();
  void _TraverseStageFindingMeshes(pxr::UsdStageRefPtr& stage);
  void _AddAnimationSamples(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path);

private:
  std::vector<Geometry*>    _subjects; 
  Mesh*                     _mesh;
  Curve*                    _rays;
  Points*                   _hits;
  BVH                       _bvh;

  std::vector<pxr::SdfPath> _subjectsId;
  pxr::SdfPath              _meshId;
  pxr::SdfPath              _raysId;
  pxr::SdfPath              _hitsId;
  pxr::SdfPath              _bvhId;
  

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H