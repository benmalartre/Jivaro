#ifndef JVR_APPLICATION_ENGINE_H
#define JVR_APPLICATION_ENGINE_H


#include <pxr/pxr.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/imaging/hd/driver.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>

#include <pxr/base/tf/token.h>
#include <pxr/base/tf/errorMark.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/rotation.h>

#include "../common.h"
#include "../app/handle.h"

#include <memory>

PXR_NAMESPACE_OPEN_SCOPE

class Engine : public pxr::UsdImagingGLEngine {
public:
  Engine(const pxr::HdDriver& driver);
  Engine( const pxr::SdfPath& rootPath,
          const pxr::SdfPathVector& excludedPaths,
          const pxr::SdfPathVector& invisedPaths = pxr::SdfPathVector(),
          const pxr::SdfPath& sceneDelegateID =
          pxr::SdfPath::AbsoluteRootPath(),
          const pxr::HdDriver& driver = pxr::HdDriver());
  ~Engine();

  /*
  pxr::HdSelectionSharedPtr _Pick(pxr::GfVec2i const& startPos, 
    pxr::GfVec2i const& endPos, pxr::TfToken const& pickTarget);
    */
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif