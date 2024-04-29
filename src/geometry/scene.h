#ifndef JVR_GEOMETRY_SCENE_H
#define JVR_GEOMETRY_SCENE_H
#include <vector>
#include "../common.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/base/tf/hashmap.h>
#include <pxr/base/tf/denseHashSet.h>
#include <pxr/usdImaging/usdImaging/delegate.h>


JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
class Mesh;
class Curve;
class Points;
class Voxels;
class Graph;
struct Sample;
class Solver;
class Execution;

class Scene  {
public:
  struct _Prim {
    Geometry*          geom;
    pxr::HdDirtyBits   bits;
  };

  struct _Graph {
    Graph*             graph;
    pxr::HdDirtyBits   bits;
  };

  typedef pxr::TfHashMap< pxr::SdfPath, _Prim, pxr::SdfPath::Hash > _PrimMap;
  typedef pxr::TfHashMap< pxr::SdfPath, _Graph, pxr::SdfPath::Hash > _GraphMap;
  
  friend class Execution;

  Scene();
  ~Scene();

  void Init(const pxr::UsdStageRefPtr& stage);
  void Sync(const pxr::UsdStageRefPtr& stage, double time);

  void Save(const std::string& filename);
  void Export(const std::string& filename);

  Mesh* AddMesh(const pxr::SdfPath& path, const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Curve* AddCurve(const pxr::SdfPath& path, const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Points* AddPoints(const pxr::SdfPath& path, const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Voxels* AddVoxels(const pxr::SdfPath& path, Mesh* mesh, float radius);

  Geometry* AddGeometry(const pxr::SdfPath& path, short type, const pxr::GfMatrix4d& xfo);
  void AddGeometry(const pxr::SdfPath& path, Geometry* geom);

  void InjectGeometry(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
    Geometry* geometry, float time);

  void RemoveGeometry(const pxr::SdfPath& path);

  void Remove(const pxr::SdfPath& path);
  bool IsMesh(const pxr::SdfPath& path);
  bool IsCurves(const pxr::SdfPath& path);
  bool IsPoints(const pxr::SdfPath& path);

  const pxr::SdfPath GetPrimPath(Geometry* geom) const;
  _PrimMap& GetPrims() { return _prims; };
  const _PrimMap& GetPrims() const { return _prims; };

  _Prim* GetPrim(const pxr::SdfPath& path);
  Geometry* GetGeometry(const pxr::SdfPath& path);

  void MarkPrimDirty(const pxr::SdfPath& path, pxr::HdDirtyBits bits);

  /// Gets the topological mesh data for a given prim.
  pxr::HdMeshTopology GetMeshTopology(pxr::SdfPath const& id);

  /// Gets the topological curve data for a given prim.
  pxr::HdBasisCurvesTopology GetBasisCurvesTopology(pxr::SdfPath const& id);

  /// Gets the axis aligned bounds of a prim.
  /// The returned bounds are in the local space of the prim
  /// (transform is yet to be applied) and should contain the
  /// bounds of any child prims.
  ///
  /// The returned bounds does not include any displacement that
  /// might occur as the result of running shaders on the prim.
  pxr::GfRange3d GetExtent(pxr::SdfPath const & id);

  /// Returns the object space transform, including all parent transforms.
  pxr::GfMatrix4d GetTransform(pxr::SdfPath const & id);

  /// Returns the authored visible state of the prim.
  bool GetVisible(pxr::SdfPath const & id);


  pxr::TfToken GetRenderTag(pxr::SdfPath const& id);
    
  /// Returns a named value.
  pxr::VtValue Get(pxr::SdfPath const& id, pxr::TfToken const& key);

  /// Returns a named primvar value. If \a *outIndices is not nullptr and the 
  /// primvar has indices, it will return the unflattened primvar and set 
  /// \a *outIndices to the primvar's associated indices, clearing the array
  /// if the primvar is not indexed.
  pxr::VtValue GetIndexedPrimvar(pxr::SdfPath const& id, 
                                    pxr::TfToken const& key, 
                                    pxr::VtIntArray *outIndices);

  pxr::HdPrimvarDescriptorVector GetPrimvarDescriptors(pxr::SdfPath const& id,
    pxr::HdInterpolation interpolation);


private:
  Solver*                                                     _solver;
  _PrimMap                                                    _prims;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_SCENE_H