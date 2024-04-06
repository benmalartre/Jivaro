#ifndef JVR_TEST_PBD_H
#define JVR_TEST_PBD_H

#include "../exec/execution.h"
#include "../pbd/solver.h"

JVR_NAMESPACE_OPEN_SCOPE

class Scene;
class TestPBD : public Execution {
public:
  friend class Scene;
  TestPBD(Scene* scene) : Execution(scene){};
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, double time, double startTime=1.0) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;
private:
  Solver*                                                    _solver;
  pxr::TfHashMap<pxr::SdfPath, _Sources, pxr::SdfPath::Hash> _sourcesMap;
  pxr::TfHashMap<pxr::SdfPath, Body*, pxr::SdfPath::Hash>    _bodyMap;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H