#include "../app/registry.h"
#include "../app/window.h"
#include "../app/view.h"
#include "../app/tools.h"
#include "../ui/ui.h"
#include "../ui/popup.h"

JVR_NAMESPACE_OPEN_SCOPE

// Registry Window singleton
//----------------------------------------------------------------------------
static WindowRegistry* WindowRegistrySingleton=nullptr;



bool WindowRegistry::PlaybackAllViews = false;

WindowRegistry* WindowRegistry::Get() { 
  if(WindowRegistrySingleton == nullptr) 
    WindowRegistrySingleton = new WindowRegistry();
  return WindowRegistrySingleton;
};


bool
WindowRegistry::Update()
{
  size_t  updated = 0;
  PopupUI* popup = WindowRegistrySingleton->_popup;
  // draw popup
  if (popup) {
    Window* window = popup->GetView()->GetWindow();
    window->DrawPopup(popup);
    if (popup->IsDone() || popup->IsCancel()) {
      popup->Terminate();
      delete popup;
      WindowRegistrySingleton->_popup = nullptr;
    }
    return true;
  }
  else 
    for (auto& window : WindowRegistrySingleton->_windows)
      updated += window->Update();

    return updated == WindowRegistrySingleton->_windows.size();

}

void 
WindowRegistry::SetActiveTool(size_t t)
{
  Tool* tool = WindowRegistrySingleton->_windows[0]->GetTool();
  size_t lastActiveTool = tool->GetActiveTool();
  if(t != lastActiveTool) {
    for (auto& window : WindowRegistrySingleton->_windows) {
      window->GetTool()->SetActiveTool(t);
    }
  }
}

bool
WindowRegistry::IsToolInteracting()
{
  for(auto& window: WindowRegistrySingleton->_windows)
    if(window->GetTool()->IsInteracting())
      return true;
  return false;
}

Window*
WindowRegistry::GetActiveWindow()
{
  return WindowRegistrySingleton->_activeWindow;
}

void
WindowRegistry::SetActiveWindow(Window* window)
{
  WindowRegistrySingleton->_activeWindow = window;
}

std::vector<Window*>& 
WindowRegistry::GetWindows()
{
  return WindowRegistrySingleton->_windows;
}

Window*
WindowRegistry::GetWindow(size_t index)
{
  if(index < WindowRegistrySingleton->_windows.size())
    return WindowRegistrySingleton->_windows[index];
  return nullptr;
}

void
WindowRegistry::AddWindow(Window* window)
{
  WindowRegistrySingleton->_windows.push_back(window);
  window->SetGLContext();
  WindowRegistrySingleton->_activeWindow = window;
}

void 
WindowRegistry::RemoveWindow(Window* window)
{
  std::vector<Window*>::iterator it = WindowRegistrySingleton->_windows.begin();
  for (; it < WindowRegistrySingleton->_windows.end(); ++it) {
    if(*it == window) {
      WindowRegistrySingleton->_windows.erase(it);
      delete window;
      break;
    }
  }
}

void 
WindowRegistry::SetWindowDirty(Window* window)
{
  window->DirtyAllViews(false);
}

void 
WindowRegistry::SetAllWindowsDirty()
{
  for(auto& window: WindowRegistrySingleton->_windows)
    window->DirtyAllViews(false);
}


// popup
//----------------------------------------------------------------------------
PopupUI* 
WindowRegistry::GetPopup()
{
  return WindowRegistrySingleton->_popup;
}

void
WindowRegistry::SetPopup(PopupUI* popup)
{
  popup->SetParent(WindowRegistrySingleton->_activeWindow->GetMainView());
  WindowRegistrySingleton->_popup = popup;
  for (auto& window: WindowRegistrySingleton->_windows)
    window->CaptureFramebuffer();
}


void
WindowRegistry::UpdatePopup()
{
  if (WindowRegistrySingleton->_popup) {
    if (!WindowRegistrySingleton->_popup->IsDone())return;
    WindowRegistrySingleton->_popup->Terminate();
    delete WindowRegistrySingleton->_popup;
  }
  WindowRegistrySingleton->_popup = nullptr;
  SetAllWindowsDirty();
}

// playback viewport
bool 
WindowRegistry::IsPlaybackView(View* view) 
{ 
  return view == WindowRegistrySingleton->_playbackView || 
    PlaybackAllViews; 
};

void 
WindowRegistry::SetPlaybackView(View* view)
{
  if(view == WindowRegistrySingleton->_playbackView)return;
  if(WindowRegistrySingleton->_playbackView)
    WindowRegistrySingleton->_playbackView->ClearFlag(View::TIMEVARYING);

  WindowRegistrySingleton->_playbackView = view;
  if(WindowRegistrySingleton->_playbackView)
    WindowRegistrySingleton->_playbackView->SetFlag(View::TIMEVARYING);
};


// create full screen window
//----------------------------------------------------------------------------
Window*
WindowRegistry::CreateFullScreenWindow(const std::string& name)
{
  WindowRegistry* registry = Get();
  Window* window = new Window("Jivaro", GfVec4i(), true);
  WindowRegistrySingleton->AddWindow(window);
  return window;
}

// create standard window
//----------------------------------------------------------------------------
Window*
WindowRegistry::CreateStandardWindow(const std::string& name, const GfVec4i& dimension)
{
  WindowRegistry* registry = Get();
  Window* window = new Window(name, dimension, false);
  WindowRegistrySingleton->AddWindow(window);
  return window;
}


// create child window
//----------------------------------------------------------------------------
Window*
WindowRegistry::CreateChildWindow(const std::string& name, const GfVec4i& dimension, Window* parent)
{
  WindowRegistry* registry = Get();
  Window* child = new Window(name, dimension, false, parent);
  WindowRegistrySingleton->AddWindow(child);
  return child;
    
}



// Registry UI singleton
//----------------------------------------------------------------------------
UIRegistry* UIRegistry::_singleton=nullptr;

UIRegistry* UIRegistry::Get() { 
  if(_singleton==nullptr){
    _singleton = new UIRegistry();
  }
  return _singleton; 
};

JVR_NAMESPACE_CLOSE_SCOPE