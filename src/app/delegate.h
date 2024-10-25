#ifndef JVR_APPLICATION_DELEGATE_H
#define JVR_APPLICATION_DELEGATE_H

#include "../common.h"
#include "pxr/imaging/hd/sceneDelegate.h"
#include "pxr/imaging/hd/tokens.h"
#include "pxr/imaging/glf/simpleLight.h"

#include <pxr/usdImaging/usdImaging/delegate.h>


JVR_NAMESPACE_OPEN_SCOPE

class Scene;
class Geometry;
class Delegate : public HdSceneDelegate {
public:

  Delegate(HdRenderIndex* parentIndex, SdfPath const& delegateID);
  ~Delegate();

  // -----------------------------------------------------------------------//
  /// \name Options
  // -----------------------------------------------------------------------//
  
  void AddLight(SdfPath const &id, GlfSimpleLight const &light);
  void SetLight(SdfPath const &id, TfToken const &key, VtValue value);
  void RemoveLight(SdfPath const &id);

  // -----------------------------------------------------------------------//
  /// \name Options
  // -----------------------------------------------------------------------//

  /// Returns true if the named option is enabled by the delegate.
  virtual bool IsEnabled(TfToken const& option) const override;


  // -----------------------------------------------------------------------//
  /// \name Rprim Aspects
  // -----------------------------------------------------------------------//

  /// Gets the topological mesh data for a given prim.
  virtual HdMeshTopology GetMeshTopology(SdfPath const& id) override;

  /// Gets the topological curve data for a given prim.
  virtual HdBasisCurvesTopology GetBasisCurvesTopology(SdfPath const& id) override;

  /// Gets the subdivision surface tags (sharpness, holes, etc).
  virtual PxOsdSubdivTags GetSubdivTags(SdfPath const& id) override;


  /// Gets the axis aligned bounds of a prim.
  /// The returned bounds are in the local space of the prim
  /// (transform is yet to be applied) and should contain the
  /// bounds of any child prims.
  ///
  /// The returned bounds does not include any displacement that
  /// might occur as the result of running shaders on the prim.
  virtual GfRange3d GetExtent(SdfPath const & id) override;

  /// Returns the object space transform, including all parent transforms.
  virtual GfMatrix4d GetTransform(SdfPath const & id) override;

  /// Returns the authored visible state of the prim.
  virtual bool GetVisible(SdfPath const & id) override;

  /// Returns the doubleSided state for the given prim.
  virtual bool GetDoubleSided(SdfPath const & id) override;

  /// Returns the cullstyle for the given prim.
  virtual HdCullStyle GetCullStyle(SdfPath const &id) override;

  /// Returns the shading style for the given prim.
  virtual VtValue GetShadingStyle(SdfPath const &id) override;

  /// Returns the refinement level for the given prim in the range [0,8].
  ///
  /// The refinement level indicates how many iterations to apply when
  /// subdividing subdivision surfaces or other refinable primitives.
  virtual HdDisplayStyle GetDisplayStyle(SdfPath const& id) override;

  virtual TfToken GetRenderTag(SdfPath const& id) override;
    
  /// Returns a named value.
  virtual VtValue Get(SdfPath const& id, TfToken const& key) override;

  /// Returns a named primvar value. If \a *outIndices is not nullptr and the 
  /// primvar has indices, it will return the unflattened primvar and set 
  /// \a *outIndices to the primvar's associated indices, clearing the array
  /// if the primvar is not indexed.
  virtual VtValue GetIndexedPrimvar(SdfPath const& id, 
                                    TfToken const& key, 
                                    VtIntArray *outIndices) override;

  /// Returns the prim categories.
  virtual VtArray<TfToken> GetCategories(SdfPath const& id) override;

  /// Returns the categories for all instances in the instancer.
  virtual std::vector<VtArray<TfToken>>
  GetInstanceCategories(SdfPath const &instancerId) override;

  /// Returns the coordinate system bindings, or a nullptr if none are bound.
  virtual HdIdVectorSharedPtr GetCoordSysBindings(SdfPath const& id) override;

  virtual HdPrimvarDescriptorVector GetPrimvarDescriptors(SdfPath const& id,
    HdInterpolation interpolation) override;

  virtual VtIntArray GetInstanceIndices(SdfPath const &instancerId,
                                             SdfPath const &prototypeId) override;

  virtual SdfPathVector GetInstancerPrototypes(SdfPath const& instancerId)override;
  virtual GfMatrix4d GetInstancerTransform(SdfPath const& instancerId)override;

  virtual SdfPath GetInstancerId(SdfPath const& primId) override;
  // Motion samples
  /*
  virtual size_t
  SampleTransform(SdfPath const& id, size_t maxNumSamples,
    float* times, GfMatrix4d* samples) override;
  
  virtual size_t
  SampleInstancerTransform(SdfPath const& instancerId,
    size_t maxSampleCount, float* times, GfMatrix4d* samples) override;*/

  virtual size_t
  SamplePrimvar(SdfPath const& id, TfToken const& key,
    size_t maxNumSamples, float* times,
    VtValue* samples) override;

  virtual size_t
  SampleIndexedPrimvar(SdfPath const& id, TfToken const& key,
    size_t maxNumSamples, float* times,
    VtValue* samples, VtIntArray* indices) override;


  // Materials
  SdfPath GetMaterialId(SdfPath const &rprimId) override;
  VtValue GetMaterialResource(SdfPath const &materialId) override;

  void SetScene(Scene* scene);
  void RemoveScene();
  Scene* GetScene() { return _scene; };
  void UpdateScene();

private:
  Scene* _scene;
  SdfPath _instancerId;

  std::map<SdfPath, VtValue> _materials;
  std::map<SdfPath, SdfPath> _materialBindings;

  using _ValueCache = TfHashMap<TfToken, VtValue, TfToken::HashFunctor>;
  using _ValueCacheMap = TfHashMap<SdfPath, _ValueCache, SdfPath::Hash>;
  _ValueCacheMap _valueCacheMap;

  SdfPath _cameraId;

};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_SCENE_H