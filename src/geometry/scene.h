#ifndef JVR_GEOMETRY_SCENE_H
#define JVR_GEOMETRY_SCENE_H
#include <vector>
#include <map>
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
    Geometry*     geom;
    HdDirtyBits   bits;
  };

  struct _Graph {
    Graph*        graph;
    HdDirtyBits   bits;
  };

  typedef TfHashMap< SdfPath, _Prim, SdfPath::Hash >        _PrimMap;
  typedef TfHashMap< SdfPath, _Graph, SdfPath::Hash >       _GraphMap;
  typedef std::map< SdfPath, VtValue >                      _MaterialMap;
  typedef std::map< SdfPath, SdfPath >                      _MaterialBindingMap;
  
  friend class Execution;

  Scene();
  ~Scene();

  void Init(const UsdStageRefPtr& stage);
  void Sync(const UsdStageRefPtr& stage, const UsdTimeCode& time=UsdTimeCode::Default());

  Mesh* AddMesh(const SdfPath& path, const GfMatrix4d& xfo=GfMatrix4d(1.0));
  Curve* AddCurve(const SdfPath& path, const GfMatrix4d& xfo=GfMatrix4d(1.0));
  Points* AddPoints(const SdfPath& path, const GfMatrix4d& xfo=GfMatrix4d(1.0));
  Voxels* AddVoxels(const SdfPath& path, Mesh* mesh, float radius);

  Geometry* AddGeometry(const SdfPath& path, short type, const GfMatrix4d& xfo);
  void AddGeometry(const SdfPath& path, Geometry* geom);

  void InjectGeometry(UsdStageRefPtr& stage, const SdfPath& path, 
    Geometry* geometry, const UsdTimeCode& time=UsdTimeCode::Default());

  void RemoveGeometry(const SdfPath& path);

  void Remove(const SdfPath& path);
  bool IsMesh(const SdfPath& path);
  bool IsCurves(const SdfPath& path);
  bool IsPoints(const SdfPath& path);

  const SdfPath GetPrimPath(Geometry* geom) const;
  _PrimMap& GetPrims() { return _prims; };
  const _PrimMap& GetPrims() const { return _prims; };

  size_t GetNumGeometries() { return _prims.size(); };
  Geometry* GetGeometry(size_t index);
  _Prim* GetPrim(const SdfPath& path);
  Geometry* GetGeometry(const SdfPath& path);
  SdfPath GetInstancerBinding(const SdfPath& path);

  void MarkPrimDirty(const SdfPath& path, HdDirtyBits bits);

  /// Gets the topological mesh data for a given prim.
  HdMeshTopology GetMeshTopology(SdfPath const& id);

  /// Gets the topological curve data for a given prim.
  HdBasisCurvesTopology GetBasisCurvesTopology(SdfPath const& id);

  /// Gets the axis aligned bounds of a prim.
  /// The returned bounds are in the local space of the prim
  /// (transform is yet to be applied) and should contain the
  /// bounds of any child prims.
  ///
  /// The returned bounds does not include any displacement that
  /// might occur as the result of running shaders on the prim.
  GfRange3d GetExtent(SdfPath const & id);

  /// Returns the object space transform, including all parent transforms.
  GfMatrix4d GetTransform(SdfPath const & id);

  /// Returns the authored visible state of the prim.
  bool GetVisible(SdfPath const & id);


  TfToken GetRenderTag(SdfPath const& id);
    
  /// Returns a named value.
  VtValue Get(SdfPath const& id, TfToken const& key);

  /// Returns a named primvar value. If \a *outIndices is not nullptr and the 
  /// primvar has indices, it will return the unflattened primvar and set 
  /// \a *outIndices to the primvar's associated indices, clearing the array
  /// if the primvar is not indexed.
  VtValue GetIndexedPrimvar(SdfPath const& id, 
                                    TfToken const& key, 
                                    VtIntArray *outIndices);

  HdPrimvarDescriptorVector GetPrimvarDescriptors(SdfPath const& id,
    HdInterpolation interpolation);

  /// Materials
  SdfPath GetMaterialId(SdfPath const &rprimId);
  VtValue GetMaterialResource(SdfPath const &materialId);


private:
  Solver*                                                     _solver;
  _PrimMap                                                    _prims;
  _MaterialMap                                                _materials;
  _MaterialBindingMap                                         _materialBindings;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_SCENE_H