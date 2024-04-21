#ifndef JVR_TEST_VELOCITY_H
#define JVR_TEST_VELOCITY_H

#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class Curve;
class Sphere;
class TestVelocity : public Execution {
public:
  friend class Scene;
  TestVelocity() : Execution(){};
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;
  
private:
  Curve*          _curve;
  Sphere*            _xfo;
  pxr::SdfPath    _curveId;
  pxr::SdfPath    _xfoId;

};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_VELOCITY_H