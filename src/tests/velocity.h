#ifndef JVR_TEST_VELOCITY_H
#define JVR_TEST_VELOCITY_H

#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class Solver;
class Plane;
class Points;

class TestVelocity : public Execution {
public:
  TestVelocity() : Execution(){};
  ~TestVelocity(){};
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;
  
private:
  Solver*        _solver;
  Points*        _points0;
  pxr::SdfPath   _points0Id;
  Points*        _points1;
  pxr::SdfPath   _points1Id;
  
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H