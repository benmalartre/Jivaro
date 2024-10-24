#ifndef JVR_TEST_INSTANCER_H
#define JVR_TEST_INSTANCER_H

#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class Mesh;
class Instancer;

class TestInstancer : public Execution {
public:
  friend class Scene;
  TestInstancer() : Execution(){};
  void InitExec(UsdStageRefPtr& stage) override;
  void UpdateExec(UsdStageRefPtr& stage, float time) override;
  void TerminateExec(UsdStageRefPtr& stage) override;

private:
  std::vector<Mesh*>        _protos;
  Mesh*                     _ground;
  Instancer*                _instancer;
  std::vector<SdfPath> _protosId;
  SdfPath              _groundId;
  SdfPath              _instancerId;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_BVH_H