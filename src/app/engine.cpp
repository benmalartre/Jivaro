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

    // Initialize GL library for GL Extensions if needed
    GarchGLApiLoad();

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

    /*
    // render task parameters.
    VtValue vParam = _delegate->GetTaskParam(renderSetupTask, HdTokens->params);
    HdxRenderTaskParams param = vParam.Get<HdxRenderTaskParams>();
    param.enableLighting = true; // use default lighting
    _delegate->SetTaskParam(renderSetupTask, HdTokens->params,
                            VtValue(param));
    // Use wireframe and enable points for edge and point picking.
    const auto sceneReprSel = HdReprSelector(HdReprTokens->wireOnSurf,
                                             HdReprTokens->disabled,
                                             _tokens->meshPoints);
    _delegate->SetTaskParam(renderTask, HdTokens->collection,
                            VtValue(HdRprimCollection(HdTokens->geometry, 
                                                      sceneReprSel)));
    */

    pxr::TfToken pointsRepr("meshPoints");
    // Add a meshPoints repr since it isn't populated in 
    // HdRenderIndex::_ConfigureReprs
    pxr::HdMesh::ConfigureRepr(
      pointsRepr,
      pxr::HdMeshReprDesc(
        HdMeshGeomStylePoints,
        HdCullStyleNothing,
        HdMeshReprDescTokens->pointColor,
        /*flatShadingEnabled=*/true,
        /*blendWireframeColor=*/false
      )
    );

    pxr::HdxTaskController* taskController = _GetTaskController();
    pxr::HdTaskSharedPtrVector renderTasks = taskController->GetRenderingTasks();

    const auto sceneReprSel = HdReprSelector(
      HdReprTokens->wireOnSurf,
      HdReprTokens->disabled,
      pointsRepr
    );
    _GetSceneDelegate()->SetReprFallback(sceneReprSel);


    for (auto& renderTask : renderTasks) {
      std::cout << "RENDER TASK : " << renderTask->GetId() << std::endl;
      

    }
    /*
    pxr::UsdImagingDelegate* sceneDelegate = _GetSceneDelegate();
    sceneDelegate->
    */
  }
  else {
    TF_CODING_ERROR("Hydra is NOT supported! ");
  }
}

Engine::~Engine() = default;

AMN_NAMESPACE_CLOSE_SCOPE
