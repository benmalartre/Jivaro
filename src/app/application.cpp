#include <iostream>
#include <string>

#include "../utils/files.h"
#include "../utils/timer.h"
#include "../utils/prefs.h"
#include "../ui/popup.h"
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
#include "../tests/pbd.h"
#include "../tests/hair.h"
#include "../tests/particles.h"
#include "../tests/raycast.h"

JVR_NAMESPACE_OPEN_SCOPE

const char* Application::name = "Jivaro";

// singleton
//----------------------------------------------------------------------------
Application* Application::_singleton=nullptr;

Application* Application::Get() { 
  if(_singleton==nullptr){
        _singleton = new Application();
    }
    return _singleton; 
};

// constructor
//----------------------------------------------------------------------------
Application::Application()
  : _mainWindow(nullptr)
  , _activeWindow(nullptr)
  , _popup(nullptr)
  , _execute(false)
  , _activeEngine(nullptr)
  , _exec(nullptr)
{  
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
Application::CreateFullScreenWindow(const std::string& name)
{
  return Window::CreateFullScreenWindow(name);
}

// create child window
//----------------------------------------------------------------------------
Window*
Application::CreateChildWindow(const std::string& name, const pxr::GfVec4i& dimension, Window* parent)
{
  return
    Window::CreateChildWindow(name, dimension, parent);
}

// create standard window
//----------------------------------------------------------------------------
Window*
Application::CreateStandardWindow(const std::string& name, const pxr::GfVec4i& dimension)
{
  return Window::CreateStandardWindow(name, dimension);
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
    for(int i = _deferred.size() - 1; i >= 0; --i) _deferred[i]();
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
  if(browser.GetStatus() == ModalBase::Status::OK) {
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

// init application
//----------------------------------------------------------------------------
void 
Application::Init(unsigned width, unsigned height, bool fullscreen)
{
  if(fullscreen) {
    _mainWindow = CreateFullScreenWindow(name);
  } else {
    _mainWindow = CreateStandardWindow(name, pxr::GfVec4i(0,0,width, height));
  }
  
  _activeWindow = _mainWindow;
  _time.Init(1, 101, 24);
  
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
  _mainWindow->SetDesiredLayout(WINDOW_LAYOUT_STANDARD);
  
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

 
  //_mainWindow->CollectLeaves();
 
  /*Window* childWindow = CreateChildWindow(200, 200, 400, 400, _mainWindow);
  AddWindow(childWindow);
  
  ViewportUI* viewport2 = new ViewportUI(childWindow->GetMainView());
  
  //DummyUI* dummy = new DummyUI(childWindow->GetMainView(), "Dummy");
  
  childWindow->CollectLeaves();*/

}

void 
Application::InitExec(pxr::UsdStageRefPtr& stage)
{
  _exec = new TestRaycast(new Scene());
  //_exec = new TestParticles(new Scene());
  //_exec = new TestPBD(new Scene());
  //_exec = new TestHair(new Scene());
  _exec->InitExec(stage);

  for(auto& engine: _engines) {
    engine->InitExec(_exec->GetScene());
  }
  
}

void
Application::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _exec->UpdateExec(stage, time);

  for (auto& engine : _engines) {
    engine->UpdateExec(time);
  }
  
}

void
Application::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  for (auto& engine : _engines) {
    engine->TerminateExec();
  }
  Scene* scene = _exec->GetScene();
  delete scene;
  delete _exec;
  _exec = nullptr;  
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
  float currentTime(GetTime().GetActiveTime());

  if (currentTime != lastTime) {
    lastTime = currentTime;
    if (_execute) {
      UpdateExec(_stage, currentTime);
    }
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
  _activeEngine = engine;
}

void 
Application::RemoveEngine(Engine* engine)
{
  if (engine == _activeEngine)  _activeEngine = NULL;
  for (size_t i = 0; i < _engines.size(); ++i) {
    if (engine == _engines[i]) {
      _engines.erase(_engines.begin() + i);
      break;
    }
  }
}

void 
Application::DirtyAllEngines()
{
  for (auto& engine : _engines) {
    engine->SetDirty(true);
  }
}

Engine* Application::GetActiveEngine()
{
  return _activeEngine;
}

void 
Application::SetActiveEngine(Engine* engine) 
{
  _activeEngine = engine;
}

void 
Application::SetActiveTool(size_t t)
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
      engine->SetSelected(_selection.GetSelectedPaths());
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
  if (_execute && _exec) {
    UpdateExec(_stage, _time.GetActiveTime());
  }
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
  const pxr::SdfPathVector& paths = selection->GetSelectedPaths();
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
  if (_execute)InitExec(_stage);
  else TerminateExec(_stage);
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
