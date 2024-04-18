#ifndef JVR_TEST_PBD_H
#define JVR_TEST_PBD_H

#include "../exec/execution.h"
#include "../pbd/solver.h"

JVR_NAMESPACE_OPEN_SCOPE

class Scene;
class Plane;
class TestPBD : public Execution {
public:
  friend class Scene;
  TestPBD() : Execution(){};
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;
private:
  Solver*                                                    _solver;
  Plane*                                                     _ground;
  pxr::TfHashMap<pxr::SdfPath, _Sources, pxr::SdfPath::Hash> _sourcesMap;
  pxr::TfHashMap<pxr::SdfPath, Body*, pxr::SdfPath::Hash>    _bodyMap;
  pxr::SdfPath                                               _groundId;
  pxr::SdfPath                                               _solverId;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H