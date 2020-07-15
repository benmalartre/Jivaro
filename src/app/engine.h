#ifndef AMN_APPLICATION_ENGINE_H
#define AMN_APPLICATION_ENGINE_H

#include "../common.h"
#include "pxr/pxr.h"
#include "pxr/imaging/hd/driver.h"
#include "pxr/imaging/hd/engine.h"
#include "pxr/imaging/hd/rprimCollection.h"
#include "pxr/imaging/hd/renderDelegate.h"
#include "pxr/imaging/hd/renderIndex.h"
#include "pxr/imaging/hd/rendererPlugin.h"
#include "pxr/imaging/hd/pluginRenderDelegateUniqueHandle.h"


#include "pxr/imaging/hdx/taskController.h"
#include "pxr/imaging/hdx/selectionTracker.h"
#include "pxr/imaging/hgi/hgi.h"
#include "pxr/usdImaging/usdImagingGL/engine.h"
#include "pxr/usdImaging/usdImagingGL/renderParams.h"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/errorMark.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/base/gf/frustum.h"
#include "pxr/base/gf/rotation.h"

#include <memory>

AMN_NAMESPACE_OPEN_SCOPE
void RunHydra();

class Engine {
public:
  Engine();
  ~Engine();

  bool SetRendererPlugin(const pxr::TfToken& rendererPluginId);
  void UpdateCameraState(const pxr::GfMatrix4d &viewMatrix,
    const pxr::GfMatrix4d &projMatrix);

protected:
  pxr::UsdImagingGLEngine*          _engine;
  pxr::UsdImagingGLRenderParams     _renderParams;

private:
  pxr::TfToken                             _rendererPluginId;
  pxr::HdRendererPlugin*                   _rendererPlugin;

};

AMN_NAMESPACE_CLOSE_SCOPE
#endif