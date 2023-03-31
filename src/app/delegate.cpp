
#include "../app/delegate.h"
#include "../app/scene.h"
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

Delegate::Delegate(pxr::HdRenderIndex* parentIndex, pxr::SdfPath const& delegateID)
  : pxr::HdSceneDelegate(parentIndex, delegateID)
  , _scene(NULL)
{
}

Delegate::~Delegate()
{
}

bool
Delegate::IsEnabled(pxr::TfToken const& option) const
{
    if (option == pxr::HdOptionTokens->parallelRprimSync) {
        return true;
    }

    return false;
}

// -----------------------------------------------------------------------//
/// \name Rprim Aspects
// -----------------------------------------------------------------------//

pxr::HdMeshTopology
Delegate::GetMeshTopology(pxr::SdfPath const& id)
{
  if (_scene)return _scene->GetMeshTopology(id);
  return pxr::HdMeshTopology();
}

pxr::HdBasisCurvesTopology
Delegate::GetBasisCurvesTopology(pxr::SdfPath const& id)
{
  if (_scene)return _scene->GetBasisCurvesTopology(id);
  return pxr::HdBasisCurvesTopology();
}

pxr::PxOsdSubdivTags
Delegate::GetSubdivTags(pxr::SdfPath const& id)
{
    return pxr::PxOsdSubdivTags();
}

pxr::GfRange3d 
Delegate::GetExtent(pxr::SdfPath const& id)
{
  if (_scene)return _scene->GetExtent(id);
}


pxr::GfMatrix4d
Delegate::GetTransform(pxr::SdfPath const & id)
{
    return pxr::GfMatrix4d(1);
}

size_t
Delegate::SamplePrimvar(pxr::SdfPath const& id,
  pxr::TfToken const& key,
  size_t maxNumSamples,
  float* sampleTimes,
  pxr::VtValue* sampleValues)
{
  std::cout << "sample primvar " << id << ":" << key << std::endl;
  return _SamplePrimvar(id, key, maxNumSamples, sampleTimes, sampleValues,
    nullptr);
}

/*virtual*/
size_t
Delegate::SampleIndexedPrimvar(pxr::SdfPath const& id,
  pxr::TfToken const& key,
  size_t maxNumSamples,
  float* sampleTimes,
  pxr::VtValue* sampleValues,
  pxr::VtIntArray* sampleIndices)
{
  return _SamplePrimvar(id, key, maxNumSamples, sampleTimes, sampleValues,
    sampleIndices);
}

size_t
Delegate::_SamplePrimvar(pxr::SdfPath const& id,
  pxr::TfToken const& key,
  size_t maxNumSamples,
  float* sampleTimes,
  pxr::VtValue* sampleValues,
  pxr::VtIntArray* sampleIndices)
{
  /*
  //pxr::SdfPath cachePath = ConvertIndexPathToCachePath(id);
  _HdPrimInfo* primInfo = _GetHdPrimInfo(id);
  if (*TF_VERIFY(primInfo)) {
    if (sampleIndices) {
      sampleIndices[0] = VtIntArray(0);
    }
    // Retrieve the multi-sampled result.
    size_t nSamples = primInfo->adapter
      ->SamplePrimvar(primInfo->usdPrim, id, key,
        pxr::UsdTimeCode::Default(), maxNumSamples, sampleTimes, sampleValues,
        sampleIndices);
    return nSamples;
  }
  */
  return 0;

}


bool
Delegate::GetVisible(pxr::SdfPath const & id)
{
    return true;
}

/*virtual*/
bool
Delegate::GetDoubleSided(pxr::SdfPath const & id)
{
    return false;
}

/*virtual*/
pxr::HdCullStyle
Delegate::GetCullStyle(pxr::SdfPath const &id)
{
  return pxr::HdCullStyleNothing;
}

/*virtual*/
pxr::VtValue
Delegate::GetShadingStyle(pxr::SdfPath const &id)
{
    return pxr::VtValue();
}

/*virtual*/
pxr::HdDisplayStyle
Delegate::GetDisplayStyle(pxr::SdfPath const& id)
{
    return pxr::HdDisplayStyle();
}

pxr::TfToken
Delegate::GetRenderTag(pxr::SdfPath const& id)
{
  return _scene->GetRenderTag(id);
}


pxr::VtValue
Delegate::Get(pxr::SdfPath const& id, pxr::TfToken const& key)
{
  return _scene->Get(id, key);
}


/*virtual*/
pxr::VtValue
Delegate::GetIndexedPrimvar(pxr::SdfPath const& id, pxr::TfToken const& key, 
                                        pxr::VtIntArray *outIndices) 
{
  return pxr::VtValue();
}


pxr::VtArray<pxr::TfToken>
Delegate::GetCategories(pxr::SdfPath const& id)
{
    return pxr::VtArray<pxr::TfToken>();
}

std::vector<pxr::VtArray<pxr::TfToken>>
Delegate::GetInstanceCategories(pxr::SdfPath const &instancerId)
{
    return std::vector<pxr::VtArray<pxr::TfToken>>();
}

pxr::HdIdVectorSharedPtr
Delegate::GetCoordSysBindings(pxr::SdfPath const& id)
{
    return nullptr;
}

// -------------------------------------------------------------------------- //
// Primvar Support Methods
// -------------------------------------------------------------------------- //

pxr::HdPrimvarDescriptorVector Delegate::GetPrimvarDescriptors(pxr::SdfPath const& id,
  pxr::HdInterpolation interpolation)
{
  pxr::HdPrimvarDescriptorVector primvars;

  if (interpolation == pxr::HdInterpolationVertex) {
    primvars.emplace_back(pxr::HdTokens->points, interpolation,
      pxr::HdPrimvarRoleTokens->point);

    primvars.emplace_back(pxr::HdTokens->displayColor, interpolation,
      pxr::HdPrimvarRoleTokens->color);

    primvars.emplace_back(pxr::HdTokens->widths, interpolation);
  }

  return primvars;
}

void Delegate::SetScene(Scene* scene) {
  pxr::HdRenderIndex& index = GetRenderIndex();
  if (_scene) {
    for (auto& prim : _scene->GetPrims()) {
      index.RemoveRprim(prim.first);
    }
  }
  _scene = scene;
  if (_scene) {
    for (auto& prim : _scene->GetPrims()) {
      switch (prim.second.geom->GetType()) {
      case Geometry::MESH:
        index.InsertRprim(pxr::HdPrimTypeTokens->mesh, this, prim.first);
        break;

      case Geometry::CURVE:
        index.InsertRprim(pxr::HdPrimTypeTokens->basisCurves, this, prim.first);
        break;

      case Geometry::POINT:
        index.InsertRprim(pxr::HdPrimTypeTokens->points, this, prim.first);
        break;
      }
      pxr::HdChangeTracker& tracker = GetRenderIndex().GetChangeTracker();
      tracker.MarkRprimDirty(prim.first, pxr::HdChangeTracker::DirtyTopology);
    }
  }
}

Scene* Delegate::RemoveScene() {
  pxr::HdRenderIndex& index = GetRenderIndex();
  if (_scene) {
    for (auto& prim : _scene->GetPrims()) {
      index.RemoveRprim(prim.first);
    }
    Scene* scene = _scene;
    _scene = NULL;
    return scene;
  }
  return NULL;
}

void Delegate::UpdateScene()
{
  pxr::HdChangeTracker& tracker = GetRenderIndex().GetChangeTracker();
  for (auto& prim : _scene->GetPrims()) {
    tracker.MarkRprimDirty(prim.first, pxr::HdChangeTracker::DirtyPoints|pxr::HdChangeTracker::DirtyWidths);
  }
}

JVR_NAMESPACE_CLOSE_SCOPE