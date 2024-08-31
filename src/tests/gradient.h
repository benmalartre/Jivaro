#ifndef JVR_TEST_GRADIENT_H
#define JVR_TEST_GRADIENT_H
#include <vector>
#include "../acceleration/bvh.h"
#include "../acceleration/gradient.h"
#include "../exec/execution.h"


JVR_NAMESPACE_OPEN_SCOPE

class Solver;
class Plane;
class Points;
class Xform;

class TestGradient : public Execution {
public:


  TestGradient() : Execution(){};
  ~TestGradient(){};
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;
  
protected:
  void _TraverseStageFindingMeshes(pxr::UsdStageRefPtr& stage);

private:
  pxr::SdfPath              _bvhId;
  BVH                       _bvh;
  Instancer*                _instancer;
  std::vector<Geometry*>    _meshes;
  std::vector<pxr::SdfPath> _meshesId;
  Points*                   _points;
  pxr::SdfPath              _pointsId;
  Xform*                    _xform;
  pxr::SdfPath              _xformId;
  Gradient                  _gradient;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_GRADIENT_H