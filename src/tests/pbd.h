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
  void InitExec(UsdStageRefPtr& stage) override;
  void UpdateExec(UsdStageRefPtr& stage, float time) override;
  void TerminateExec(UsdStageRefPtr& stage) override;
  void PopulateSceneIndex(HdSceneIndexBase* index) override;
  void RemoveFromSceneIndex(HdSceneIndexBase* index) override;

protected: 
  void _TraverseStageFindingElements(UsdStageRefPtr& stage);
  void _AddAnimationSamples(UsdStageRefPtr& stage, SdfPath& path);

private:
  Solver*                   _solver;
  Plane*                    _ground;
  SdfPath              _groundId;
  SdfPath              _solverId;

  std::vector<Geometry*>    _clothes;
  std::vector<SdfPath> _clothesId;
  std::vector<Geometry*>    _colliders;
  std::vector<SdfPath> _collidersId;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H