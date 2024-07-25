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
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;
  
private:
  Solver*        _solver;
  pxr::SdfPath   _solverId;
  Plane*         _ground;
  pxr::SdfPath   _groundId;
  Points*        _points;
  pxr::SdfPath   _pointsId;
 
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PENDULUM_H