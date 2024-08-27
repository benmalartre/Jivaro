#ifndef JVR_TEST_GRADIENT_H
#define JVR_TEST_GRADIENT_H

#include "../acceleration/bvh.h"
#include "../acceleration/gradient.h"
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
  void _TraverseStageFindingMesh(pxr::UsdStageRefPtr& stage);

private:
  pxr::SdfPath              _bvhId;
  BVH                       _bvh;
  Instancer*                _instancer;
  Mesh*                     _mesh;
  pxr::SdfPath              _meshId;
  Points*                   _points;
  pxr::SdfPath              _pointsId;
  Gradient                  _gradient;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_GRADIENT_H