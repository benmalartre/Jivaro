//
// Copyright 2020 benmalartre
//
// Unlicensed
//
#include <iostream>
#include "pxr/imaging/glf/glew.h"
#include "pxr/imaging/plugin/LoFi/resourceRegistry.h"
#include "pxr/imaging/plugin/LoFi/renderDelegate.h"
#include "pxr/imaging/plugin/LoFi/renderPass.h"

#include "pxr/imaging/hd/extComputation.h"
#include "pxr/imaging/hd/resourceRegistry.h"
#include "pxr/imaging/hd/tokens.h"

#include "pxr/imaging/plugin/LoFi/mesh.h"
#include "pxr/imaging/plugin/LoFi/scene.h"
#include "pxr/imaging/plugin/LoFi/renderer.h"
#include "pxr/imaging/hd/camera.h"
#include "pxr/imaging/hd/bprim.h"
#include "pxr/imaging/hd/perfLog.h"
#include "pxr/imaging/hd/tokens.h"



PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PUBLIC_TOKENS(LoFiRenderSettingsTokens, LOFI_RENDER_SETTINGS_TOKENS);


const TfTokenVector LoFiRenderDelegate::SUPPORTED_RPRIM_TYPES =
{
    HdPrimTypeTokens->mesh,
};

const TfTokenVector LoFiRenderDelegate::SUPPORTED_SPRIM_TYPES =
{
  HdPrimTypeTokens->camera
};

const TfTokenVector LoFiRenderDelegate::SUPPORTED_BPRIM_TYPES =
{
};

std::mutex LoFiRenderDelegate::_mutexResourceRegistry;
std::atomic_int LoFiRenderDelegate::_counterResourceRegistry;
LoFiResourceRegistrySharedPtr LoFiRenderDelegate::_resourceRegistry;

LoFiRenderDelegate::LoFiRenderDelegate()
    : HdRenderDelegate()
{
    _Initialize();
}

LoFiRenderDelegate::LoFiRenderDelegate(
    HdRenderSettingsMap const& settingsMap)
    : HdRenderDelegate(settingsMap)
{
    _Initialize();
}

void
LoFiRenderDelegate::_Initialize()
{
  // Initialize one resource registry for all St plugins
  // It will also add the resource to the logging object so we
  // can query the resources used by all St plugins later
  std::lock_guard<std::mutex> guard(_mutexResourceRegistry);
  
  if (_counterResourceRegistry.fetch_add(1) == 0) {
      _resourceRegistry.reset( new LoFiResourceRegistry() );
      HdPerfLog::GetInstance().AddResourceRegistry(_resourceRegistry);
  }

  // Create the top-level renderer.
  _renderer = new LoFiRenderer(_resourceRegistry);
    

  // Create the RenderPassState Object (???)
  _renderPassState = CreateRenderPassState();

}

LoFiRenderDelegate::~LoFiRenderDelegate()
{
  // Clean the resource registry only when it is the last LoFi delegate
  {
    std::lock_guard<std::mutex> guard(_mutexResourceRegistry);
    if (_counterResourceRegistry.fetch_sub(1) == 1) {
      _resourceRegistry.reset();
    }
  }

  delete _renderer;
}

void 
LoFiRenderDelegate::CommitResources(HdChangeTracker *tracker)
{
  //_renderPassState->SetCamera(_sceneDelegate)
}

HdRenderSettingDescriptorList
LoFiRenderDelegate::GetRenderSettingDescriptors() const
{
    return _settingDescriptors;
}

TfTokenVector const&
LoFiRenderDelegate::GetSupportedRprimTypes() const
{
    return SUPPORTED_RPRIM_TYPES;
}

TfTokenVector const&
LoFiRenderDelegate::GetSupportedSprimTypes() const
{
    return SUPPORTED_SPRIM_TYPES;
}

TfTokenVector const&
LoFiRenderDelegate::GetSupportedBprimTypes() const
{
    return SUPPORTED_BPRIM_TYPES;
}

HdResourceRegistrySharedPtr
LoFiRenderDelegate::GetResourceRegistry() const
{
    return _resourceRegistry;
}

HdRenderPassSharedPtr 
LoFiRenderDelegate::CreateRenderPass(
    HdRenderIndex *index,
    HdRprimCollection const& collection)
{
    return HdRenderPassSharedPtr(
      new LoFiRenderPass(index, collection, _renderer)
    );  
}

HdRprim *
LoFiRenderDelegate::CreateRprim(TfToken const& typeId,
                                    SdfPath const& rprimId,
                                    SdfPath const& instancerId)
{
    if (typeId == HdPrimTypeTokens->mesh) {
        return new LoFiMesh(rprimId, instancerId);
    } else {
        TF_CODING_ERROR("Unknown Rprim type=%s id=%s", 
            typeId.GetText(), 
            rprimId.GetText());
    }
    return nullptr;
}

void
LoFiRenderDelegate::DestroyRprim(HdRprim *rPrim)
{
    //std::cout << "Destroy LoFi Rprim id=" << rPrim->GetId() << std::endl;
    delete rPrim;
}

HdSprim *
LoFiRenderDelegate::CreateSprim(TfToken const& typeId,
                                    SdfPath const& sprimId)
{
    if (typeId == HdPrimTypeTokens->camera) 
    {
        return new HdCamera(sprimId);
    } 
    else 
    {
        TF_CODING_ERROR("Unknown Sprim Type %s", typeId.GetText());
    }

    return nullptr;
}

HdSprim *
LoFiRenderDelegate::CreateFallbackSprim(TfToken const& typeId)
{
    // For fallback sprims, create objects with an empty scene path.
    // They'll use default values and won't be updated by a scene delegate.
    if (typeId == HdPrimTypeTokens->camera) {
        return new HdCamera(SdfPath::EmptyPath());
    }
    else {
        TF_CODING_ERROR("Unknown Sprim Type %s", typeId.GetText());
    }

    return nullptr;
}

void
LoFiRenderDelegate::DestroySprim(HdSprim *sPrim)
{
    TF_CODING_ERROR("Destroy Sprim not supported");
}

HdBprim *
LoFiRenderDelegate::CreateBprim(TfToken const& typeId, SdfPath const& bprimId)
{
    TF_CODING_ERROR("Unknown Bprim type=%s id=%s", 
        typeId.GetText(), 
        bprimId.GetText());
    return nullptr;
}

HdBprim *
LoFiRenderDelegate::CreateFallbackBprim(TfToken const& typeId)
{
    TF_CODING_ERROR("Creating unknown fallback bprim type=%s", 
        typeId.GetText()); 
    return nullptr;
}

void
LoFiRenderDelegate::DestroyBprim(HdBprim *bPrim)
{
    TF_CODING_ERROR("Destroy Bprim not supported");
}

HdInstancer *
LoFiRenderDelegate::CreateInstancer(
    HdSceneDelegate *delegate,
    SdfPath const& id,
    SdfPath const& instancerId)
{
    TF_CODING_ERROR("Creating Instancer not supported id=%s instancerId=%s", 
        id.GetText(), instancerId.GetText());
    return nullptr;
}

void 
LoFiRenderDelegate::DestroyInstancer(HdInstancer *instancer)
{
    TF_CODING_ERROR("Destroy instancer not supported");
}

PXR_NAMESPACE_CLOSE_SCOPE
