#ifndef JVR_APPLICATION_SCENE_H
#define JVR_APPLICATION_SCENE_H

#include "../common.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"
#include "../geometry/voxels.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/base/tf/hashmap.h>


JVR_NAMESPACE_OPEN_SCOPE

typedef pxr::TfHashMap< pxr::SdfPath, pxr::UsdStageRefPtr, pxr::SdfPath::Hash > 
  _StageCacheMap;


typedef pxr::TfHashMap< pxr::SdfPath, Mesh, pxr::SdfPath::Hash > _MeshMap;
typedef pxr::TfHashMap< pxr::SdfPath, Curve, pxr::SdfPath::Hash > _CurveMap;
typedef pxr::TfHashMap< pxr::SdfPath, Points, pxr::SdfPath::Hash > _PointsMap;
typedef pxr::TfHashMap< pxr::SdfPath, Voxels, pxr::SdfPath::Hash > _VoxelsMap;

class Scene {
public:
  enum ItemType {
    STATIC,
    DEFORMED,
    GENERATED
  };
  Scene(pxr::UsdStageRefPtr& stage);
  ~Scene();

  void Init(const pxr::UsdStageRefPtr& stage);
  void Save(const std::string& filename);
  void Export(const std::string& filename);
  void TestVoronoi();
  void Update(double time);

  Mesh* AddMesh(const pxr::SdfPath& path, 
    const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d());
  Curve* AddCurve(const pxr::SdfPath& path, 
    const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d());
  Points* AddPoints(const pxr::SdfPath& path, 
    const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d());
  Voxels* AddVoxels(const pxr::SdfPath& path, Mesh* mesh, float radius);

  _MeshMap& GetMeshes() { return _meshes; };
  const _MeshMap& GetMeshes() const { return _meshes; };
  _CurveMap& GetCurves() { return _curves; };
  const _CurveMap& GetCurves() const { return _curves; };
  _PointsMap& GetPoints() { return _points; };
  const _PointsMap& GetPoints() const { return _points; };
  _VoxelsMap& GetVoxels() { return _voxels; };
  const _VoxelsMap& GetVoxels() const { return _voxels; };

  Geometry* GetGeometry(const pxr::SdfPath& path);
  pxr::UsdStageRefPtr& GetStage();

private:
  pxr::UsdStageRefPtr _inputStage;
  pxr::UsdStageRefPtr _stage;

  _MeshMap            _meshes;
  _CurveMap           _curves;
  _PointsMap          _points;
  _VoxelsMap          _voxels;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_SCENE_H