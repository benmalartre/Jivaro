#include <iostream>
#include <string>
#include <pxr/base/tf/debug.h>
#include <pxr/base/tf/refPtr.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/layerStateDelegate.h>
#include <pxr/base/tf/refPtr.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/imaging/hd/renderPassState.h>
#include <pxr/imaging/LoFi/debugCodes.h>

#include "../utils/files.h"
#include "../utils/prefs.h"
#include "../ui/fileBrowser.h"
#include "../ui/viewport.h"
#include "../ui/menu.h"
#include "../ui/popup.h"
#include "../ui/graphEditor.h"
#include "../ui/timeline.h"
#include "../ui/demo.h"
#include "../ui/toolbar.h"
#include "../ui/explorer.h"
#include "../ui/layers.h"
#include "../ui/layerEditor.h"
#include "../ui/propertyEditor.h"
#include "../ui/curveEditor.h"

#include "../app/application.h"
#include "../app/commands.h"
#include "../app/modal.h"
#include "../app/notice.h"
#include "../app/handle.h"
#include "../app/engine.h"
#include "../app/selection.h"
#include "../app/scene.h"
#include "../app/window.h"
#include "../app/view.h"
#include "../app/camera.h"
#include "../app/tools.h"

JVR_NAMESPACE_OPEN_SCOPE

Application* APPLICATION = nullptr;
const char* Application::APPLICATION_NAME = "Jivaro";


// constructor
//----------------------------------------------------------------------------
Application::Application(unsigned width, unsigned height):
  _mainWindow(nullptr), _activeWindow(nullptr), _popup(nullptr), _execute(false)
{  
  _mainWindow = CreateStandardWindow(width, height);
  _activeWindow = _mainWindow;
  _time.Init(1, 101, 24);
  
};

Application::Application(bool fullscreen):
  _mainWindow(nullptr), _activeWindow(nullptr), _popup(nullptr), _execute(false)
{
  _mainWindow = CreateFullScreenWindow();
  _activeWindow = _mainWindow;
  _time.Init(1, 101, 24);
};

// destructor
//----------------------------------------------------------------------------
Application::~Application()
{
  if(_mainWindow) delete _mainWindow;
};

// create full screen window
//----------------------------------------------------------------------------
Window*
Application::CreateFullScreenWindow()
{
  return Window::CreateFullScreenWindow();
}

// create child window
//----------------------------------------------------------------------------
Window*
Application::CreateChildWindow(
  int x, int y, int width, int height, Window* parent,
  const std::string& name, bool decorated)
{
  return
    Window::CreateChildWindow(x, y, width, height,
      parent->GetGlfwWindow(), name, decorated);
}

// create standard window
//----------------------------------------------------------------------------
Window*
Application::CreateStandardWindow(int width, int height)
{
  return Window::CreateStandardWindow(width, height);
}

// popup
//----------------------------------------------------------------------------
void
Application::SetPopup(PopupUI* popup)
{
  popup->SetParent(GetActiveWindow()->GetMainView());
  _popup = popup;
  _mainWindow->CaptureFramebuffer();
  for (auto& childWindow : _childWindows)childWindow->CaptureFramebuffer();
}

/*
void
Application::SetPopupDeferred(PopupUI* popup)
{
  popup->SetParent(GetActiveWindow()->GetMainView());
  _popup = popup;
  _needCaptureFramebuffers = true;
}
*/


void
Application::UpdatePopup()
{
  if (_popup) {
    if (!_popup->IsDone())return;
    _popup->Terminate();
    delete _popup;
  }
  _popup = nullptr;
  _mainWindow->ForceRedraw();
  for (auto& childWindow : _childWindows)childWindow->ForceRedraw();
}

void
Application::AddDeferredCommand(CALLBACK_FN fn)
{
  _deferred.push_back(fn);
}

void
Application::ExecuteDeferredCommands()
{
  // execute any registered command that could not been run during draw
  if (_deferred.size()) {
    for (size_t i = _deferred.size() - 1; i >= 0; --i)_deferred[i]();
    _deferred.clear();
  }
}


// browse for file
//----------------------------------------------------------------------------
std::string
Application::BrowseFile(int x, int y, const char* folder, const char* filters[], 
  const int numFilters, const char* name, bool readOrWrite)
{
  std::string result = 
    "/Users/malartrebenjamin/Documents/RnD/Jivaro/assets/Kitchen_set 3/Kitchen_set.usd";
  
  ModalFileBrowser::Mode mode = readOrWrite ? 
    ModalFileBrowser::Mode::SAVE : ModalFileBrowser::Mode::OPEN;

  const std::string label = readOrWrite ? "New" : "Open";

  ModalFileBrowser browser(x, y, label, mode);
  browser.Loop();
  if(browser.GetStatus() == BaseModal::Status::OK) {
    result = browser.GetResult();
  }
  browser.Term();  

  return result;
}

bool
Application::_IsAnyEngineDirty()
{
  for (auto& engine : _engines) {
    if (engine->IsDirty())return true;
  }
  return false;
}

void
Application::SetStage(pxr::UsdStageRefPtr& stage)
{
  _stageCache.Insert(stage);
  _stage = stage;
  _layer = stage->GetRootLayer();
}

static void _RecurseSplitRandomLayout(View* view, size_t depth, size_t maxDepth)
{
  std::cout << "recurse split..." << std::endl;
  if (depth > maxDepth)return;
  view->Split(RANDOM_0_1, rand() % 2);
  _RecurseSplitRandomLayout(view->GetLeft(), depth + 1, maxDepth);
  _RecurseSplitRandomLayout(view->GetRight(), depth + 1, maxDepth);
}

static void _BaseLayout(Window* window)
{
  window->SetGLContext();

  View* mainView = window->GetMainView();
  mainView->DeleteChildren();

  int width, height;
  glfwGetWindowSize(window->GetGlfwWindow(), &width, &height);

  window->SplitView(mainView, 0.5, true, View::LFIXED, 32);
  View* menuView = mainView->GetLeft();
  menuView->SetTabed(false);

  window->Resize(width, height);
  new MenuUI(menuView);
}

static void _RandomLayout(Window* window)
{
  window->SetGLContext();

  View* mainView = window->GetMainView();
  mainView->DeleteChildren();

  int width, height;
  glfwGetWindowSize(window->GetGlfwWindow(), &width, &height);

  window->SplitView(mainView, 0.5, true, View::LFIXED, 32);
  View* menuView = mainView->GetLeft();
  menuView->SetTabed(false);

  window->Resize(width, height);
  new MenuUI(menuView);

  View* mosaic = mainView->GetRight();
  _RecurseSplitRandomLayout(mosaic, 0, 3);
}

static void _StandardLayout(Window* window)
{
  window->SetGLContext();

  View* mainView = window->GetMainView();
  mainView->DeleteChildren();

  int width, height;
  glfwGetWindowSize(window->GetGlfwWindow(), &width, &height);
  window->SplitView(mainView, 0.5, true, View::LFIXED, 22);

  View* bottomView = mainView->GetRight();
  window->SplitView(bottomView, 0.9, true, false);

  View* timelineView = bottomView->GetRight();
  timelineView->SetTabed(false);

  View* centralView = bottomView->GetLeft();
  window->SplitView(centralView, 0.6, true);

  View* middleView = centralView->GetLeft();
  View* menuView = mainView->GetLeft();
  menuView->SetTabed(false);

  window->SplitView(middleView, 0.9, false);

  View* workingView = middleView->GetLeft();
  window->SplitView(workingView, 0.15, false);

  View* propertyView = middleView->GetRight();
  View* leftTopView = workingView->GetLeft();
  window->SplitView(leftTopView, 0.1, false, View::LFIXED, 32);

  View* toolView = leftTopView->GetLeft();
  toolView->SetTabed(false);
  View* explorerView = leftTopView->GetRight();
  View* viewportView = workingView->GetRight();
  View* graphView = centralView->GetRight();

  window->Resize(width, height);

  /*
  new GraphEditorUI(graphView);
  new ViewportUI(viewportView);
  new TimelineUI(timelineView);
  */
  new MenuUI(menuView);
  /*
  new ToolbarUI(toolView, true);
  new ExplorerUI(explorerView);
  */
}

static void _RawLayout(Window* window)
{
  window->SetGLContext();

  View* mainView = window->GetMainView();
  mainView->DeleteChildren();

  int width, height;
  glfwGetWindowSize(window->GetGlfwWindow(), &width, &height);

  window->SplitView(mainView, 0.5, true, View::LFIXED, 32);
  View* menuView = mainView->GetLeft();
  menuView->SetTabed(false);

  View* middleView = mainView->GetRight();
  window->SplitView(middleView, 0.5, true, View::RFIXED, 64);

  View* viewportView = middleView->GetLeft();
  View* timelineView = middleView->GetRight();

  window->Resize(width, height);

  new MenuUI(menuView);
  //new ViewportUI(viewportView);
  //new TimelineUI(timelineView);
}

void
Application::SetLayout(Window*  window, short layout)
{
  if (layout == 0) {
    _BaseLayout(window);
  } else if (layout == 1) {
    _RawLayout(window);
  }  else if(layout == 2) {
    _StandardLayout(window);
  }
  else {
    _RandomLayout(window);
  }
  window->ForceRedraw();
}

// init application
//----------------------------------------------------------------------------
void 
Application::Init()
{
 #ifdef _WIN32
  std::string filename =
    //"E:/Projects/RnD/USD_BUILD/assets/animX/test.usda";
    //"C:/Users/graph/Documents/bmal/src/USD_ASSETS/Kitchen_set/Kitchen_set.usd";
    //"E:/Projects/RnD/USD_BUILD/assets/Contour/JackTurbulized.usda";
    //"E:/Projects/RnD/USD/extras/usd/examples/usdGeomExamples/basisCurves.usda";
    //"E:/Projects/RnD/USD_BUILD/assets/maneki_anim.usd";
    "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usda";
    //"/Users/benmalartre/Documents/RnD/USD_BUILD/assets/UsdSkelExamples/HumanFemale/HumanFemal.usda";
    //"/Users/benmalartre/Documents/RnD/USD_BUILD/assets/Kitchen_set/Kitchen_set.usd";
    //"/Users/benmalartre/Documents/RnD/amnesie/usd/result.usda";
#else
  std::string filename = 
    "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usda";
#endif

  // build test scene
  //pxr::TestScene(filename);
  // create imaging gl engine
  //TfDebug::Enable(HD_MDI);
  //TfDebug::Enable(HD_ENGINE_PHASE_INFO);
  //TfDebug::Enable(GLF_DEBUG_CONTEXT_CAPS);
  //TfDebug::Enable(HDST_DUMP_SHADER_SOURCEFILE);
  //pxr::TfDebug::Enable(pxr::HD_DIRTY_LIST);
  //pxr::TfDebug::Enable(pxr::HD_COLLECTION_CHANGED);
  //pxr::TfDebug::Enable(pxr::LOFI_REGISTRY);

    // setup notifications
  pxr::TfNotice::Register(TfCreateWeakPtr(this), &Application::SelectionChangedCallback);
  pxr::TfNotice::Register(TfCreateWeakPtr(this), &Application::NewSceneCallback);
  pxr::TfNotice::Register(TfCreateWeakPtr(this), &Application::SceneChangedCallback);
  pxr::TfNotice::Register(TfCreateWeakPtr(this), &Application::AttributeChangedCallback);
  pxr::TfNotice::Register(TfCreateWeakPtr(this), &Application::UndoStackNoticeCallback);

  // create window
  _StandardLayout(_mainWindow);
  
  //_stage = TestAnimXFromFile(filename, editor);
  //pxr::UsdStageRefPtr stage = TestAnimX(editor);
  //_scene->GetRootStage()->GetRootLayer()->InsertSubLayerPath(stage->GetRootLayer()->GetIdentifier());

  /*
  // Create the layer to populate.
  std::string shotFilePath = "E:/Projects/RnD/USD_BUILD/assets/AnimX/test.usda";
  std::string animFilePath = "E:/Projects/RnD/USD_BUILD/assets/AnimX/anim.animx";
  //pxr::SdfLayerRefPtr baseLayer = pxr::SdfLayer::FindOrOpen(shotFilePath);
  
  // Create a UsdStage with that root layer.
  pxr::UsdStageRefPtr stage = pxr::UsdStage::Create(shotFilePath);
  stage->SetStartTimeCode(1);
  stage->SetEndTimeCode(100);
  
  pxr::UsdGeomCube cube =
    pxr::UsdGeomCube::Define(stage, pxr::SdfPath("/Cube"));
    

  stage->GetRootLayer()->Save();

  // we use Sdf, a lower level library, to obtain the 'anim' layer.
  pxr::SdfLayerRefPtr animLayer = pxr::SdfLayer::FindOrOpen(animFilePath);
  std::cout << "HAS LOCAL LAYER : " << stage->HasLocalLayer(animLayer) << std::endl;

  stage->SetEditTarget(animLayer);
  std::cout << "HAS LOCAL LAYER : " << stage->HasLocalLayer(animLayer) << std::endl;
  */
  
  /*
  // Create a mesh for the group.
        UsdGeomMesh mesh =
            UsdGeomMesh::Define(stage, SdfPath("/" + group.name));*/
  
  //_stage = pxr::UsdStage::CreateNew("test_stage");
  //_stage = pxr::UsdStage::Open(filename);

  //_stage = pxr::UsdStage::CreateNew("test.usda", pxr::TfNullPtr);
  //_stage = pxr::UsdStage::CreateInMemory();

  //_mesh = MakeColoredPolygonSoup(_scene->GetCurrentStage(), pxr::TfToken("/polygon_soup"));
  //Mesh* vdbMesh = MakeOpenVDBSphere(_stage, pxr::TfToken("/openvdb_sphere"));
/*
  for(size_t i=0; i< 12; ++i) {
    pxr::SdfPath path(pxr::TfToken("/cube_"+std::to_string(i)));
    pxr::UsdGeomCube cube = pxr::UsdGeomCube::Define(_stage, path);
    cube.AddTranslateOp().Set(pxr::GfVec3d(i * 3, 0, 0), pxr::UsdTimeCode::Default());
  }
*/
  //_stages.push_back(stage1);
  //TestStageUI(graph, _stages);

 
  _mainWindow->CollectLeaves();
 
  /*Window* childWindow = CreateChildWindow(200, 200, 400, 400, _mainWindow);
  AddWindow(childWindow);
  
  ViewportUI* viewport2 = new ViewportUI(childWindow->GetMainView());
  
  //DummyUI* dummy = new DummyUI(childWindow->GetMainView(), "Dummy");
  
  childWindow->CollectLeaves();*/

}

void 
Application::InitExec()
{
  Scene* scene = new Scene();
  scene->InitExec();
  for(auto& engine: _engines) {
    engine->InitExec(scene);
  }
  /*
  if (!_execInitialized) {
    _execStage = UsdStage::CreateInMemory("exec");
    _execScene = new Scene(_execStage);
    _solver = new PBDSolver();
   
    Time& time = GetApplication()->GetTime();
    _startFrame = time.GetStartTime();
    _lastFrame = time.GetActiveTime();
    
    _execStage->GetRootLayer()->TransferContent(_workStage->GetRootLayer());
    _execStage->SetDefaultPrim(_execStage->GetPrimAtPath(
      _workStage->GetDefaultPrim().GetPath()));

    pxr::UsdGeomXformCache xformCache(_startFrame);

    pxr::UsdPrimRange primRange = _execStage->TraverseAll();
    for (pxr::UsdPrim prim : primRange) {
      if (prim.IsA<pxr::UsdGeomMesh>()) {
        Mesh* mesh = _execScene->AddMesh(prim.GetPath());
        _solver->AddGeometry(mesh, 
          pxr::GfMatrix4f(xformCache.GetLocalToWorldTransform(prim)));
        
        Voxels* voxels = _execScene->AddVoxels(prim.GetPath().AppendElementString("Voxels"), mesh, 0.2f);
        _solver->AddGeometry(voxels,
          pxr::GfMatrix4f(xformCache.GetLocalToWorldTransform(prim)));

      }
    }

    
    std::vector<Geometry*> colliders;
    for (auto& mesh : _execScene->GetMeshes()) {
      colliders.push_back(&mesh.second);
    }
    for(auto& collider: colliders)
      _solver->AddCollider(collider);
    

    PBDParticle* system = _solver->GetSystem();
    _CreateSystemPoints(_execStage, system->Get(), 0.05);

    
    _execInitialized = true;
  }
  */
}

void 
Application::UpdateExec(double time)
{
  Scene* scene = _engines[0]->GetDelegate()->GetScene();
  scene->UpdateExec(time);
  for(auto& engine: _engines) {
    engine->UpdateExec(time);
  }
  /*
  for (auto& meshMapIt : _execScene->GetMeshes()) {
    pxr::SdfPath path = meshMapIt.first;
    Mesh* mesh = &meshMapIt.second;
    pxr::VtArray<pxr::GfVec3f> positions;
    pxr::UsdGeomMesh input(_workStage->GetPrimAtPath(path));

    double t = pxr::GfSin(GetApplication()->GetTime().GetActiveTime() * 100);

    input.GetPointsAttr().Get(&positions, time);
    for (auto& position : positions) {
      position += pxr::GfVec3f(
        pxr::GfSin(position[0] + t),
        pxr::GfCos(position[0] + t) * 5.0,
        RANDOM_0_1 * 0.05
      );
    }
    mesh->Update(positions);
  }
  
  if (time <= _startFrame) {
    _solver->Reset();
  }
  if(time > _lastFrame) {
    _solver->Step();
    _execScene->Update(time);
  }
  _lastFrame = (float)time;
  */
}

void 
Application::TerminateExec()
{
  Scene* scene = _engines[0]->GetDelegate()->GetScene();
  for (auto& engine : _engines) {
    engine->TerminateExec();
  }
  delete scene;
  //delete _solver;
}


void 
Application::Term()
{

}

bool 
Application::Update()
{
  ExecuteDeferredCommands();

  /*
  if (_needCaptureFramebuffers) {
    _mainWindow->CaptureFramebuffer();
    for (auto& childWindow : _childWindows)childWindow->CaptureFramebuffer();
    _needCaptureFramebuffers = false;
  }
  */
  
  static double lastTime = 0.f;
  static double refreshRate = 1.f / 60.f;
  double currentTime = glfwGetTime();
  if (currentTime - lastTime > refreshRate) {
    lastTime = currentTime;
    if (_execute && (_time.IsPlaying() || _IsAnyEngineDirty())) {
      UpdateExec(_time.GetActiveTime());
    }
  } else {
    if(!_time.IsPlaying())
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  glfwPollEvents();

  _time.ComputeFramerate(glfwGetTime());
  if (_time.IsPlaying()) {
    if (_time.PlayBack()) {
      GetActiveEngine()->SetDirty(true);
    }
  }
  
  // draw popup
  if (_popup) {
    Window* window = _popup->GetView()->GetWindow();
    window->DrawPopup(_popup);
    if (_popup->IsDone() || _popup->IsCancel()) {
      _popup->Terminate();
      delete _popup;
      _popup = nullptr;
    }
  } else {
    if (!_mainWindow->Update()) return false;
    for (auto& childWindow : _childWindows)childWindow->Update();
  }
  
    
  return true;
}

void
Application::AddWindow(Window* window)
{
 _childWindows.push_back(window);
  window->Init();
  window->SetGLContext();
}

void 
Application::RemoveWindow(Window* window)
{
  std::vector<Window*>::iterator it = _childWindows.begin();
  for (; it < _childWindows.end(); ++it) {
    if(*it == window) {
      _childWindows.erase(it);
    }
  }
}

void 
Application::AddEngine(Engine* engine)
{
  _engines.push_back(engine);
}

void 
Application::RemoveEngine(Engine* engine)
{
  for (size_t i = 0; i < _engines.size(); ++i) {
    if (engine == _engines[i]) {
      _engines.erase(_engines.begin() + i);
      break;
    }
  }
}

void Application::DirtyAllEngines()
{
  for (auto& engine : _engines) {
    engine->SetDirty(true);
  }
}

Engine* Application::GetActiveEngine()
{
  if (_viewport)return _viewport->GetEngine();
}

void 
Application::SetActiveViewport(ViewportUI* viewport) 
{
  if (_viewport) {
    _viewport->GetView()->ClearFlag(View::TIMEVARYING);
  }
  _viewport = viewport;
  _viewport->GetView()->SetFlag(View::TIMEVARYING);
}

void 
Application::SetActiveTool(short t)
{
  _mainWindow->GetTool()->SetActiveTool(t);
  for (auto& window : _childWindows) {
    window->GetTool()->SetActiveTool(t);
  }
}

void 
Application::SelectionChangedCallback(const SelectionChangedNotice& n)
{
  for (auto& engine : _engines) {
    if (!_selection.IsEmpty() && _selection.IsObject()) {
      engine->SetSelected(_selection.GetSelectedPrims());
    } else {
      engine->ClearSelected();
    }
  }
  _mainWindow->GetTool()->ResetSelection();
  _mainWindow->ForceRedraw();
  for (auto& window : _childWindows) {
    window->GetTool()->ResetSelection();
    window->ForceRedraw();
  }
  DirtyAllEngines();
}

void 
Application::NewSceneCallback(const NewSceneNotice& n)
{
  _selection.Clear();
  _manager.Clear();
  DirtyAllEngines();
}

void 
Application::SceneChangedCallback(const SceneChangedNotice& n)
{
  _mainWindow->GetTool()->ResetSelection();
  _mainWindow->ForceRedraw();
  for (auto& window : _childWindows) {
    window->GetTool()->ResetSelection();
    window->ForceRedraw();
  }
  
  DirtyAllEngines();
}

void
Application::AttributeChangedCallback(const AttributeChangedNotice& n)
{
  _mainWindow->ForceRedraw();
  _mainWindow->GetTool()->ResetSelection();
  for (auto& window : _childWindows) {
    window->ForceRedraw();
    window->GetTool()->ResetSelection();
  }
  DirtyAllEngines();
}

void
Application::UndoStackNoticeCallback(const UndoStackNotice& n)
{
  ADD_COMMAND(UsdGenericCommand);
}


void 
Application::AddCommand(std::shared_ptr<Command> command)
{
  _manager.AddCommand(command);
  _manager.ExecuteCommands();
  GetMainWindow()->ForceRedraw();
}

void 
Application::Undo()
{
  _manager.Undo();
}

void 
Application::Redo()
{
  _manager.Redo();
}

void 
Application::Delete()
{
  Selection* selection = GetSelection();
  const pxr::SdfPathVector& paths = selection->GetSelectedPrims();
  selection->Clear();
  ADD_COMMAND(DeletePrimCommand, GetWorkStage(), paths);
}

void
Application::Duplicate()
{
  Selection* selection = GetSelection();
  if (!selection->IsEmpty()) {
    const Selection::Item& item = selection->GetItem(0);
    ADD_COMMAND(DuplicatePrimCommand, GetWorkStage(), item.path);
  }
}

void 
Application::OpenScene(const std::string& filename)
{
  ADD_COMMAND(OpenSceneCommand, filename);
}

void
Application::NewScene(const std::string& filename)
{
  ADD_COMMAND(NewSceneCommand, filename);
}

void Application::SaveScene()
{
  GetWorkStage()->GetRootLayer()->Save(true);
}

void Application::SaveSceneAs(const std::string& filename)
{
  GetWorkStage()->GetRootLayer()->Save(true);
}

// execution
void 
Application::ToggleExec() 
{
  _execute = 1 - _execute; 
  if (_execute)InitExec();
  else TerminateExec();
  DirtyAllEngines();
};

void 
Application::SetExec(bool state) 
{ 
  _execute = state; 
};

bool 
Application::GetExec() 
{ 
  return _execute; 
};

// get stage for display
pxr::UsdStageRefPtr
Application::GetDisplayStage()
{
  return _stage;
}

// get stage for work
pxr::UsdStageRefPtr
Application::GetWorkStage()
{
  return _stage;
}

// get current layer
pxr::SdfLayerRefPtr
Application::GetCurrentLayer()
{
  return _layer;
}

// selection
void 
Application::SetSelection(const pxr::SdfPathVector& selection)
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, selection, SelectCommand::SET);
}

void
Application::ToggleSelection(const pxr::SdfPathVector& selection)
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, selection, SelectCommand::TOGGLE);
}

void 
Application::AddToSelection(const pxr::SdfPathVector& paths)
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, paths, SelectCommand::ADD);
}

void 
Application::RemoveFromSelection(const pxr::SdfPathVector& paths)
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, paths, SelectCommand::REMOVE);
}

void 
Application::ClearSelection()
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, {}, SelectCommand::SET);
}

pxr::GfBBox3d
Application::GetStageBoundingBox()
{
  pxr::GfBBox3d bbox;
  pxr::TfTokenVector purposes = { pxr::UsdGeomTokens->default_ };
  pxr::UsdGeomBBoxCache bboxCache(
    pxr::UsdTimeCode(_time.GetActiveTime()), purposes, false, false);
  return bboxCache.ComputeWorldBound(_stage->GetPseudoRoot());
}

pxr::GfBBox3d 
Application::GetSelectionBoundingBox()
{
  pxr::GfBBox3d bbox;
  static pxr::TfTokenVector purposes = {
    pxr::UsdGeomTokens->default_,
    pxr::UsdGeomTokens->proxy,
    pxr::UsdGeomTokens->guide,
    pxr::UsdGeomTokens->render
  };
  pxr::UsdGeomBBoxCache bboxCache(
    pxr::UsdTimeCode(_time.GetActiveTime()), purposes, false, false);
  for (size_t n = 0; n < _selection.GetNumSelectedItems(); ++n) {
    const Selection::Item& item = _selection[n];
    if (item.type == Selection::Type::PRIM) {
      pxr::UsdPrim prim = _stage->GetPrimAtPath(item.path);
      
      if (prim.IsActive()) {
        const pxr::GfBBox3d primBBox = bboxCache.ComputeWorldBound(prim);
        bbox = bbox.Combine(bbox, pxr::GfBBox3d(primBBox.ComputeAlignedRange()));
      }
        
    }
    else if (item.type == Selection::Type::VERTEX) {

    }
    else if (item.type == Selection::Type::EDGE) {

    }
    else if (item.type == Selection::Type::FACE) {

    }
  }

  /*
  pxr::UsdPrim& prim = _stage->GetPrimAtPath(pxr::SdfPath("/Cube"));
  if (!prim.IsValid()) {
    prim = pxr::UsdGeomCube::Define(_stage, pxr::SdfPath("/Cube")).GetPrim();
    pxr::UsdGeomCube cube(prim);
    cube.CreateSizeAttr().Set(pxr::VtValue(1.0));
   
    pxr::VtArray<pxr::TfToken> xformOpOrderTokens =
    { pxr::TfToken("xformOp:scale"), pxr::TfToken("xformOp:translate")};
    cube.CreateXformOpOrderAttr().Set(pxr::VtValue(xformOpOrderTokens));
   
  }
  pxr::UsdGeomCube cube(prim);

  bool resetXformStack = false;
  bool foundScaleOp = false;
  bool foundTranslateOp = false;
  std::vector<pxr::UsdGeomXformOp> xformOps = cube.GetOrderedXformOps(&resetXformStack);
  for (auto& xformOp : xformOps) {
 
    if (xformOp.GetName() == pxr::TfToken("xformOp:scale")) {
      pxr::GfRange3d bboxRange = bbox.GetRange();
      xformOp.Set(pxr::VtValue(pxr::GfVec3f(bboxRange.GetMax() - bboxRange.GetMin())));
      foundScaleOp = true;
    } else if(xformOp.GetName() == pxr::TfToken("xformOp:translate")) {
      pxr::GfVec3d center = bbox.ComputeCentroid();
      xformOp.Set(pxr::VtValue(center));
      foundTranslateOp = true;
    }
  }
  if (!foundScaleOp) {
    pxr::UsdGeomXformOp scaleOp = cube.AddScaleOp();
    pxr::GfRange3d bboxRange = bbox.GetRange();
    scaleOp.Set(pxr::VtValue(pxr::GfVec3f(bboxRange.GetMax() - bboxRange.GetMin())));
  }
  if (!foundTranslateOp) {
    pxr::UsdGeomXformOp translateOp = cube.AddTranslateOp();
    pxr::GfVec3d center = bbox.ComputeCentroid();
    translateOp.Set(pxr::VtValue(center));
  }
  */
  return bbox;
}

JVR_NAMESPACE_CLOSE_SCOPE
