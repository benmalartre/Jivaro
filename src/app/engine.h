#ifndef JVR_APPLICATION_ENGINE_H
#define JVR_APPLICATION_ENGINE_H

#include <pxr/base/tf/token.h>
#include <pxr/imaging/glf/drawTarget.h>
#include <pxr/imaging/hd/engine.h>
#include <pxr/imaging/hd/pluginRenderDelegateUniqueHandle.h>
#include <pxr/imaging/hd/renderDelegate.h>
#include <pxr/imaging/hd/sceneIndex.h>
#include <pxr/imaging/hdx/taskController.h>
#include <pxr/imaging/hgi/hgi.h>
#include <pxr/imaging/hgiInterop/hgiInterop.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>

#include "../common.h"
#include "../geometry/intersection.h"
#include "../app/handle.h"

#include <memory>

JVR_NAMESPACE_OPEN_SCOPE

using HgiUniquePtr = std::unique_ptr<class Hgi>;

class Engine {
public:
  enum DrawMode {
    DRAW_POINTS,
    DRAW_GEOM_FLAT,
    DRAW_SHADED_FLAT,
    DRAW_WIREFRAME,
    DRAW_WIREFRAME_ON_SURFACE
  };

  struct RenderParams {
    float complexity;
    short drawMode;
  };

  struct PickHit {
      SdfPath objectId;

      SdfPath instancerId;
      int     instanceIndex;

      GfVec3d hitPoint;
      GfVec3f hitNormal;

      float   hitNormalizedDepth;
  };

  Engine(HdSceneIndexBaseRefPtr sceneIndex, TfToken plugin);
  ~Engine();

  // ---------------------------------------------------------------------
  /// \name Rendering
  /// @{
  // ---------------------------------------------------------------------
  void Prepare();
  void Render();
  bool IsConverged() const;


  static TfTokenVector GetRendererPlugins();
  static TfToken GetDefaultRendererPlugin();
  std::string GetRendererPluginName(TfToken plugin);
  TfToken GetCurrentRendererPlugin();
  void SetCameraMatrices(GfMatrix4d view, GfMatrix4d proj);
  void SetSelection(SdfPathVector paths);
  void SetRenderSize(int width, int height);
  void SetRenderViewport(const pxr::GfVec4d &viewport);

  bool TestIntersection(
    const GfMatrix4d& frustumView,
    const GfMatrix4d& frustumProj,
    const SdfPath& root, 
    PickHit* hit
  );

  void ActivateShadows(bool active);

  void SetHighlightSelection(bool state) { _highlightSelection = state; };
  bool GetHighlightSelection() { return _highlightSelection; };

  GfFrustum GetFrustum();

  bool PollForAsynchronousUpdates() const;

protected:
  static HdPluginRenderDelegateUniqueHandle 
    _GetRenderDelegateFromPlugin(TfToken plugin);

  static bool 
    _UpdateHydraCollection( HdRprimCollection *collection,
                            SdfPathVector const& roots,
                            RenderParams const& params  );

  void _Initialize();
  void _PrepareDefaultLighting();
  bool _CheckPrimSelectable(const SdfPath& path);

private:
  UsdStageRefPtr                      _stage;
  GfMatrix4d                          _camView;
  GfMatrix4d                          _camProj;
  int                                 _width;
  int                                 _height;

  HgiUniquePtr                        _hgi;
  HdDriver                            _hgiDriver;

  HdEngine                            _engine;
  HdPluginRenderDelegateUniqueHandle  _renderDelegate;
  HdRenderIndex*                      _renderIndex;
  HdxTaskController*                  _taskController;
  HdRprimCollection                   _collection;
  HdSceneIndexBaseRefPtr              _sceneIndex;
  SdfPath                             _taskControllerId;

  HgiInterop                          _interop;
  HdxSelectionTrackerSharedPtr        _selTracker;

  TfToken                             _curRendererPlugin;

  bool                                _highlightSelection;
  bool                                _allowAsynchronousSceneProcessing;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif