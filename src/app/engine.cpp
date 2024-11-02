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
#include <pxr/imaging/hdx/taskController.h>
#include <pxr/imaging/hgi/tokens.h>

#include "../app/time.h"
#include "../app/engine.h"
#include "../app/application.h"

JVR_NAMESPACE_OPEN_SCOPE

namespace {
class ShadowMatrix : public HdxShadowMatrixComputation
{
public:
  ShadowMatrix(GlfSimpleLight const &light) {
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
    const GfVec4f &viewport,
    CameraUtilConformWindowPolicy policy) override {
      return { _shadowMatrix };
  }

  std::vector<GfMatrix4d> Compute(
    const CameraUtilFraming &framing,
    CameraUtilConformWindowPolicy policy) override {
      return { _shadowMatrix };
  }

private:
    GfMatrix4d _shadowMatrix;
};
}

Engine::Engine(HdSceneIndexBaseRefPtr sceneIndex, TfToken plugin)
  : _sceneIndex(sceneIndex)
  , _rendererPlugin(plugin)
  , _hgi(Hgi::CreatePlatformDefaultHgi())
  , _hgiDriver{HgiTokens->renderDriver, VtValue(_hgi.get())}
  , _engine(nullptr)
  , _renderDelegate(nullptr)
  , _renderIndex(nullptr)
  , _taskController(nullptr)
  , _taskControllerId("/taskController")
  , _allowAsynchronousSceneProcessing(true)
  , _lightingContext(nullptr)
{
  _width = 512;
  _height = 512;
  _params.drawMode = Engine::DRAW_SHADED_SMOOTH;
  _params.complexity = 1.0f;

  _Initialize();
}

Engine::~Engine()
{
  _engine = nullptr;
  _taskController = nullptr;
  if (_renderIndex && _sceneIndex) {
    _renderIndex->RemoveSceneIndex(_sceneIndex);
  }
  
  _renderIndex = nullptr;
  
  _renderDelegate = nullptr;
}

void Engine::_Initialize()
{
  // init render delegate
  _renderDelegate = _GetRenderDelegateFromPlugin(_rendererPlugin);

  // Use the render delegate ptr (rather than 'this' ptr) for generating the unique id.
  const std::string renderInstanceId =
    TfStringPrintf("Engine_%s_%p",
      _renderDelegate.GetPluginId().GetText(),
      (void*)_renderDelegate.Get());

  // init render index
  _renderIndex.reset(HdRenderIndex::New(
    _renderDelegate.Get(), {&_hgiDriver}, renderInstanceId));

  _renderIndex->InsertSceneIndex(_sceneIndex, _taskControllerId);

  // init task controller

  _taskController = std::make_unique<HdxTaskController>(_renderIndex.get(), _taskControllerId);
  _engine = std::make_unique<HdEngine>();

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
  _engine->SetTaskContextData(HdxTokens->selectionState, selectionValue);

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
    return _rendererPlugin;
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

  // get default lights
  _lights.resize(3);

  // set a animated spot light
  const float t = Time::Get()->GetActiveTime();

  for (size_t i = 0; i < 3; ++i) {
    const GfVec3d lightPos(GfSin(i + (t *0.1)) * 20.f, 20.f, GfCos(i + (t *0.1))*20.f);
    const pxr::GfVec3f direction(-lightPos.GetNormalized());

    SdfPath defaultLightId = _taskController->GetControllerId().AppendChild(TfToken("light" +std::to_string(i)));
    _lights[i].SetID(defaultLightId);
    _lights[i].SetAmbient(GfVec4f(GfSin(i + (t *0.1)),0.5f,GfCos(i + (t *0.1)), 0));
    _lights[i].SetPosition(GfVec4f(lightPos[0], lightPos[1], lightPos[2], 1));
    _lights[i].SetSpotDirection(direction);
    _lights[i].SetIsDomeLight(false);

    _lights[i].SetHasShadow(true);
    _lights[i].SetShadowResolution(512);
    _lights[i].SetShadowBlur(0.25f);
    _lights[i].SetShadowBias(-0.005f);

  }
  if (!_lightingContext) {
    _lightingContext = GlfSimpleLightingContext::New();
  }

  GlfSimpleMaterial material;
  material.SetAmbient(GfVec4f(0.2,0.2,0.2, 1.0));
  material.SetSpecular(GfVec4f(0.1, 0.1, 0.1, 1.0));
  material.SetShininess(32.0);

  GfVec4f sceneAmbient(0.01, 0.01, 0.01, 1.0);

  _lightingContext->SetLights(_lights);
  _lightingContext->SetMaterial(material);
  _lightingContext->SetSceneAmbient(sceneAmbient);
  _lightingContext->SetUseLighting(true);
  _taskController->SetLightingState(_lightingContext);

    // try enabling shadow 
  _taskController->SetEnableShadows(true);

}

void 
Engine::Prepare()
{
  _PrepareDefaultLighting();
  _taskController->ComputeInverseProjectionViewMatrix(_camView, _camProj);
  _taskController->SetFreeCameraMatrices(_camView, _camProj);

/*
  if (_materialPruningSceneIndex) {
    _materialPruningSceneIndex->SetEnabled(
      !params.enableSceneMaterials);
  }
  if (_lightPruningSceneIndex) {
    _lightPruningSceneIndex->SetEnabled(
      !params.enableSceneLights);
  }
*/
}

void
Engine::Render()
{
  _params.enableLighting = RANDOM_0_1 > 0.5;
  _params.enableSceneLights = RANDOM_0_1 > 0.5;
  _params.cullStyle = (HdCullStyle)RANDOM_0_X(6);

  _UpdateHydraCollection(&_collection, { SdfPath::AbsoluteRootPath() }, _params);
  _taskController->SetCollection(_collection);

  // init render paramss
  HdxRenderTaskParams params;
  params.enableLighting = true;
  params.enableSceneLights = true;
  //params.cullStyle = _params.cullStyle;

  _taskController->SetRenderParams(params);

  HdTaskSharedPtrVector tasks = _taskController->GetRenderingTasks();
  _engine->Execute(_renderIndex.get(), &tasks);
}

bool
Engine::IsConverged() const
{
  if (ARCH_UNLIKELY(!_renderDelegate)) {
    return true;
  }

  return _taskController->IsConverged();
}


bool 
Engine::TestIntersection(
  const GfMatrix4d& frustumView,
  const GfMatrix4d& frustumProj,
  const SdfPath& root, 
  Engine::PickHit* hit)
{
  // check the intersection from the narrowed frustum
  HdxPickHitVector allHits;
  HdxPickTaskContextParams pickParams;
  pickParams.resolveMode = HdxPickTokens->resolveNearestToCenter;
  pickParams.viewMatrix = frustumView;
  pickParams.projectionMatrix = frustumProj;
  pickParams.collection = _collection;
  pickParams.outHits = &allHits;

  const VtValue vtPickParams(pickParams);

  _engine->SetTaskContextData(HdxPickTokens->pickParams, vtPickParams);

  // render with the picking task
  HdTaskSharedPtrVector tasks = _taskController->GetPickingTasks();
  _engine->Execute(_renderIndex.get(), &tasks);

  // did we hit something
  if (allHits.size() != 1) return false;

  if(hit) {
    hit->objectId = allHits[0].objectId.ReplacePrefix(
      _taskControllerId, SdfPath::AbsoluteRootPath());
    hit->hitPoint = allHits[0].worldSpaceHitPoint;
    hit->hitNormal = allHits[0].worldSpaceHitNormal;
    hit->hitNormalizedDepth = allHits[0].normalizedDepth;
  }

  return true;
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



/* static */
bool
Engine::_UpdateHydraCollection(
  HdRprimCollection *collection,
  SdfPathVector const& roots,
  RenderParams const& params)
{
  if (collection == nullptr) {
    TF_CODING_ERROR("Null passed to _UpdateHydraCollection");
    return false;
  }

  // choose repr
  HdReprSelector reprSelector = HdReprSelector(HdReprTokens->smoothHull);
  const bool refined = params.complexity > 1.0;
  
  if (params.drawMode == Engine::DRAW_POINTS) {
    reprSelector = HdReprSelector(HdReprTokens->points);
  } else if (params.drawMode == Engine::DRAW_SHADED_FLAT ||
    params.drawMode == Engine::DRAW_SHADED_FLAT) {
    // Flat shading
    reprSelector = HdReprSelector(HdReprTokens->hull);
  } else if (
    params.drawMode == Engine::DRAW_WIREFRAME_ON_SHADED) {
    // Wireframe on surface
    reprSelector = HdReprSelector(refined ?
      HdReprTokens->refinedWireOnSurf : HdReprTokens->wireOnSurf);
  } else if (params.drawMode == Engine::DRAW_WIREFRAME) {
    // Wireframe
    reprSelector = HdReprSelector(refined ?
      HdReprTokens->refinedWire : HdReprTokens->wire);
  } else {
    // Smooth shading
    reprSelector = HdReprSelector(refined ?
      HdReprTokens->refined : HdReprTokens->smoothHull);
  }

  // By default our main collection will be called geometry
  const TfToken colName = HdTokens->geometry;

  // Check if the collection needs to be updated (so we can avoid the sort).
  SdfPathVector const& oldRoots = collection->GetRootPaths();

  // inexpensive comparison first
  bool match = collection->GetName() == colName &&
               oldRoots.size() == roots.size() &&
               collection->GetReprSelector() == reprSelector;

  // Only take the time to compare root paths if everything else matches.
  if (match) {
    // Note that oldRoots is guaranteed to be sorted.
    for(size_t i = 0; i < roots.size(); i++) {
      // Avoid binary search when both vectors are sorted.
      if (oldRoots[i] == roots[i])
          continue;
      // Binary search to find the current root.
      if (!std::binary_search(oldRoots.begin(), oldRoots.end(), roots[i])) 
      {
        match = false;
        break;
      }
    }

      // if everything matches, do nothing.
      if (match) return false;
  }

  // Recreate the collection.
  *collection = HdRprimCollection(colName, reprSelector);
  collection->SetRootPaths(roots);

  return true;
}



PXR_NAMESPACE_CLOSE_SCOPE