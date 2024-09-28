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
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;

protected:
  void _TraverseStageFindingMeshes(pxr::UsdStageRefPtr& stage);

private:
  
  std::vector<pxr::SdfPath>    _meshesId;
  std::vector<Mesh*>           _baseMeshes;
  std::vector<Mesh*>           _meshes;
  
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PUSH_H