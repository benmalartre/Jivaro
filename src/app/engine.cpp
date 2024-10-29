#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/imaging/cameraUtil/conformWindow.h>
#include <pxr/imaging/hd/light.h>
#include <pxr/imaging/hd/rendererPlugin.h>
#include <pxr/imaging/hd/rendererPluginRegistry.h>
#include <pxr/imaging/hd/systemMessages.h>
#include <pxr/imaging/hdx/pickTask.h>
#include <pxr/imaging/hdx/simpleLightTask.h>
#include <pxr/imaging/hdx/shadowTask.h>
#include <pxr/imaging/hdx/shadowMatrixComputation.h>
#include <pxr/imaging/hgi/tokens.h>

#include "../app/time.h"
#include "../app/engine.h"

JVR_NAMESPACE_OPEN_SCOPE

namespace {
  class ShadowMatrix : public HdxShadowMatrixComputation
  {
  public:
    ShadowMatrix(GlfSimpleLight const& light) {
      GfFrustum frustum;
      frustum.SetProjectionType(GfFrustum::Orthographic);
      frustum.SetWindow(GfRange2d(GfVec2d(-10, -10), GfVec2d(10, 10)));
      frustum.SetNearFar(GfRange1d(0, 100));
      const GfVec4d pos = light.GetPosition();
      frustum.SetPosition(GfVec3d(0, 0, 10));
      frustum.SetRotation(GfRotation(GfVec3d(0, 0, 1),
        GfVec3d(pos[0], pos[1], pos[2])));

      _shadowMatrix =
        frustum.ComputeViewMatrix() * frustum.ComputeProjectionMatrix();
    }

    std::vector<GfMatrix4d> Compute(
      const GfVec4f& viewport,
      CameraUtilConformWindowPolicy policy) override {
      return { _shadowMatrix };
    }

    std::vector<GfMatrix4d> Compute(
      const CameraUtilFraming& framing,
      CameraUtilConformWindowPolicy policy) override {
      return { _shadowMatrix };
    }

  private:
    GfMatrix4d _shadowMatrix;
  };
}

Engine::Engine(HdSceneIndexBaseRefPtr sceneIndex, TfToken plugin)
  : _sceneIndex(sceneIndex),
    _curRendererPlugin(plugin),
    _hgi(Hgi::CreatePlatformDefaultHgi()),
    _hgiDriver{HgiTokens->renderDriver, VtValue(_hgi.get())},
    _engine(),
    _renderDelegate(nullptr),
    _renderIndex(nullptr),
    _taskController(nullptr),
    _taskControllerId("/defaultTaskController"),
    _allowAsynchronousSceneProcessing(true)
{
  _width = 512;
  _height = 512;

  _Initialize();
}

Engine::~Engine()
{
  // Destroy objects in opposite order of construction.
  delete _taskController;

  if (_renderIndex && _sceneIndex) {
    _renderIndex->RemoveSceneIndex(_sceneIndex);
    _sceneIndex = nullptr;
  }

  delete _renderIndex;
  _renderDelegate = nullptr;
}

void Engine::_Initialize()
{

  // init render delegate
  _renderDelegate = _GetRenderDelegateFromPlugin(_curRendererPlugin);

  // init render index
  _renderIndex = HdRenderIndex::New(_renderDelegate.Get(), {&_hgiDriver});

  _renderIndex->InsertSceneIndex(_sceneIndex, _taskControllerId);

  // init task controller

  _taskController = new HdxTaskController(_renderIndex, _taskControllerId);

  // init render paramss
  HdxRenderTaskParams params;
  params.viewport = GfVec4f(0, 0, _width, _height);
  params.enableLighting = true;
  params.enableSceneLights = true;

  _taskController->SetRenderParams(params);

  // init collection
  _collection = HdRprimCollection(HdTokens->geometry,
                                  HdReprSelector(HdReprTokens->smoothHull));
  _taskController->SetCollection(_collection);

  // init render tags
  _taskController->SetRenderTags(TfTokenVector());

  // init AOVs
  TfTokenVector _aovOutputs{HdAovTokens->color};

  _taskController->SetRenderOutputs(_aovOutputs);

  GfVec4f clearColor = GfVec4f(.2f, .2f, .2f, 1.0f);
  HdAovDescriptor colorAovDesc =
      _taskController->GetRenderOutputSettings(HdAovTokens->color);
  if (colorAovDesc.format != HdFormatInvalid) {
    colorAovDesc.clearValue = VtValue(clearColor);
    _taskController->SetRenderOutputSettings( HdAovTokens->color,
                                              colorAovDesc);
  }


  // init selection
  GfVec4f selectionColor = GfVec4f(1.f, 1.f, 0.f, .5f);
  _selTracker = std::make_shared<HdxSelectionTracker>();

  _taskController->SetEnableSelection(true);
  _taskController->SetSelectionColor(selectionColor);

  VtValue selectionValue(_selTracker);
  _engine.SetTaskContextData(HdxTokens->selectionState, selectionValue);

  _taskController->SetOverrideWindowPolicy(CameraUtilFit);
}

TfTokenVector Engine::GetRendererPlugins()
{
  HfPluginDescVector pluginDescriptors;
  HdRendererPluginRegistry::GetInstance().GetPluginDescs(&pluginDescriptors);

  TfTokenVector plugins;
  for (size_t i = 0; i < pluginDescriptors.size(); ++i) {
      plugins.push_back(pluginDescriptors[i].id);
  }
  return plugins;
}

TfToken Engine::GetDefaultRendererPlugin()
{
    HdRendererPluginRegistry& registry =
        HdRendererPluginRegistry::GetInstance();
    return registry.GetDefaultPluginId(true);
}

TfToken Engine::GetCurrentRendererPlugin()
{
    return _curRendererPlugin;
}

HdPluginRenderDelegateUniqueHandle 
Engine::_GetRenderDelegateFromPlugin(TfToken plugin)
{
  HdRendererPluginRegistry& registry =
    HdRendererPluginRegistry::GetInstance();

  TfToken resolvedId = registry.GetDefaultPluginId(true);

  return registry.CreateRenderDelegate(plugin);
}

std::string 
Engine::GetRendererPluginName(TfToken plugin)
{
  HfPluginDesc pluginDescriptor;
  bool foundPlugin = HdRendererPluginRegistry::GetInstance().GetPluginDesc(
    plugin, &pluginDescriptor);

  if (!foundPlugin) { return std::string(); }

  // TODO: fix that will be eventually delegate to Hgi
#if defined(__APPLE__)
  if (pluginDescriptor.id == TfToken("HdStormRendererPlugin")) {
    return "Metal";
  }
#endif

  return pluginDescriptor.displayName;
}

void 
Engine::SetCameraMatrices(GfMatrix4d view, GfMatrix4d proj)
{
  _camView = view;
  _camProj = proj;
}

void 
Engine::SetSelection(SdfPathVector paths)
{
  HdSelectionSharedPtr const selection = std::make_shared<HdSelection>();

  HdSelection::HighlightMode mode = HdSelection::HighlightModeSelect;

  for (auto&& path : paths) {
    SdfPath realPath =
      path.ReplacePrefix(SdfPath::AbsoluteRootPath(), _taskControllerId);
    selection->AddRprim(mode, realPath);
  }

  _selTracker->SetSelection(selection);
}

void
Engine::SetRenderSize(int width, int height)
{
  _width = width;
  _height = height;

  _taskController->SetRenderViewport(GfVec4f(0, 0, width, height));
  _taskController->SetRenderBufferSize(GfVec2i(width, height));

  GfRange2f displayWindow(GfVec2f(0, 0), GfVec2f(width, height));
  GfRect2i renderBufferRect(GfVec2i(0, 0), width, height);
  GfRect2i dataWindow = renderBufferRect.GetIntersection(renderBufferRect);
  CameraUtilFraming framing(displayWindow, dataWindow);

  _taskController->SetFraming(framing);
}

void
Engine::SetRenderViewport(const pxr::GfVec4d &viewport)
{
  _taskController->SetRenderViewport(viewport);
}

void 
Engine::_PrepareDefaultLighting()
{
  // set a animated spot light
  const float t = Time::Get()->GetActiveTime();
  GfVec3d lightPos(pxr::GfSin(t)*10, 8.f, GfCos(t)*10);
  GlfSimpleLight l;
  l.SetAmbient(GfVec4f(0, 0, 0, 0));
  l.SetPosition(GfVec4f(lightPos[0], lightPos[1], lightPos[2], 1));
  l.SetHasShadow(true);

  GlfSimpleMaterial material;
  material.SetAmbient(GfVec4f(2, 2, 2, 1.0));
  material.SetSpecular(GfVec4f(0.1, 0.1, 0.1, 1.0));
  material.SetShininess(32.0);

  GfVec4f sceneAmbient(0.01, 0.01, 0.01, 1.0);

  GlfSimpleLightingContextRefPtr lightingContextState =
      GlfSimpleLightingContext::New();

  lightingContextState->SetLights({l});
  lightingContextState->SetMaterial(material);
  lightingContextState->SetSceneAmbient(sceneAmbient);
  lightingContextState->SetUseLighting(true);
  _taskController->SetLightingState(lightingContextState);

  // try enabling shadow 
  _taskController->SetEnableShadows(true);

  HdxShadowParams shadowParams;
  shadowParams.enabled = true;
  shadowParams.resolution = 512;
  shadowParams.shadowMatrix
      = HdxShadowMatrixComputationSharedPtr(new ShadowMatrix(l));
  shadowParams.bias = -0.001;
  shadowParams.blur = 0.1;
  const VtValue vtShadowParams(shadowParams);

  _engine.SetTaskContextData(HdLightTokens->shadowParams, vtShadowParams);

}

void 
Engine::Prepare()
{
  _PrepareDefaultLighting();
  _taskController->SetFreeCameraMatrices(_camView, _camProj);
}

void
Engine::Render()
{
  HdTaskSharedPtrVector tasks = _taskController->GetRenderingTasks();
  _engine.Execute(_renderIndex, &tasks);
}

bool
Engine::IsConverged() const
{
  if (ARCH_UNLIKELY(!_renderDelegate)) {
    return true;
  }

  return _taskController->IsConverged();
}


SdfPath 
Engine::FindIntersection(GfVec2f screenPos)
{
  // create a narrowed frustum on the given position
  float normalizedXPos = screenPos[0] / _width;
  float normalizedYPos = screenPos[1] / _height;

  GfVec2d size(1.0 / _width, 1.0 / _height);

  GfCamera gfCam;
  gfCam.SetFromViewAndProjectionMatrix(_camView, _camProj);
  GfFrustum frustum = gfCam.GetFrustum();

  auto nFrustum = frustum.ComputeNarrowedFrustum(
    GfVec2d(2.0 * normalizedXPos - 1.0,
            2.0 * (1.0 - normalizedYPos) - 1.0),
    size);

  // check the intersection from the narrowed frustum
  HdxPickHitVector allHits;
  HdxPickTaskContextParams pickParams;
  pickParams.resolveMode = HdxPickTokens->resolveNearestToCenter;
  pickParams.viewMatrix = nFrustum.ComputeViewMatrix();
  pickParams.projectionMatrix = nFrustum.ComputeProjectionMatrix();
  pickParams.collection = _collection;
  pickParams.outHits = &allHits;
  const VtValue vtPickParams(pickParams);

  _engine.SetTaskContextData(HdxPickTokens->pickParams, vtPickParams);

  // render with the picking task
  HdTaskSharedPtrVector tasks = _taskController->GetPickingTasks();
  _engine.Execute(_renderIndex, &tasks);

  // get the hitting point
  if (allHits.size() != 1) return SdfPath();

  const SdfPath path = allHits[0].objectId.ReplacePrefix(
    _taskControllerId, SdfPath::AbsoluteRootPath());

  return path;
}

GfFrustum 
Engine::GetFrustum()
{
  GfCamera gfCam;
  gfCam.SetFromViewAndProjectionMatrix(_camView, _camProj);
  return gfCam.GetFrustum();
}

bool
Engine::PollForAsynchronousUpdates() const
{
  class _Observer : public HdSceneIndexObserver
  {
  public:
    void PrimsAdded(
      const HdSceneIndexBase &sender,
      const AddedPrimEntries &entries) override
    {

      _changed = true;
    }

    void PrimsRemoved(
      const HdSceneIndexBase &sender,
      const RemovedPrimEntries &entries) override
    {
      _changed = true;
    }

    void PrimsDirtied(
      const HdSceneIndexBase &sender,
      const DirtiedPrimEntries &entries) override
    {
      _changed = true;
    }

    void PrimsRenamed(
      const HdSceneIndexBase &sender,
      const RenamedPrimEntries &entries) override
    {
      _changed = true;
    }

    bool IsChanged() { return _changed; }
  private:
    bool _changed = false;
  };

  if (_allowAsynchronousSceneProcessing && _renderIndex) {
    if (HdSceneIndexBaseRefPtr si = _renderIndex->GetTerminalSceneIndex()) {
      _Observer ob;
      si->AddObserver(HdSceneIndexObserverPtr(&ob));
      si->SystemMessage(HdSystemMessageTokens->asyncPoll, nullptr);
      si->RemoveObserver(HdSceneIndexObserverPtr(&ob));
      return ob.IsChanged();
    }
  }
  return false;
}



PXR_NAMESPACE_CLOSE_SCOPE