#ifndef JVR_TEST_HAIR_H
#define JVR_TEST_HAIR_H

#include "../exec/execution.h"
#include "../pbd/solver.h"

JVR_NAMESPACE_OPEN_SCOPE

class Scene;
class TestHair : public Execution {
public:
  friend class Scene;
  TestHair() : Execution(){};
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;
private:
  pxr::TfHashMap<pxr::SdfPath, _Sources, pxr::SdfPath::Hash> _sourcesMap;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H