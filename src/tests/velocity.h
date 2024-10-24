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
  void InitExec(UsdStageRefPtr& stage) override;
  void UpdateExec(UsdStageRefPtr& stage, float time) override;
  void TerminateExec(UsdStageRefPtr& stage) override;
  
private:
  Solver*        _solver;
  SdfPath   _solverId;
  Plane*         _ground;
  SdfPath   _groundId;
  Points*        _points0;
  SdfPath   _points0Id;
  Points*        _points1;
  SdfPath   _points1Id;
  
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H