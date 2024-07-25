#ifndef JVR_TEST_POINTS_H
#define JVR_TEST_POINTS_H

#include "../acceleration/bvh.h"
#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class Solver;
class Plane;
class Points;

class TestPoints : public Execution {
public:
  TestPoints() : Execution(){};
  ~TestPoints(){};
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;
  
private:
  Points*                                                    _points;
  pxr::SdfPath                                               _pointsId;
  
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H