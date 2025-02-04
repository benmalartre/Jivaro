#ifndef JVR_TEST_RIVET_H
#define JVR_TEST_RIVET_H

#include "../acceleration/intersector.h"
#include "../acceleration/grid3d.h"
#include "../acceleration/bvh.h"
#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class Curve;
class Mesh;
class Points;
class Grid;
class Instancer;
class Geometry;


class TestRivet : public Execution {
public:
  friend class Scene;
  TestRivet() : Execution(){};
  void InitExec(UsdStageRefPtr& stage) override;
  void UpdateExec(UsdStageRefPtr& stage, float time) override;
  void TerminateExec(UsdStageRefPtr& stage) override;
  void _TraverseStageFindingMeshes(UsdStageRefPtr& stage);

protected:

private:
  std::vector<Location>     _rivets;
  Mesh*                     _mesh;
  Curve*                    _curves;
  SdfPath              _curvesId;
  Points*                   _points;
  SdfPath              _pointsId;
  BVH                       _bvh;
  std::vector<Mesh*>        _meshes;
  std::vector<SdfPath> _meshesId;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_GRID_H