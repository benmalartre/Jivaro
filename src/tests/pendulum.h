#ifndef JVR_TEST_PENDULUM_H
#define JVR_TEST_PENDULUM_H

#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class Solver;
class Plane;
class Points;

class TestPendulum : public Execution {
public:
  TestPendulum() : Execution(){};
  ~TestPendulum(){};
  void InitExec(UsdStageRefPtr& stage) override;
  void UpdateExec(UsdStageRefPtr& stage, float time) override;
  void TerminateExec(UsdStageRefPtr& stage) override;
  
private:
  Solver*        _solver;
  SdfPath   _solverId;
  Plane*         _ground;
  SdfPath   _groundId;
  Points*        _points;
  SdfPath   _pointsId;
 
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PENDULUM_H