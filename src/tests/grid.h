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
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;
  void _TraverseStageFindingMeshes(pxr::UsdStageRefPtr& stage);
  void _AddAnimationSamples(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path);

protected:
  void _UpdateRays();
  void _FindHits(size_t begin, size_t end, const pxr::GfVec3f* positions, 
     pxr::GfVec3f* results, bool* hits, Intersector* intersector);
  void _UpdateHits();
  bool _CompareHits(const pxr::VtArray<bool>& hits, const  pxr::VtArray<pxr::GfVec3f> points);

private:
  Mesh*                     _mesh;
  Curve*                    _rays;
  Points*                   _hits;
  Grid3D                    _grid;
  BVH                       _bvh;
  pxr::SdfPath              _meshId;
  pxr::SdfPath              _raysId;
  pxr::SdfPath              _hitsId;
  pxr::SdfPath              _gridId;
  std::vector<Geometry*>    _meshes;
  std::vector<pxr::SdfPath> _meshesId;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_GRID_H