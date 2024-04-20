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
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;

private:
  std::vector<Mesh*>        _protos;
  Mesh*                     _ground;
  Instancer*                _instancer;
  std::vector<pxr::SdfPath> _protosId;
  pxr::SdfPath              _groundId;
  pxr::SdfPath              _instancerId;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_BVH_H