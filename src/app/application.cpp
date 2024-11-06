#include <iostream>
#include <string>


#include "../app/application.h"
#include "../app/index.h"
#include "../app/window.h"
#include "../app/commands.h"
#include "../app/layout.h"

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
  : _index(new Index())
{  

};

// destructor
//----------------------------------------------------------------------------
Application::~Application()
{
  delete _index;
};


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
  window->SetIndex(_index);
  Time::Get()->Init(1, 101, 24);
  short tool;
  // setup notifications
  TfNotice::Register(TfCreateWeakPtr(this), &Application::SelectionChangedCallback);
  TfNotice::Register(TfCreateWeakPtr(this), &Application::NewSceneCallback);
  TfNotice::Register(TfCreateWeakPtr(this), &Application::SceneChangedCallback);
  TfNotice::Register(TfCreateWeakPtr(this), &Application::AttributeChangedCallback);
  TfNotice::Register(TfCreateWeakPtr(this), &Application::TimeChangedCallback);
  TfNotice::Register(TfCreateWeakPtr(this), &Application::ToolChangedCallback);
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
  _index->SetStage(_model->GetStage());

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
    if(_index->GetExec() ) 
      _index->UpdateExec(currentTime);

    _index->Update(currentTime);
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
  _index->Update(Time::Get()->GetActiveTime());
}

void 
Application::Redo()
{
  CommandManager::Get()->Redo();
  _index->Update(Time::Get()->GetActiveTime());
}

void 
Application::Delete()
{
  Selection* selection = _model->GetSelection();
  const SdfPathVector& paths = selection->GetSelectedPaths();
  selection->Clear();
  ADD_COMMAND(DeletePrimCommand, _model->GetRootLayer(), paths);
}

void
Application::Duplicate()
{
  Selection* selection = _model->GetSelection();
  if (!selection->IsEmpty()) {
    const Selection::Item& item = selection->GetItem(0);
    ADD_COMMAND(DuplicatePrimCommand, _model->GetRootLayer(), item.path);
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
  
 
  _index->Update(Time::Get()->GetActiveTime());
  WindowRegistry::SetAllWindowsDirty();
}

void 
Application::NewSceneCallback(const NewSceneNotice& n)
{
  CommandManager::Get()->Clear();
  if(_index->GetExec()) _index->TerminateExec();

  _model->ClearSelection();
  WindowRegistry::SetAllWindowsDirty();
}

void 
Application::SceneChangedCallback(const SceneChangedNotice& n)
{
  _index->Update(Time::Get()->GetActiveTime());
  WindowRegistry::SetAllWindowsDirty();
}

void
Application::AttributeChangedCallback(const AttributeChangedNotice& n)
{
  if (_index->GetExec()) 
    _index->UpdateExec(Time::Get()->GetActiveTime());
  
  _index->Update(Time::Get()->GetActiveTime());
  WindowRegistry::SetAllWindowsDirty();

}

void
Application::TimeChangedCallback(const TimeChangedNotice& n)
{
  if (_index->GetExec()) 
    _index->UpdateExec(Time::Get()->GetActiveTime());

  WindowRegistry::SetAllWindowsDirty();
}


void
Application::ToolChangedCallback(const ToolChangedNotice& n)
{
  std::cout << "Tool Change Notice Callback!!!" << std::endl;
  //WindowRegistry::SetActiveTool(n.GetTool());
}

void
Application::UndoStackNoticeCallback(const UndoStackNotice& n)
{
  ADD_COMMAND(UsdGenericCommand);
}



JVR_NAMESPACE_CLOSE_SCOPE
