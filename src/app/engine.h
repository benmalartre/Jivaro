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

JVR_NAMESPACE_OPEN_SCOPE

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

  inline bool IsDirty() { return _dirty;};
  inline void SetDirty(bool dirty) { _dirty = dirty; };
  /*
  pxr::HdSelectionSharedPtr _Pick(pxr::GfVec2i const& startPos, 
    pxr::GfVec2i const& endPos, pxr::TfToken const& pickTarget);
    */
  bool TestIntersection(
    const pxr::GfMatrix4d& viewMatrix,
    const pxr::GfMatrix4d& projectionMatrix,
    const pxr::UsdPrim& root,
    const pxr::UsdImagingGLRenderParams& params,
    pxr::GfVec3d* outHitPoint,
    pxr::GfVec3d* outHitNormal,
    pxr::SdfPath* outHitPrimPath = NULL,
    pxr::SdfPath* outHitInstancerPath = NULL,
    int* outHitInstanceIndex = NULL,
    pxr::HdInstancerContext* outInstancerContext = NULL);

  /// Decodes a pick result given hydra prim ID/instance ID (like you'd get
  /// from an ID render).
  bool DecodeIntersection(
    unsigned char const primIdColor[4],
    unsigned char const instanceIdColor[4],
    pxr::SdfPath* outHitPrimPath = NULL,
    pxr::SdfPath* outHitInstancerPath = NULL,
    int* outHitInstanceIndex = NULL,
    pxr::HdInstancerContext* outInstancerContext = NULL);

  void SetHighlightSelection(bool state) { _highlightSelection = state; };
  bool GetHighlightSelection() { return _highlightSelection; };

private:
  bool _dirty;
  bool _highlightSelection;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif