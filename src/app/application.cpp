#include <iostream>
#include <string>

#include "../utils/files.h"
#include "../utils/timer.h"
#include "../utils/prefs.h"
#include "../ui/popup.h"
#include "../command/manager.h"
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
bool Application::PlaybackAllViewports = false;

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
  SetAllWindowsDirty();
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

  // setup notifications
  TfNotice::Register(TfCreateWeakPtr(this), &Application::SelectionChangedCallback);
  TfNotice::Register(TfCreateWeakPtr(this), &Application::NewSceneCallback);
  TfNotice::Register(TfCreateWeakPtr(this), &Application::SceneChangedCallback);
  TfNotice::Register(TfCreateWeakPtr(this), &Application::AttributeChangedCallback);
  TfNotice::Register(TfCreateWeakPtr(this), &Application::TimeChangedCallback);
  TfNotice::Register(TfCreateWeakPtr(this), &Application::UndoStackNoticeCallback);

  
  //TfDebug::Enable(HD_MDI);
  //TfDebug::Enable(HD_ENGINE_PHASE_INFO);
  //TfDebug::Enable(GLF_DEBUG_CONTEXT_CAPS);
  //TfDebug::Enable(HDST_DUMP_SHADER_SOURCEFILE);
  //TfDebug::Enable(HD_DIRTY_LIST);
  //TfDebug::Enable(HD_COLLECTION_CHANGED);
  //TfDebug::Enable(LOFI_REGISTRY);

  // create window
  _mainWindow->SetDesiredLayout(WINDOW_LAYOUT_STANDARD);

  _model = new Model();


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
 Time* time = Time::Get();
 
  static bool vSync = true;
  if(vSync && time->IsPlaying())
    glfwPollEvents();
  else
    glfwWaitEvents();

  float currentTime(time->GetActiveTime());
  int playback = time->Playback();
  if( playback == Time::PLAYBACK_WAITING) return true;
  
  // update model
  if (!_popup && (playback > Time::PLAYBACK_IDLE || Application::Get()->IsToolInteracting())) {
    if(_model->GetExec() ) 
      _model->UpdateExec(currentTime);
    SetAllWindowsDirty();
  }

  _model->Update(currentTime);
    //
  //}
  
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
  

  CommandManager::Get()->ExecuteCommands();
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
Application::SetWindowDirty(Window* window)
{
  window->DirtyAllViews();
}

void 
Application::SetAllWindowsDirty()
{
  _mainWindow->DirtyAllViews();
  for(auto& childWindow: _childWindows)
    childWindow->DirtyAllViews();
}

// playback viewport
bool 
Application::IsPlaybackViewport(ViewportUI* viewport) 
{ 
  return viewport == _viewport || PlaybackAllViewports; 
};

void 
Application::SetPlaybackViewport(ViewportUI* viewport){
  _viewport = viewport;
};

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

bool
Application::IsToolInteracting()
{
  Tool* tool = _mainWindow->GetTool();
  return tool->IsInteracting();
}

void 
Application::Undo()
{
  CommandManager::Get()->Undo();
  _model->Update(Time::Get()->GetActiveTime());
}

void 
Application::Redo()
{
  CommandManager::Get()->Redo();
  _model->Update(Time::Get()->GetActiveTime());
}

void 
Application::Delete()
{
  Selection* selection = _model->GetSelection();
  const SdfPathVector& paths = selection->GetSelectedPaths();
  selection->Clear();
  ADD_COMMAND(DeletePrimCommand, _model->GetStage(), paths);
}

void
Application::Duplicate()
{
  Selection* selection = _model->GetSelection();
  if (!selection->IsEmpty()) {
    const Selection::Item& item = selection->GetItem(0);
    ADD_COMMAND(DuplicatePrimCommand, _model->GetStage(), item.path);
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
  _model->GetStage()->GetRootLayer()->Save(true);
}

void Application::SaveSceneAs(const std::string& filename)
{
  _model->GetStage()->GetRootLayer()->Save(true);
}

// ---------------------------------------------------------------------------------------------
// Notices Callbacks
//----------------------------------------------------------------------------------------------
void 
Application::SelectionChangedCallback(const SelectionChangedNotice& n)
{  
  if(_mainWindow->GetTool()->IsActive())
    _mainWindow->GetTool()->ResetSelection();

  for (auto& window : _childWindows) {
    if(window->GetTool()->IsActive())
      window->GetTool()->ResetSelection();
  }
  SetAllWindowsDirty();
}

void 
Application::NewSceneCallback(const NewSceneNotice& n)
{
  if(_model->GetExec()) _model->TerminateExec();

  _model->ClearSelection();
  SetAllWindowsDirty();
}

void 
Application::SceneChangedCallback(const SceneChangedNotice& n)
{
  Application::Get()->
  _mainWindow->GetTool()->ResetSelection();
  for (auto& window : _childWindows) {
    window->GetTool()->ResetSelection();
  }
  
  SetAllWindowsDirty();
}

void
Application::AttributeChangedCallback(const AttributeChangedNotice& n)
{
  if (_model->GetExec()) 
    _model->UpdateExec(Time::Get()->GetActiveTime());
  
  _mainWindow->GetTool()->ResetSelection();
  for (auto& window : _childWindows) {
    window->GetTool()->ResetSelection();
  }

  SetAllWindowsDirty();

}

void
Application::TimeChangedCallback(const TimeChangedNotice& n)
{
  if (_model->GetExec()) 
    _model->UpdateExec(Time::Get()->GetActiveTime());

  SetAllWindowsDirty();
}

void
Application::UndoStackNoticeCallback(const UndoStackNotice& n)
{
  ADD_COMMAND(UsdGenericCommand);
}



JVR_NAMESPACE_CLOSE_SCOPE
