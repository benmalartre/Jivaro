
#include "../app/delegate.h"
#include "../geometry/geometry.h"
#include "../geometry/scene.h"

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
  return pxr::GfRange3d();
}


pxr::GfMatrix4d
Delegate::GetTransform(pxr::SdfPath const & id)
{
  if (_scene)return _scene->GetTransform(id);
  return pxr::GfMatrix4d(1);
}

size_t
Delegate::SamplePrimvar(pxr::SdfPath const& id,
  pxr::TfToken const& key,
  size_t maxNumSamples,
  float* sampleTimes,
  pxr::VtValue* sampleValues)
{
  //std::cout << "sample prim var : " << id << ":" << key << std::endl;
  if (key == pxr::HdTokens->widths) {
    auto& prims = _scene->GetPrims();
    if (prims.find(id) != prims.end()) {
      //std::cout << "set sample widths : " << std::endl;
      //*sampleValues = pxr::VtValue(prims[id].geom->GetRadius());
    }
    return 1;
  }
  return 0;
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
  /*return _SamplePrimvar(id, key, maxNumSamples, sampleTimes, sampleValues,
    sampleIndices);
    */
  //std::cout << "sample index primvar not implemented" << std::endl;
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
  return pxr::HdDisplayStyle(1, false, true, false, true, false);
}

pxr::TfToken
Delegate::GetRenderTag(pxr::SdfPath const& id)
{
  return _scene->GetRenderTag(id);
}


pxr::VtValue
Delegate::Get(pxr::SdfPath const& id, pxr::TfToken const& key)
{
  //std::cout << "delegate get " << id << ": " << key << std::endl;
  return _scene->Get(id, key);
}


/*virtual*/
pxr::VtValue
Delegate::GetIndexedPrimvar(pxr::SdfPath const& id, pxr::TfToken const& key, 
                                        pxr::VtIntArray *outIndices) 
{
  //std::cout << "get indexed prim var : " << id << ":" << key << std::endl;
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
  //std::cout << "delegate get primvar descriptor : " << id << " : interpolation = " << interpolation << std::endl;
  pxr::HdPrimvarDescriptorVector primvars;
  if (interpolation == pxr::HdInterpolationVertex) {
    primvars.emplace_back(pxr::HdTokens->points, interpolation,
      pxr::HdPrimvarRoleTokens->point);

    primvars.emplace_back(pxr::HdTokens->displayColor, interpolation,
      pxr::HdPrimvarRoleTokens->color);

  } else if(interpolation == pxr::HdInterpolationVarying) {
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
  if (!_scene)return;
  
  for (auto& prim : _scene->GetPrims()) {
    Geometry* geometry = prim.second.geom;
    if (!geometry->IsOutput())continue;

    switch (geometry->GetType()) {
      case Geometry::MESH:
        index.InsertRprim(pxr::HdPrimTypeTokens->mesh, this, prim.first);
        break;

      case Geometry::CURVE:
        index.InsertRprim(pxr::HdPrimTypeTokens->basisCurves, this, prim.first);
        break;

      case Geometry::POINT:
        index.InsertRprim(pxr::HdPrimTypeTokens->points, this, prim.first);
        break;

      case Geometry::INSTANCER:
        index.InsertInstancer(this, prim.first);
        break;
    }
    
    if(prim.second.bits != pxr::HdChangeTracker::Clean) {
      pxr::HdChangeTracker& tracker = GetRenderIndex().GetChangeTracker();
      tracker.MarkRprimDirty(prim.first, prim.second.bits);
    }
    
  }
}

void Delegate::RemoveScene() {
  pxr::HdRenderIndex& index = GetRenderIndex();
  if (_scene) {
    for (auto& prim : _scene->GetPrims()) {
      index.RemoveRprim(prim.first);
    }
    _scene = NULL;
  }
}

void Delegate::UpdateScene()
{
  pxr::HdChangeTracker& tracker = GetRenderIndex().GetChangeTracker();
  for (auto& prim : _scene->GetPrims()) {
    if(prim.second.bits != pxr::HdChangeTracker::Clean) {
      tracker.MarkRprimDirty(prim.first, prim.second.bits);
    }
  }
}

JVR_NAMESPACE_CLOSE_SCOPE