#ifndef JVR_TEST_INSTANCER_H
#define JVR_TEST_INSTANCER_H

#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class Mesh;
class Instancer;

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

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_BVH_H