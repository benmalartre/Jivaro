#ifndef JVR_APPLICATION_SCENE_H
#define JVR_APPLICATION_SCENE_H

#include "../common.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/base/tf/hashmap.h>


PXR_NAMESPACE_OPEN_SCOPE

typedef pxr::TfHashMap< pxr::SdfPath, pxr::UsdStageRefPtr, pxr::SdfPath::Hash > 
  _StageCacheMap;


typedef pxr::TfHashMap< pxr::SdfPath, Mesh, pxr::SdfPath::Hash > _MeshMap;
typedef pxr::TfHashMap< pxr::SdfPath, Curve, pxr::SdfPath::Hash > _CurveMap;
typedef pxr::TfHashMap< pxr::SdfPath, Points, pxr::SdfPath::Hash > _PointsMap;

class Scene {
public:
  Scene();
  ~Scene();

  void Init(const pxr::UsdStageRefPtr& stage);
  void Save(const std::string& filename);
  void Export(const std::string& filename);
  void TestVoronoi();

  Mesh* AddMesh(pxr::SdfPath& path, 
    const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d());
  Curve* AddCurve(pxr::SdfPath& path, 
    const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d());
  Points* AddPoints(pxr::SdfPath& path, 
    const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d());

private:
  pxr::UsdStageRefPtr _inputStage;
  pxr::UsdStageRefPtr _stage;

  _MeshMap            _meshes;
  _CurveMap           _curves;
  _PointsMap          _points;
};


PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_SCENE_H