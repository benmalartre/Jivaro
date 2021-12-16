#pragma once

#include "../common.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/base/tf/hashmap.h>


JVR_NAMESPACE_OPEN_SCOPE

typedef pxr::TfHashMap< pxr::SdfPath, pxr::UsdStageRefPtr, pxr::SdfPath::Hash > 
  _StageCacheMap;


class Item {
public:
  Item() { _prim = pxr::UsdPrim(); _geometry = NULL; };
  Item(pxr::UsdPrim& inPrim, Geometry* inGeom) { 
    _prim = inPrim; _geometry = inGeom; };
private:
  pxr::UsdPrim _prim;
  Geometry* _geometry;

};

typedef pxr::TfHashMap< pxr::SdfPath, Mesh, pxr::SdfPath::Hash > _MeshMap;
typedef pxr::TfHashMap< pxr::SdfPath, Curve, pxr::SdfPath::Hash > _CurveMap;
typedef pxr::TfHashMap< pxr::SdfPath, Points, pxr::SdfPath::Hash > _PointsMap;

class Scene {
public:
  Scene();
  ~Scene();

  void ClearAllStages();
  void RemoveStage(const std::string& name);
  void RemoveStage(const pxr::SdfPath& path);
  pxr::UsdStageRefPtr& AddStageFromMemory(const std::string& name);
  pxr::UsdStageRefPtr& AddStageFromDisk(const std::string& filename);
  pxr::UsdStageRefPtr& GetRootStage() { return _rootStage; };
  pxr::UsdStageRefPtr& GetCurrentStage() { return _currentStage; };

  void TestVoronoi();

  Mesh* AddMesh(pxr::SdfPath& path, 
    const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d());
  Curve* AddCurve(pxr::SdfPath& path, 
    const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d());
  Points* AddPoints(pxr::SdfPath& path, 
    const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d());

private:
  pxr::UsdStageRefPtr _currentStage;
  pxr::UsdStageRefPtr _rootStage;
  _StageCacheMap  _allStages;
  _MeshMap _meshes;
  _CurveMap _curves;
  _PointsMap _points;
};


JVR_NAMESPACE_CLOSE_SCOPE