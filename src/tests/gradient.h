#ifndef JVR_TEST_GRADIENT_H
#define JVR_TEST_GRADIENT_H

#include "../acceleration/bvh.h"
#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class Solver;
class Plane;
class Points;

class TestGradient : public Execution {
public:
  TestGradient() : Execution(){};
  ~TestGradient(){};
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;
  
protected:
  void _Initialize(pxr::UsdStageRefPtr& stage);
  void _Terminate(pxr::UsdStageRefPtr& stage);
  size_t _TraverseStageFindingMeshes(pxr::UsdStageRefPtr& stage);

private:
  pxr::SdfPath              _bvhId;
  BVH                       _bvh;
  Instancer*                _leaves;
  std::vector<Geometry*>    _meshes;
  std::vector<pxr::SdfPath> _meshesId;
  Points*                   _points;
  pxr::SdfPath              _pointsId;
 
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_GRADIENT_H