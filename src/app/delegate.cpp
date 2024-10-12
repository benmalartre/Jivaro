
#include "../app/delegate.h"
#include "../geometry/deformable.h"
#include "../geometry/instancer.h"
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
  return pxr::GfMatrix4d(1.0);
}

size_t
Delegate::SamplePrimvar(pxr::SdfPath const& id,
  pxr::TfToken const& key,
  size_t maxNumSamples,
  float* sampleTimes,
  pxr::VtValue* sampleValues)
{
  if (key == pxr::HdTokens->widths) {
    auto& prims = _scene->GetPrims();
    if (prims.find(id) != prims.end()) {
      //std::cout << "set sample widths : " << std::endl;
      *sampleValues = pxr::VtValue(((Deformable*)prims[id].geom)->GetWidths());
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
  //return _SamplePrimvar(id, key, maxNumSamples, sampleTimes, sampleValues,
   // sampleIndices);
    
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
// Instancer Support Methods
// -------------------------------------------------------------------------- //

pxr::VtIntArray
Delegate::GetInstanceIndices( pxr::SdfPath const &instancerId,
                              pxr::SdfPath const &prototypeId)
{
  Scene::_Prim* instancerPrim = _scene->GetPrim(instancerId);
  pxr::VtIntArray indices;

  if(instancerPrim && instancerPrim->geom->GetType() == Geometry::INSTANCER) {
    Instancer* instancer = (Instancer*)instancerPrim->geom;
    const pxr::VtArray<pxr::SdfPath>& prototypes = instancer->GetPrototypes();
    size_t prototypeIndex = 0;
    for (; prototypeIndex < prototypes.size(); ++prototypeIndex)
      if (prototypes[prototypeIndex] == prototypeId) break;
    
    if (prototypeIndex == prototypes.size()) return indices;

    const pxr::VtArray<int>& prototypeIndices = instancer->GetProtoIndices();
    for (size_t i = 0; i < prototypeIndices.size(); ++i)
      if (static_cast<size_t>(prototypeIndices[i]) == prototypeIndex)
        indices.push_back(i);

  }

  return indices;
}

pxr::GfMatrix4d
Delegate::GetInstancerTransform(pxr::SdfPath const &instancerId)
{
  return pxr::GfMatrix4d();
}

pxr::SdfPathVector
Delegate::GetInstancerPrototypes(pxr::SdfPath const &instancerId)
{
  return pxr::SdfPathVector();
}

pxr::SdfPath
Delegate::GetInstancerId(pxr::SdfPath const& primId)
{
  return _scene->GetInstancerBinding(primId);
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

  } else if(interpolation == pxr::HdInterpolationVarying) {
    primvars.emplace_back(pxr::HdTokens->widths, interpolation);
  }

  return primvars;
}

void Delegate::SetScene(Scene* scene) {
  pxr::HdRenderIndex& index = GetRenderIndex();
  if (_scene) {
    for (auto& prim : _scene->GetPrims()) {
      if(prim.second.geom->GetType() == Geometry::INSTANCER)
        index.RemoveInstancer(prim.first);
      else
        index.RemoveRprim(prim.first);
    }
  }
  _scene = scene;
  if (!_scene)return;

  pxr::HdChangeTracker& tracker = GetRenderIndex().GetChangeTracker();
  
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
        tracker.MarkInstancerDirty(prim.first,  pxr::HdChangeTracker::DirtyTransform |
                                                pxr::HdChangeTracker::DirtyPrimvar |
                                                pxr::HdChangeTracker::DirtyInstanceIndex);
        continue;
    }
    
    if(prim.second.bits != pxr::HdChangeTracker::Clean) {  
      tracker.MarkRprimDirty(prim.first, prim.second.bits);
    }
    
  }
}

void Delegate::RemoveScene() {
  pxr::HdRenderIndex& index = GetRenderIndex();
  if (_scene) {
    for (auto& prim : _scene->GetPrims()) {
      if(prim.second.geom->GetType() == Geometry::INSTANCER)
        index.RemoveInstancer(prim.first);
      else
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
      if(prim.second.geom->GetType() == Geometry::INSTANCER) 
        tracker.MarkInstancerDirty(prim.first, prim.second.bits);

      else
        tracker.MarkRprimDirty(prim.first, prim.second.bits);
    }
  }
}

JVR_NAMESPACE_CLOSE_SCOPE