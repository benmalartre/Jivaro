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
#include "../geometry/intersection.h"
#include "../app/handle.h"
#include "../app/delegate.h"

#include <memory>

JVR_NAMESPACE_OPEN_SCOPE

class Engine : public UsdImagingGLEngine {
public:
  Engine(const HdDriver& driver);
  Engine( const SdfPath& rootPath,
          const SdfPathVector& excludedPaths,
          const SdfPathVector& invisedPaths = SdfPathVector(),
          const SdfPath& sceneDelegateID =
          SdfPath::AbsoluteRootPath(),
          const HdDriver& driver = HdDriver());
  ~Engine();

  inline bool IsDirty() { return _dirty;};
  inline void SetDirty(bool dirty) { _dirty = dirty; };

  void InitExec(Scene* scene);
  void UpdateExec(double time);
  void TerminateExec();

  /*
  HdSelectionSharedPtr _Pick(GfVec2i const& startPos, 
    GfVec2i const& endPos, TfToken const& pickTarget);
    */
  bool TestIntersection(
    const GfMatrix4d& viewMatrix,
    const GfMatrix4d& projectionMatrix,
    const UsdPrim& root,
    const UsdImagingGLRenderParams& params,
    GfVec3d* outHitPoint,
    GfVec3d* outHitNormal,
    SdfPath* outHitPrimPath = NULL,
    SdfPath* outHitInstancerPath = NULL,
    int* outHitInstanceIndex = NULL,
    HdInstancerContext* outInstancerContext = NULL);

  /// Decodes a pick result given hydra prim ID/instance ID (like you'd get
  /// from an ID render).
  bool DecodeIntersection(
    unsigned char const primIdColor[4],
    unsigned char const instanceIdColor[4],
    SdfPath* outHitPrimPath = NULL,
    SdfPath* outHitInstancerPath = NULL,
    int* outHitInstanceIndex = NULL,
    HdInstancerContext* outInstancerContext = NULL);

  void SetHighlightSelection(bool state) { _highlightSelection = state; };
  bool GetHighlightSelection() { return _highlightSelection; };

  void SetSceneColReprSelector(HdReprSelector const &reprSelector) {
    _collection.SetReprSelector(reprSelector);
    SdfPath renderTask("/renderTask");
    _delegate->SetTaskParam(renderTask, HdTokens->collection,
        VtValue(_collection));
  }

  Delegate* GetDelegate() { return _delegate; };

protected:
  const HdRprimCollection &_GetCollection() const { return _collection; };
  bool _CheckPrimSelectable(const SdfPath &path);

private:
  bool                    _dirty;
  bool                    _highlightSelection;
  Delegate*               _delegate;
  HdRprimCollection  _collection;
  //std::vector<View*> _views;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif