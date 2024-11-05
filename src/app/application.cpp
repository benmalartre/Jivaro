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
#include "../app/layout.h"

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
{  

};

// destructor
//----------------------------------------------------------------------------
Application::~Application()
{

};

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
  Window* parent = WindowRegistry::GetActiveWindow();
  ModalFileBrowser browser(parent, x, y, label, mode);
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
  Window* window;
  if(fullscreen) {
    window = WindowRegistry::CreateFullScreenWindow(name);
  } else {
    window = WindowRegistry::CreateStandardWindow(name, GfVec4i(0,0,width, height));
  }

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

  window->SetDesiredLayout(WINDOW_LAYOUT_STANDARD);

  _model = new Model();


}




void
Application::Term()
{
  std::cout << "Jivaro Application Terminate!!" << std::endl;
}

bool
Application::Update()
{
  CommandManager::Get()->ExecuteDeferredCommands();
  
  Time* time = Time::Get();
  float currentTime(time->GetActiveTime());
  int playback = time->Playback();
  if( playback == Time::PLAYBACK_WAITING) return true;

  static bool vSync = true;
  if(vSync && time->IsPlaying())
    glfwPollEvents();
  else
    glfwWaitEvents();
  
  // update model
  if (!WindowRegistry::GetPopup() && (playback > Time::PLAYBACK_IDLE || WindowRegistry::IsToolInteracting())) {
    if(_model->GetExec() ) 
      _model->UpdateExec(currentTime);

    _model->Update(currentTime);
    WindowRegistry::SetAllWindowsDirty();
  }

  if(!WindowRegistry::Update())
    return false;

  CommandManager::Get()->ExecuteCommands();
  return true;
}


// COMMANDS
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
    ADD_COMMAND(DuplicatePrimCommand, _model->GetSessionLayer(), item.path);
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
  ADD_COMMAND(SaveLayerCommand, _model->GetRootLayer());
}

void Application::SaveSceneAs(const std::string& filename)
{
  ADD_COMMAND(SaveLayerAsCommand, _model->GetRootLayer(), filename);
}

// ---------------------------------------------------------------------------------------------
// Notices Callbacks
//----------------------------------------------------------------------------------------------
void 
Application::SelectionChangedCallback(const SelectionChangedNotice& n)
{  
  for (Window* window : WindowRegistry::GetWindows())
    if(window->GetTool()->IsActive())
      window->GetTool()->ResetSelection();
  
 
  _model->Update(Time::Get()->GetActiveTime());
  WindowRegistry::SetAllWindowsDirty();
}

void 
Application::NewSceneCallback(const NewSceneNotice& n)
{
  if(_model->GetExec()) _model->TerminateExec();

  _model->ClearSelection();
  WindowRegistry::SetAllWindowsDirty();
}

void 
Application::SceneChangedCallback(const SceneChangedNotice& n)
{
  /*
  WindowRegistry::GetMainWindow()->GetTool()->ResetSelection();
  for (auto& window : WindowRegistry::GetChildWindows()) {
    window->GetTool()->ResetSelection();
  }
  */
  _model->Update(Time::Get()->GetActiveTime());
  WindowRegistry::SetAllWindowsDirty();
}

void
Application::AttributeChangedCallback(const AttributeChangedNotice& n)
{
  if (_model->GetExec()) 
    _model->UpdateExec(Time::Get()->GetActiveTime());
  
  /*
  WindowRegistry::GetMainWindow()->GetTool()->ResetSelection();
  for (auto& child : WindowRegistry::GetChildWindows()) {
    child->GetTool()->ResetSelection();
  }
  */
  _model->Update(Time::Get()->GetActiveTime());
  WindowRegistry::SetAllWindowsDirty();

}

void
Application::TimeChangedCallback(const TimeChangedNotice& n)
{
  if (_model->GetExec()) 
    _model->UpdateExec(Time::Get()->GetActiveTime());

  WindowRegistry::SetAllWindowsDirty();
}

void
Application::UndoStackNoticeCallback(const UndoStackNotice& n)
{
  ADD_COMMAND(UsdGenericCommand);
}



JVR_NAMESPACE_CLOSE_SCOPE
