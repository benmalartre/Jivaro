#ifndef JVR_TEST_GRID_H
#define JVR_TEST_GRID_H

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


class TestGrid : public Execution {
public:
  friend class Scene;
  TestGrid() : Execution(){};
  void InitExec(UsdStageRefPtr& stage) override;
  void UpdateExec(UsdStageRefPtr& stage, float time) override;
  void TerminateExec(UsdStageRefPtr& stage) override;
  void _TraverseStageFindingMeshes(UsdStageRefPtr& stage);
  void _AddAnimationSamples(UsdStageRefPtr& stage, SdfPath& path);

protected:
  void _UpdateRays();
  void _FindHits(size_t begin, size_t end, const GfVec3f* positions, 
     GfVec3f* results, bool* hits, Intersector* intersector);
  void _UpdateHits();
  bool _CompareHits(const VtArray<bool>& hits, const  VtArray<GfVec3f> points);

private:
  Mesh*                     _mesh;
  Curve*                    _rays;
  Points*                   _hits;
  Instancer*                _leaves;
  Grid3D                    _grid;
  BVH                       _bvh;
  SdfPath              _meshId;
  SdfPath              _raysId;
  SdfPath              _hitsId;
  SdfPath              _gridId;
  std::vector<Geometry*>    _meshes;
  std::vector<SdfPath> _meshesId;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_GRID_H