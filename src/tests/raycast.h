#ifndef JVR_TEST_RAYCAST_H
#define JVR_TEST_RAYCAST_H

#include "../exec/execution.h"
#include "../pbd/solver.h"

JVR_NAMESPACE_OPEN_SCOPE

class Curve;
class Mesh;
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
  pxr::SdfPath                                                _meshId;
  pxr::SdfPath                                                _raysId;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H