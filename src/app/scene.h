#ifndef JVR_APPLICATION_SCENE_H
#define JVR_APPLICATION_SCENE_H
#include <vector>
#include "../common.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/stageCache.h>
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

class Scene  {
public:
  Scene();
  ~Scene();

  void Init(const pxr::UsdStageRefPtr& stage);
  void Save(const std::string& filename);
  void Export(const std::string& filename);
  void Update(double time);

  Mesh* AddMesh(const pxr::SdfPath& path, 
    const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d());
  Curve* AddCurve(const pxr::SdfPath& path, 
    const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d());
  Points* AddPoints(const pxr::SdfPath& path, 
    const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d());
  Voxels* AddVoxels(const pxr::SdfPath& path, Mesh* mesh, float radius);

  void Remove(const pxr::SdfPath& path);
  bool IsMesh(const pxr::SdfPath& path);
  bool IsCurves(const pxr::SdfPath& path);
  bool IsPoints(const pxr::SdfPath& path);


  _PrimMap& GetPrims() { return _prims; };
  const _PrimMap& GetPrims() const { return _prims; };

  Geometry* GetGeometry(const pxr::SdfPath& path);

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

  void InitExec();
  void UpdateExec(double time);
  void TerminateExec();


private:
  Solver*                                                     _solver;
  _PrimMap                                                    _prims;
  typedef pxr::VtArray<Sample>                                _Samples;
  typedef std::pair<pxr::SdfPath, pxr::HdDirtyBits>           _Source;
  typedef pxr::VtArray<_Source>                               _Sources;
  pxr::TfHashMap<pxr::SdfPath, _Sources, pxr::SdfPath::Hash>  _sourcesMap;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_SCENE_H