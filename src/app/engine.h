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
#include "../app/delegate.h"

#include <memory>

JVR_NAMESPACE_OPEN_SCOPE

using HgiUniquePtr = std::unique_ptr<class Hgi>;

class Engine {
public:
  Engine(HdSceneIndexBaseRefPtr sceneIndex, TfToken plugin);
  ~Engine();

  // ---------------------------------------------------------------------
  /// \name Rendering
  /// @{
  // ---------------------------------------------------------------------
  void Prepare();
  void Render();
  bool IsConverged() const;
  void Present();


  static TfTokenVector GetRendererPlugins();
  static TfToken GetDefaultRendererPlugin();
  std::string GetRendererPluginName(TfToken plugin);
  TfToken GetCurrentRendererPlugin();
  void SetCameraMatrices(GfMatrix4d view, GfMatrix4d proj);
  void SetSelection(SdfPathVector paths);
  void SetRenderSize(int width, int height);
  void SetRenderViewport(const pxr::GfVec4d &viewport);

  SdfPath FindIntersection(GfVec2f screenPos);

  inline bool IsDirty() { return _dirty; };
  inline void SetDirty(bool dirty) { _dirty = dirty; };

  void InitExec(Scene* scene);
  void UpdateExec(double time);
  void TerminateExec();

  void ActivateShadows(bool active);

  void SetHighlightSelection(bool state) { _highlightSelection = state; };
  bool GetHighlightSelection() { return _highlightSelection; };

  Delegate* GetDelegate() { return _delegate; };
  GfFrustum GetFrustum();

protected:
  static HdPluginRenderDelegateUniqueHandle 
    _GetRenderDelegateFromPlugin(TfToken plugin);

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

  bool                                _dirty;
  bool                                _highlightSelection;

  Delegate*                           _delegate;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif