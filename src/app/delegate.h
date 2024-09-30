#ifndef JVR_APPLICATION_DELEGATE_H
#define JVR_APPLICATION_DELEGATE_H

#include "../common.h"
#include <pxr/usdImaging/usdImaging/delegate.h>


JVR_NAMESPACE_OPEN_SCOPE

class Scene;
class Geometry;
class Delegate : public pxr::HdSceneDelegate {
public:

  Delegate(pxr::HdRenderIndex* parentIndex, pxr::SdfPath const& delegateID);
  ~Delegate();

  

  // -----------------------------------------------------------------------//
  /// \name Options
  // -----------------------------------------------------------------------//

  /// Returns true if the named option is enabled by the delegate.
  virtual bool IsEnabled(pxr::TfToken const& option) const override;


  // -----------------------------------------------------------------------//
  /// \name Rprim Aspects
  // -----------------------------------------------------------------------//

  /// Gets the topological mesh data for a given prim.
  virtual pxr::HdMeshTopology GetMeshTopology(pxr::SdfPath const& id) override;

  /// Gets the topological curve data for a given prim.
  virtual pxr::HdBasisCurvesTopology GetBasisCurvesTopology(pxr::SdfPath const& id) override;

  /// Gets the subdivision surface tags (sharpness, holes, etc).
  virtual pxr::PxOsdSubdivTags GetSubdivTags(pxr::SdfPath const& id) override;


  /// Gets the axis aligned bounds of a prim.
  /// The returned bounds are in the local space of the prim
  /// (transform is yet to be applied) and should contain the
  /// bounds of any child prims.
  ///
  /// The returned bounds does not include any displacement that
  /// might occur as the result of running shaders on the prim.
  virtual pxr::GfRange3d GetExtent(pxr::SdfPath const & id) override;

  /// Returns the object space transform, including all parent transforms.
  virtual pxr::GfMatrix4d GetTransform(pxr::SdfPath const & id) override;

  /// Returns the authored visible state of the prim.
  virtual bool GetVisible(pxr::SdfPath const & id) override;

  /// Returns the doubleSided state for the given prim.
  virtual bool GetDoubleSided(pxr::SdfPath const & id) override;

  /// Returns the cullstyle for the given prim.
  virtual pxr::HdCullStyle GetCullStyle(pxr::SdfPath const &id) override; 

  /// Returns the refinement level for the given prim in the range [0,8].
  ///
  /// The refinement level indicates how many iterations to apply when
  /// subdividing subdivision surfaces or other refinable primitives.
  virtual pxr::HdDisplayStyle GetDisplayStyle(pxr::SdfPath const& id) override;

  virtual pxr::TfToken GetRenderTag(pxr::SdfPath const& id) override;
    
  /// Returns a named value.
  virtual pxr::VtValue Get(pxr::SdfPath const& id, pxr::TfToken const& key) override;

  /// Returns a named primvar value. If \a *outIndices is not nullptr and the 
  /// primvar has indices, it will return the unflattened primvar and set 
  /// \a *outIndices to the primvar's associated indices, clearing the array
  /// if the primvar is not indexed.
  virtual pxr::VtValue GetIndexedPrimvar(pxr::SdfPath const& id, 
                                    pxr::TfToken const& key, 
                                    pxr::VtIntArray *outIndices) override;

  /// Returns the prim categories.
  virtual pxr::VtArray<pxr::TfToken> GetCategories(pxr::SdfPath const& id) override;

  /// Returns the categories for all instances in the instancer.
  virtual std::vector<pxr::VtArray<pxr::TfToken>>
  GetInstanceCategories(pxr::SdfPath const &instancerId) override;

  /// Returns the coordinate system bindings, or a nullptr if none are bound.
  virtual pxr::HdIdVectorSharedPtr GetCoordSysBindings(pxr::SdfPath const& id) override;

  virtual pxr::HdPrimvarDescriptorVector GetPrimvarDescriptors(pxr::SdfPath const& id,
    pxr::HdInterpolation interpolation) override;

  virtual pxr::VtIntArray GetInstanceIndices(pxr::SdfPath const& instancerId,
                                          pxr::SdfPath const& prototypeId) override;

  virtual pxr::SdfPathVector GetInstancerPrototypes(pxr::SdfPath const& instancerId)override;
  virtual pxr::GfMatrix4d GetInstancerTransform(pxr::SdfPath const& instancerId)override;

  virtual pxr::SdfPath GetInstancerId(pxr::SdfPath const& primId) override;
  // Motion samples
  /*
  virtual size_t
  SampleTransform(pxr::SdfPath const& id, size_t maxNumSamples,
    float* times, pxr::GfMatrix4d* samples) override;
  
  virtual size_t
  SampleInstancerTransform(pxr::SdfPath const& instancerId,
    size_t maxSampleCount, float* times, pxr::GfMatrix4d* samples) override;*/

  virtual size_t
  SamplePrimvar(pxr::SdfPath const& id, pxr::TfToken const& key,
    size_t maxNumSamples, float* times,
    pxr::VtValue* samples) override;

  virtual size_t
  SampleIndexedPrimvar(pxr::SdfPath const& id, pxr::TfToken const& key,
    size_t maxNumSamples, float* times,
    pxr::VtValue* samples, pxr::VtIntArray* indices) override;

  void SetScene(Scene* scene);
  void RemoveScene();
  Scene* GetScene() { return _scene; };
  void UpdateScene();

private:
  Scene* _scene;
  pxr::SdfPath _instancerId;

};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_SCENE_H