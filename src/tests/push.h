#ifndef JVR_TEST_PUSH_H
#define JVR_TEST_PUSH_H

#include "../acceleration/bvh.h"
#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class Mesh;

class TestPush : public Execution {
public:
  TestPush(){};
  ~TestPush(){};
  void InitExec(UsdStageRefPtr& stage) override;
  void UpdateExec(UsdStageRefPtr& stage, float time) override;
  void TerminateExec(UsdStageRefPtr& stage) override;

protected:
  void _TraverseStageFindingMeshes(UsdStageRefPtr& stage);

private:
  
  std::vector<SdfPath>    _meshesId;
  std::vector<Mesh*>           _meshes;
  
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PUSH_H