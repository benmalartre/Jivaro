#include "pxr/imaging/garch/glApi.h"
#include "pxr/imaging/glf/contextCaps.h"
#include "pxr/imaging/glf/glContext.h"
#include "pxr/imaging/hdx/taskController.h"
#include "pxr/imaging/hd/mesh.h"
#include "pxr/usdImaging/usdImaging/delegate.h"
#include "pxr/base/tf/envSetting.h"
#include "pxr/base/tf/diagnostic.h"
#include "pxr/base/tf/callContext.h"
#include "engine.h"

#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_ENV_SETTING(ENGINE_DEBUG_SCENE_DELEGATE_ID, "/",
  "Default Jivaro scene delegate id");

pxr::SdfPath const&
_GetUsdImagingDelegateId()
{
  static pxr::SdfPath const delegateId =
    pxr::SdfPath(pxr::TfGetEnvSetting(ENGINE_DEBUG_SCENE_DELEGATE_ID));

  return delegateId;
}

void _InitGL()
{
  static std::once_flag initFlag;

  std::call_once(initFlag, [] {

    // Initialize GL library for GL Extensions if needed
    GarchGLApiLoad();

    // Initialize if needed and switch to shared GL context.
    pxr::GlfSharedGLContextScopeHolder sharedContext;

    // Initialize GL context caps based on shared context
    pxr::GlfContextCaps::InitInstance();

  });
}

Engine::Engine(const pxr::HdDriver& driver)
  : Engine(pxr::SdfPath::AbsoluteRootPath(), {}, {}, 
    _GetUsdImagingDelegateId(), driver)
{
  //SetRendererAov()
}


Engine::Engine(
  const pxr::SdfPath& rootPath,
  const pxr::SdfPathVector& excludedPaths,
  const pxr::SdfPathVector& invisedPaths,
  const pxr::SdfPath& sceneDelegateID,
  const pxr::HdDriver& driver)
  : pxr::UsdImagingGLEngine(
    rootPath,
    excludedPaths,
    invisedPaths,
    sceneDelegateID,
    driver)
  , _dirty(true)
{
}

Engine::~Engine() = default;


PXR_NAMESPACE_CLOSE_SCOPE
