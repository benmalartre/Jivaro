#include "pxr/imaging/glf/glew.h"
#include "pxr/imaging/glf/contextCaps.h"
#include "pxr/imaging/glf/glContext.h"
#include "pxr/base/tf/envSetting.h"
#include "pxr/base/tf/diagnostic.h"
#include "pxr/base/tf/callContext.h"
#include "engine.h"

#include <iostream>

AMN_NAMESPACE_OPEN_SCOPE

PXR_NAMESPACE_USING_DIRECTIVE

TF_DEFINE_ENV_SETTING(AMN_ENGINE_DEBUG_SCENE_DELEGATE_ID, "/",
  "Default Amnesia scene delegate id");

pxr::SdfPath const&
_GetUsdImagingDelegateId()
{
  static pxr::SdfPath const delegateId =
    pxr::SdfPath(pxr::TfGetEnvSetting(AMN_ENGINE_DEBUG_SCENE_DELEGATE_ID));

  return delegateId;
}

void _InitGL()
{
  static std::once_flag initFlag;

  std::call_once(initFlag, [] {

    // Initialize Glew library for GL Extensions if needed
    pxr::GlfGlewInit();

    // Initialize if needed and switch to shared GL context.
    pxr::GlfSharedGLContextScopeHolder sharedContext;

    // Initialize GL context caps based on shared context
    pxr::GlfContextCaps::InitInstance();

  });
}

Engine::Engine(const pxr::HdDriver& driver)
  : Engine(pxr::SdfPath::AbsoluteRootPath(),
    {},
    {},
    _GetUsdImagingDelegateId(),
    driver
  )
{
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
{
  _InitGL();

  if (IsHydraEnabled()) {

    // _renderIndex, _taskController, and _sceneDelegate are initialized
    // by the plugin system.
    if (!SetRendererPlugin(_GetDefaultRendererPluginId())) {
      TF_CODING_ERROR("No renderer plugins found! "
        "Check before creation.");
    }

  }
  else {
    TF_CODING_ERROR("Hydra is NOT supported! ");
  }
}

Engine::~Engine() = default;

AMN_NAMESPACE_CLOSE_SCOPE
