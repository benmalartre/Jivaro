#include <iostream>
#include <string>

#include "../utils/files.h"
#include "../utils/timer.h"
#include "../utils/prefs.h"
#include "../ui/popup.h"
#include "../geometry/scene.h"
#include "../app/application.h"
#include "../app/commands.h"
#include "../app/modal.h"
#include "../app/notice.h"
#include "../app/handle.h"
#include "../app/engine.h"
#include "../app/selection.h"
#include "../app/window.h"
#include "../app/view.h"
#include "../app/camera.h"
#include "../app/tools.h"

#include "../tests/grid.h"
#include "../tests/raycast.h"
#include "../tests/particles.h"
#include "../tests/pbd.h"
#include "../tests/hair.h"
#include "../tests/bvh.h"
#include "../tests/points.h"
#include "../tests/instancer.h"
#include "../tests/velocity.h"
#include "../tests/pendulum.h"
#include "../tests/geodesic.h"
#include "../tests/push.h"


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
Application::CreateChildWindow(const std::string& name, const GfVec4i& dimension, Window* parent)
{
  return
    Window::CreateChildWindow(name, dimension, parent);
}

// create standard window
//----------------------------------------------------------------------------
Window*
Application::CreateStandardWindow(const std::string& name, const GfVec4i& dimension)
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
Application::SetStage(UsdStageRefPtr& stage)
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
    _mainWindow = CreateStandardWindow(name, GfVec4i(0,0,width, height));
  }
  
  _activeWindow = _mainWindow;
  Time::Get()->Init(1, 101, 24);
  
  //TfDebug::Enable(HD_MDI);
  //TfDebug::Enable(HD_ENGINE_PHASE_INFO);
  //TfDebug::Enable(GLF_DEBUG_CONTEXT_CAPS);
  //TfDebug::Enable(HDST_DUMP_SHADER_SOURCEFILE);
  //TfDebug::Enable(HD_DIRTY_LIST);
  //TfDebug::Enable(HD_COLLECTION_CHANGED);
  //TfDebug::Enable(LOFI_REGISTRY);

    // setup notifications
  TfNotice::Register(TfCreateWeakPtr(this), &Application::SelectionChangedCallback);
  TfNotice::Register(TfCreateWeakPtr(this), &Application::NewSceneCallback);
  TfNotice::Register(TfCreateWeakPtr(this), &Application::SceneChangedCallback);
  TfNotice::Register(TfCreateWeakPtr(this), &Application::AttributeChangedCallback);
  TfNotice::Register(TfCreateWeakPtr(this), &Application::TimeChangedCallback);
  TfNotice::Register(TfCreateWeakPtr(this), &Application::UndoStackNoticeCallback);

  // create window
  _mainWindow->SetDesiredLayout(WINDOW_LAYOUT_STANDARD);
  
  //_stage = TestAnimXFromFile(filename, editor);
  //UsdStageRefPtr stage = TestAnimX(editor);
  //_scene->GetRootStage()->GetRootLayer()->InsertSubLayerPath(stage->GetRootLayer()->GetIdentifier());

  /*
  // Create the layer to populate.
  std::string shotFilePath = "E:/Projects/RnD/USD_BUILD/assets/AnimX/test.usda";
  std::string animFilePath = "E:/Projects/RnD/USD_BUILD/assets/AnimX/anim.animx";
  //SdfLayerRefPtr baseLayer = SdfLayer::FindOrOpen(shotFilePath);
  
  // Create a UsdStage with that root layer.
  UsdStageRefPtr stage = UsdStage::Create(shotFilePath);
  stage->SetStartTimeCode(1);
  stage->SetEndTimeCode(100);
  
  UsdGeomCube cube =
    UsdGeomCube::Define(stage, SdfPath("/Cube"));
    

  stage->GetRootLayer()->Save();

  // we use Sdf, a lower level library, to obtain the 'anim' layer.
  SdfLayerRefPtr animLayer = SdfLayer::FindOrOpen(animFilePath);
  std::cout << "HAS LOCAL LAYER : " << stage->HasLocalLayer(animLayer) << std::endl;

  stage->SetEditTarget(animLayer);
  std::cout << "HAS LOCAL LAYER : " << stage->HasLocalLayer(animLayer) << std::endl;
  */
  
  /*
  // Create a mesh for the group.
        UsdGeomMesh mesh =
            UsdGeomMesh::Define(stage, SdfPath("/" + group.name));*/
  
  //_stage = UsdStage::CreateNew("test_stage");
  //_stage = UsdStage::Open(filename);

  //_stage = UsdStage::CreateNew("test.usda", TfNullPtr);
  //_stage = UsdStage::CreateInMemory();

  //_mesh = MakeColoredPolygonSoup(_scene->GetCurrentStage(), TfToken("/polygon_soup"));
  //Mesh* vdbMesh = MakeOpenVDBSphere(_stage, TfToken("/openvdb_sphere"));
/*
  for(size_t i=0; i< 12; ++i) {
    SdfPath path(TfToken("/cube_"+std::to_string(i)));
    UsdGeomCube cube = UsdGeomCube::Define(_stage, path);
    cube.AddTranslateOp().Set(GfVec3d(i * 3, 0, 0), UsdTimeCode::Default());
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
Application::InitExec(UsdStageRefPtr& stage)
{
  //_exec = new TestPendulum();
  //_exec = new TestVelocity();
  //_exec = new TestPoints();
  //_exec = new TestGrid();
  //_exec = new TestParticles();
  //_exec = new TestInstancer();
  //_exec = new TestRaycast();
  _exec = new TestPBD();
  //_exec = new TestPush();
  //_exec = new TestHair();
  //_exec = new TestGeodesic();
  //_exec = new TestBVH();
  _exec->InitExec(stage);

  for(auto& engine: _engines) {
    engine->InitExec(_exec->GetScene());
  }
  
}

void
Application::UpdateExec(UsdStageRefPtr& stage, float time)
{
  _exec->UpdateExec(stage, time);

  for (auto& engine : _engines) {
    engine->UpdateExec(time);
  }
  
}

void
Application::TerminateExec(UsdStageRefPtr& stage)
{
  for (auto& engine : _engines) {
    engine->TerminateExec();
  }
  _exec->TerminateExec(stage);
  delete _exec;
  _exec = NULL;  
  _execute = false;
  NewSceneNotice().Send();
}

void 
Application::SendExecViewEvent(const ViewEventData *data)
{
  _exec->ViewEvent(data);
}



void
Application::Term()
{
  std::cout << "Jivaro Application Terminate!!" << std::endl;
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

  glfwPollEvents();
  //Time::Get()->ComputeFramerate();

  static double refreshRate = 1.f / 60.f;
  static int playback;
  Time* time = Time::Get();
  float currentTime(time->GetActiveTime());
  
  
  // execution if needed
  if (time->IsPlaying()) {
    playback = time->Playback();
    if (playback != Time::PLAYBACK_WAITING) {
      if(_execute) UpdateExec(_stage, currentTime);
      GetActiveEngine()->SetDirty(true);
      _lastTime = currentTime;
    }
  } else {
    if (currentTime != _lastTime || _mainWindow->GetTool()->IsInteracting()) {
      _lastTime = currentTime;
      if (_execute) UpdateExec(_stage, currentTime);
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

  // playback if needed
  if(time->IsPlaying() && playback != Time::PLAYBACK_WAITING) {
    switch(playback) {
      case Time::PLAYBACK_NEXT:
        time->NextFrame(); break;
      case Time::PLAYBACK_PREVIOUS:
        time->PreviousFrame(); break;
      case Time::PLAYBACK_FIRST:
        time->FirstFrame(); break;
      case Time::PLAYBACK_LAST:
        time->LastFrame(); break;
      case Time::PLAYBACK_STOP:
        time->StopPlayback(); break;
    }
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
  Tool* tool = _mainWindow->GetTool();
  size_t lastActiveTool = tool->GetActiveTool();
  if(t != lastActiveTool) {
    tool->SetActiveTool(t);
    for (auto& window : _childWindows) {
      window->GetTool()->SetActiveTool(t);
    }
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
  if(_mainWindow->GetTool()->IsActive())
    _mainWindow->GetTool()->ResetSelection();
  _mainWindow->ForceRedraw();
  for (auto& window : _childWindows) {
    if(window->GetTool()->IsActive())
      window->GetTool()->ResetSelection();
    window->ForceRedraw();
  }
  DirtyAllEngines();
}

void 
Application::NewSceneCallback(const NewSceneNotice& n)
{
  if(_exec) TerminateExec(_stage);

  _selection.Clear();
  _manager.Clear();
  DirtyAllEngines();
}

void 
Application::SceneChangedCallback(const SceneChangedNotice& n)
{
  if(_exec) TerminateExec(_stage);

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
  if (_exec && _execute) {
    UpdateExec(_stage, Time::Get()->GetActiveTime());
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
Application::TimeChangedCallback(const TimeChangedNotice& n)
{
  if (_exec && _execute) {
    UpdateExec(_stage, Time::Get()->GetActiveTime());
    
  }
  _mainWindow->ForceRedraw();
  for (auto& window : _childWindows) {
    window->ForceRedraw();
  }
  DirtyAllEngines();
  _lastTime = Time::Get()->GetActiveTime();
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
  const SdfPathVector& paths = selection->GetSelectedPaths();
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
UsdStageRefPtr
Application::GetDisplayStage()
{
  return _stage;
}

// get stage for work
UsdStageRefPtr
Application::GetWorkStage()
{
  return _stage;
}

// get current layer
SdfLayerRefPtr
Application::GetCurrentLayer()
{
  return _layer;
}

// selection
void 
Application::SetSelection(const SdfPathVector& selection)
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, selection, SelectCommand::SET);
}

void
Application::ToggleSelection(const SdfPathVector& selection)
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, selection, SelectCommand::TOGGLE);
}

void 
Application::AddToSelection(const SdfPathVector& paths)
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, paths, SelectCommand::ADD);
}

void 
Application::RemoveFromSelection(const SdfPathVector& paths)
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, paths, SelectCommand::REMOVE);
}

void 
Application::ClearSelection()
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, {}, SelectCommand::SET);
}

GfBBox3d
Application::GetStageBoundingBox()
{
  GfBBox3d bbox;
  TfTokenVector purposes = { UsdGeomTokens->default_ };
  UsdGeomBBoxCache bboxCache(
    UsdTimeCode(Time::Get()->GetActiveTime()), purposes, false, false);
  return bboxCache.ComputeWorldBound(_stage->GetPseudoRoot());
}

GfBBox3d 
Application::GetSelectionBoundingBox()
{
  GfBBox3d bbox;
  static TfTokenVector purposes = {
    UsdGeomTokens->default_,
    UsdGeomTokens->proxy,
    UsdGeomTokens->guide,
    UsdGeomTokens->render
  };
  UsdGeomBBoxCache bboxCache(
    UsdTimeCode(Time::Get()->GetActiveTime()), purposes, false, false);
  for (size_t n = 0; n < _selection.GetNumSelectedItems(); ++n) {
    const Selection::Item& item = _selection[n];
    if (item.type == Selection::Type::PRIM) {
      UsdPrim prim = _stage->GetPrimAtPath(item.path);
      
      if (prim.IsActive()) {
        const GfBBox3d primBBox = bboxCache.ComputeWorldBound(prim);
        bbox = bbox.Combine(bbox, GfBBox3d(primBBox.ComputeAlignedRange()));
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
  UsdPrim& prim = _stage->GetPrimAtPath(SdfPath("/Cube"));
  if (!prim.IsValid()) {
    prim = UsdGeomCube::Define(_stage, SdfPath("/Cube")).GetPrim();
    UsdGeomCube cube(prim);
    cube.CreateSizeAttr().Set(VtValue(1.0));
   
    VtArray<TfToken> xformOpOrderTokens =
    { TfToken("xformOp:scale"), TfToken("xformOp:translate")};
    cube.CreateXformOpOrderAttr().Set(VtValue(xformOpOrderTokens));
   
  }
  UsdGeomCube cube(prim);

  bool resetXformStack = false;
  bool foundScaleOp = false;
  bool foundTranslateOp = false;
  std::vector<UsdGeomXformOp> xformOps = cube.GetOrderedXformOps(&resetXformStack);
  for (auto& xformOp : xformOps) {
 
    if (xformOp.GetName() == TfToken("xformOp:scale")) {
      GfRange3d bboxRange = bbox.GetRange();
      xformOp.Set(VtValue(GfVec3f(bboxRange.GetMax() - bboxRange.GetMin())));
      foundScaleOp = true;
    } else if(xformOp.GetName() == TfToken("xformOp:translate")) {
      GfVec3d center = bbox.ComputeCentroid();
      xformOp.Set(VtValue(center));
      foundTranslateOp = true;
    }
  }
  if (!foundScaleOp) {
    UsdGeomXformOp scaleOp = cube.AddScaleOp();
    GfRange3d bboxRange = bbox.GetRange();
    scaleOp.Set(VtValue(GfVec3f(bboxRange.GetMax() - bboxRange.GetMin())));
  }
  if (!foundTranslateOp) {
    UsdGeomXformOp translateOp = cube.AddTranslateOp();
    GfVec3d center = bbox.ComputeCentroid();
    translateOp.Set(VtValue(center));
  }
  */
  return bbox;
}

JVR_NAMESPACE_CLOSE_SCOPE
