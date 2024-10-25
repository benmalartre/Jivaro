#include <pxr/base/gf/frustum.h>
#include <pxr/imaging/hd/light.h>
#include <pxr/imaging/hd/rprimCollection.h>
#include <pxr/imaging/hdx/drawTargetTask.h>
#include <pxr/imaging/hdx/pickTask.h>
#include <pxr/imaging/hdx/renderTask.h>
#include <pxr/imaging/hdx/selectionTask.h>
#include <pxr/imaging/hdx/simpleLightTask.h>
#include <pxr/imaging/hdx/shadowTask.h>
#include <pxr/imaging/hdx/shadowMatrixComputation.h>

#include "../app/delegate.h"
#include "../geometry/deformable.h"
#include "../geometry/instancer.h"
#include "../geometry/scene.h"

JVR_NAMESPACE_OPEN_SCOPE


namespace {
class ShadowMatrix : public HdxShadowMatrixComputation
{
public:
  ShadowMatrix(GlfSimpleLight const &light) {
    GfFrustum frustum;
    frustum.SetProjectionType(GfFrustum::Orthographic);
    frustum.SetWindow(GfRange2d(GfVec2d(-10, -10), GfVec2d(10, 10)));
    frustum.SetNearFar(GfRange1d(0, 100));
    const GfVec4d pos = light.GetPosition();
    frustum.SetPosition(GfVec3d(0, 0, 10));
    frustum.SetRotation(GfRotation(GfVec3d(0, 0, 1),
                                    GfVec3d(pos[0], pos[1], pos[2])));

    _shadowMatrix =
      frustum.ComputeViewMatrix() * frustum.ComputeProjectionMatrix();
  }

  std::vector<GfMatrix4d> Compute(
      const GfVec4f &viewport,
      CameraUtilConformWindowPolicy policy) override {
    return { _shadowMatrix };
  }

  std::vector<GfMatrix4d> Compute(
    const CameraUtilFraming &framing,
    CameraUtilConformWindowPolicy policy) override {
      return { _shadowMatrix };
  }

private:
  GfMatrix4d _shadowMatrix;
};

}

Delegate::Delegate(HdRenderIndex* parentIndex, SdfPath const& delegateID)
  : HdSceneDelegate(parentIndex, delegateID)
  , _scene(NULL)
{
}

Delegate::~Delegate()
{
}

bool
Delegate::IsEnabled(TfToken const& option) const
{
  if (option == HdOptionTokens->parallelRprimSync)
    return true;
  return false;
}

// -----------------------------------------------------------------------//
/// \name Lights
// -----------------------------------------------------------------------//
void
Delegate::AddLight(SdfPath const &id, GlfSimpleLight const &light)
{
  // add light
  GetRenderIndex().InsertSprim(HdPrimTypeTokens->simpleLight, this, id);
  _ValueCache &cache = _valueCacheMap[id];

  HdxShadowParams shadowParams;
  shadowParams.enabled = light.HasShadow();
  shadowParams.resolution = 512;
  shadowParams.shadowMatrix
    = HdxShadowMatrixComputationSharedPtr(new ShadowMatrix(light));
  shadowParams.bias = -0.001;
  shadowParams.blur = 0.1;

  cache[HdLightTokens->params] = light;
  cache[HdLightTokens->shadowParams] = shadowParams;
  cache[HdLightTokens->shadowCollection]
    = HdRprimCollection(HdTokens->geometry, 
        HdReprSelector(HdReprTokens->refined));
}

void
Delegate::SetLight(SdfPath const &id, TfToken const &key,
                               VtValue value)
{
  _ValueCache &cache = _valueCacheMap[id];
  cache[key] = value;
  if (key == HdLightTokens->params) {
    // update shadow matrix too
    GlfSimpleLight light = value.Get<GlfSimpleLight>();
    HdxShadowParams shadowParams
      = cache[HdLightTokens->shadowParams].Get<HdxShadowParams>();
    shadowParams.shadowMatrix
      = HdxShadowMatrixComputationSharedPtr(new ShadowMatrix(light));

    GetRenderIndex().GetChangeTracker().MarkSprimDirty(
      id, HdLight::DirtyParams|HdLight::DirtyShadowParams);
    cache[HdLightTokens->shadowParams] = shadowParams;
  } else if (key == HdTokens->transform) {
    GetRenderIndex().GetChangeTracker().MarkSprimDirty(
      id, HdLight::DirtyTransform);
  } else if (key == HdLightTokens->shadowCollection) {
    GetRenderIndex().GetChangeTracker().MarkSprimDirty(
      id, HdLight::DirtyCollection);
  }
}

void
Delegate::RemoveLight(SdfPath const &id)
{
  // remove light
  GetRenderIndex().RemoveSprim(HdPrimTypeTokens->simpleLight, id);
  _valueCacheMap.erase(id);
}


// -----------------------------------------------------------------------//
/// \name Rprim Aspects
// -----------------------------------------------------------------------//

HdMeshTopology
Delegate::GetMeshTopology(SdfPath const& id)
{
  if (_scene)return _scene->GetMeshTopology(id);
  return HdMeshTopology();
}

HdBasisCurvesTopology
Delegate::GetBasisCurvesTopology(SdfPath const& id)
{
  if (_scene)return _scene->GetBasisCurvesTopology(id);
  return HdBasisCurvesTopology();
}

PxOsdSubdivTags
Delegate::GetSubdivTags(SdfPath const& id)
{
    return PxOsdSubdivTags();
}

GfRange3d 
Delegate::GetExtent(SdfPath const& id)
{
  if (_scene)return _scene->GetExtent(id);
  return GfRange3d();
}


GfMatrix4d
Delegate::GetTransform(SdfPath const & id)
{
  if (_scene)return _scene->GetTransform(id);
  return GfMatrix4d(1.0);
}

size_t
Delegate::SamplePrimvar(SdfPath const& id,
  TfToken const& key,
  size_t maxNumSamples,
  float* sampleTimes,
  VtValue* sampleValues)
{
  if (key == HdTokens->widths) {
    auto& prims = _scene->GetPrims();
    if (prims.find(id) != prims.end()) {
      //std::cout << "set sample widths : " << std::endl;
      *sampleValues = VtValue(((Deformable*)prims[id].geom)->GetWidths());
    }
    return 1;
  }
  return 0;
}

/*virtual*/
size_t
Delegate::SampleIndexedPrimvar(SdfPath const& id,
  TfToken const& key,
  size_t maxNumSamples,
  float* sampleTimes,
  VtValue* sampleValues,
  VtIntArray* sampleIndices)
{
  //return _SamplePrimvar(id, key, maxNumSamples, sampleTimes, sampleValues,
   // sampleIndices);
    
  //std::cout << "sample index primvar not implemented" << std::endl;
  return 0;
}

bool
Delegate::GetVisible(SdfPath const & id)
{
    return true;
}

/*virtual*/
bool
Delegate::GetDoubleSided(SdfPath const & id)
{
    return true;
}

/*virtual*/
HdCullStyle
Delegate::GetCullStyle(SdfPath const &id)
{
  return HdCullStyleNothing;
}

/*virtual*/
VtValue
Delegate::GetShadingStyle(SdfPath const &id)
{
    return VtValue();
}

/*virtual*/
HdDisplayStyle
Delegate::GetDisplayStyle(SdfPath const& id)
{
  return HdDisplayStyle(1, false, true, false, true, false);
}

TfToken
Delegate::GetRenderTag(SdfPath const& id)
{
  return _scene->GetRenderTag(id);
}


VtValue
Delegate::Get(SdfPath const& id, TfToken const& key)
{
  return _scene->Get(id, key);
}


/*virtual*/
VtValue
Delegate::GetIndexedPrimvar(SdfPath const& id, TfToken const& key, 
                                        VtIntArray *outIndices) 
{
  return VtValue();
}


VtArray<TfToken>
Delegate::GetCategories(SdfPath const& id)
{
    return VtArray<TfToken>();
}

std::vector<VtArray<TfToken>>
Delegate::GetInstanceCategories(SdfPath const &instancerId)
{
    return std::vector<VtArray<TfToken>>();
}

HdIdVectorSharedPtr
Delegate::GetCoordSysBindings(SdfPath const& id)
{
    return nullptr;
}

// -------------------------------------------------------------------------- //
// Instancer Support Methods
// -------------------------------------------------------------------------- //

VtIntArray
Delegate::GetInstanceIndices( SdfPath const &instancerId,
                              SdfPath const &prototypeId)
{
  Scene::_Prim* instancerPrim = _scene->GetPrim(instancerId);
  VtIntArray indices;

  if(instancerPrim && instancerPrim->geom->GetType() == Geometry::INSTANCER) {
    Instancer* instancer = (Instancer*)instancerPrim->geom;
    const VtArray<SdfPath>& prototypes = instancer->GetPrototypes();
    size_t prototypeIndex = 0;
    for (; prototypeIndex < prototypes.size(); ++prototypeIndex)
      if (prototypes[prototypeIndex] == prototypeId) break;
    
    if (prototypeIndex == prototypes.size()) return indices;

    const VtArray<int>& prototypeIndices = instancer->GetProtoIndices();
    for (size_t i = 0; i < prototypeIndices.size(); ++i)
      if (static_cast<size_t>(prototypeIndices[i]) == prototypeIndex)
        indices.push_back(i);

  }

  return indices;
}

GfMatrix4d
Delegate::GetInstancerTransform(SdfPath const &instancerId)
{
  return GfMatrix4d();
}

SdfPathVector
Delegate::GetInstancerPrototypes(SdfPath const &instancerId)
{
  return SdfPathVector();
}

SdfPath
Delegate::GetInstancerId(SdfPath const& primId)
{
  return _scene->GetInstancerBinding(primId);
}

// -------------------------------------------------------------------------- //
// Primvar Support Methods
// -------------------------------------------------------------------------- //
HdPrimvarDescriptorVector Delegate::GetPrimvarDescriptors(SdfPath const& id,
  HdInterpolation interpolation)
{
  HdPrimvarDescriptorVector primvars;
  if (interpolation == HdInterpolationVertex) {
    primvars.emplace_back(HdTokens->points, interpolation,
      HdPrimvarRoleTokens->point);

    primvars.emplace_back(HdTokens->displayColor, interpolation,
      HdPrimvarRoleTokens->color);

  } else if(interpolation == HdInterpolationVarying) {
    primvars.emplace_back(HdTokens->widths, interpolation);
  }

  return primvars;
}


// ---------------------------------------------------------------------------- //
// Materials 
// ---------------------------------------------------------------------------- //
/*virtual*/ 
SdfPath 
Delegate::GetMaterialId(SdfPath const &rprimId)
{
  if(_scene) 
    return _scene->GetMaterialId(rprimId);
  return SdfPath();
}

/*virtual*/
VtValue 
Delegate::GetMaterialResource(SdfPath const &materialId)
{
  if (_scene)
    return _scene->GetMaterialResource(materialId);
  return VtValue();
}


// ---------------------------------------------------------------------------- //
// Scene 
// ---------------------------------------------------------------------------- //
void Delegate::SetScene(Scene* scene) {
  HdRenderIndex& index = GetRenderIndex();
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

  HdChangeTracker& tracker = GetRenderIndex().GetChangeTracker();
  
  for (auto& prim : _scene->GetPrims()) {
    Geometry* geometry = prim.second.geom;
    if (!geometry->IsOutput())continue;

    switch (geometry->GetType()) {
      case Geometry::MESH:
        index.InsertRprim(HdPrimTypeTokens->mesh, this, prim.first);
        break;

      case Geometry::CURVE:
        index.InsertRprim(HdPrimTypeTokens->basisCurves, this, prim.first);
        break;

      case Geometry::POINT:
        index.InsertRprim(HdPrimTypeTokens->points, this, prim.first);
        break;

      case Geometry::INSTANCER:       
        index.InsertInstancer(this, prim.first);
        tracker.MarkInstancerDirty(prim.first,  HdChangeTracker::DirtyTransform |
                                                HdChangeTracker::DirtyPrimvar |
                                                HdChangeTracker::DirtyInstanceIndex);
        continue;
    }
    
    if(prim.second.bits != HdChangeTracker::Clean) {  
      tracker.MarkRprimDirty(prim.first, prim.second.bits);
    }
    
  }
}

void Delegate::RemoveScene() {
  HdRenderIndex& index = GetRenderIndex();
  if (_scene) {
    for (auto& prim : _scene->GetPrims()) {
      if(prim.second.geom->GetType() == Geometry::INSTANCER) {
        index.RemoveInstancer(prim.first);
      }
      else
        index.RemoveRprim(prim.first);
    }
    _scene = NULL;
  }
}

void Delegate::UpdateScene()
{
  HdChangeTracker& tracker = GetRenderIndex().GetChangeTracker();
  for (auto& prim : _scene->GetPrims()) {
    if(prim.second.bits != HdChangeTracker::Clean) {
      if(prim.second.geom->GetType() == Geometry::INSTANCER) 
        tracker.MarkInstancerDirty(prim.first, prim.second.bits);

      else
        tracker.MarkRprimDirty(prim.first, prim.second.bits);
    }
  }
}

JVR_NAMESPACE_CLOSE_SCOPE