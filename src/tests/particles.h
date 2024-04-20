#ifndef JVR_TEST_PARTICLES_H
#define JVR_TEST_PARTICLES_H

#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class Solver;
class Plane;
class Points;

class TestParticles : public Execution {
public:
  friend class Scene;
  TestParticles() : Execution(){};
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;
private:
  Solver*                                                    _solver;
  Plane*                                                     _ground;
  Points*                                                    _points;
  pxr::SdfPath                                               _groundId;
  pxr::SdfPath                                               _solverId;
  pxr::SdfPath                                               _pointsId;

};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H