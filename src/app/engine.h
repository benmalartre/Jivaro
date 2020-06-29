#ifndef AMN_APPLICATION_ENGINE_H
#define AMN_APPLICATION_ENGINE_H

#include "../common.h"
#include "pxr/pxr.h"

#include "pxr/base/tf/errorMark.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/base/gf/frustum.h"
#include "pxr/base/gf/rotation.h"

AMN_NAMESPACE_OPEN_SCOPE
void RunHydra();

class Engine {
  //pxr::TfToken              _rendererPluginId;
  /*
  pxr::HdRendererPlugin*    _rendererPlugin;
  pxr::HdRenderDelegate*    _renderDelegate;
  pxr::HdRenderIndex*       _renderIndex;
  pxr::HdUnitTestDelegate*  _sceneDelegate;
  */
};

AMN_NAMESPACE_CLOSE_SCOPE
#endif