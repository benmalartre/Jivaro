#ifndef JVR_TEST_PBD_H
#define JVR_TEST_PBD_H

#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class Solver;
class Plane;
class Body;

class TestPBD : public Execution {
public:
  friend class Scene;
  TestPBD() : Execution(){};
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;

protected: 
  void _TraverseStageFindingMeshes(pxr::UsdStageRefPtr& stage);
  void _AddAnimationSamples(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path);

private:
  Solver*                   _solver;
  Plane*                    _ground;
  pxr::SdfPath              _groundId;
  pxr::SdfPath              _solverId;

  std::vector<Mesh*>        _clothMeshes;
  std::vector<pxr::SdfPath> _clothMeshesId;
  std::vector<Mesh*>        _collideMeshes;
  std::vector<pxr::SdfPath> _collideMeshesId;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H