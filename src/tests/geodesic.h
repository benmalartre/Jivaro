#ifndef JVR_TEST_GEODESIC_H
#define JVR_TEST_GEODESIC_H
#include <vector>
#include "../acceleration/kdtree.h"
#include "../acceleration/bvh.h"
#include "../geometry/geodesic.h"
#include "../geometry/implicit.h"
#include "../geometry/points.h"
#include "../exec/execution.h"


JVR_NAMESPACE_OPEN_SCOPE

class TestGeodesic : public Execution {
public:


  TestGeodesic() : Execution(){};
  ~TestGeodesic(){};
  void InitExec(UsdStageRefPtr& stage) override;
  void UpdateExec(UsdStageRefPtr& stage, float time) override;
  void TerminateExec(UsdStageRefPtr& stage) override;
  
protected:
  void _TraverseStageFindingMeshes(UsdStageRefPtr& stage);

  void _BenchmarckClosestPoints();
  void _BenchmarckClosestPoints2();
  void _ClosestPointQuery(size_t begin, size_t end, const GfVec3f* positions, GfVec3f* results);

  

private:
  SdfPath              _bvhId;
  BVH                       _bvh;
  KDTree                    _kdtree;
  Instancer*                _instancer;
  std::vector<Geometry*>    _meshes;
  std::vector<SdfPath> _meshesId;
  Points*                   _points;
  SdfPath              _pointsId;
  Xform*                    _xform;
  SdfPath              _xformId;
  Geodesic*                 _geodesic;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_GEODESIC_H