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

WindowRegistry* WindowRegistry::New() 
{
  if(WindowRegistrySingleton == nullptr) 
    WindowRegistrySingleton = new WindowRegistry();
  return WindowRegistrySingleton;
}

WindowRegistry* WindowRegistry::Get() { 
  return WindowRegistrySingleton; 
};

bool
WindowRegistry::Update()
{
  size_t  updated = 0;
  // draw popup
  if (_popup) {
    Window* window = _popup->GetView()->GetWindow();
    window->DrawPopup(_popup);
    if (_popup->IsDone() || _popup->IsCancel()) {
      _popup->Terminate();
      delete _popup;
      _popup = nullptr;
    }
  }
  else 
    for (auto& window : _windows)
      updated += window->Update();

  return updated == _windows.size();

}

void 
WindowRegistry::SetActiveTool(size_t t)
{
  Tool* tool = _windows[0]->GetTool();
  size_t lastActiveTool = tool->GetActiveTool();
  if(t != lastActiveTool) {
    for (auto& window : _windows) {
      window->GetTool()->SetActiveTool(t);
    }
  }
}

bool
WindowRegistry::IsToolInteracting()
{
  for(auto& window: _windows)
    if(window->GetTool()->IsInteracting())
      return true;
  return false;
}

void
WindowRegistry::AddWindow(Window* window)
{
  _windows.push_back(window);
  window->SetGLContext();
  _activeWindow = window;
}

void 
WindowRegistry::RemoveWindow(Window* window)
{
  std::vector<Window*>::iterator it = _windows.begin();
  for (; it < _windows.end(); ++it) {
    if(*it == window) {
      _windows.erase(it);
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
  for(auto& window: _windows)
    window->DirtyAllViews(false);
}


// popup
//----------------------------------------------------------------------------
void
WindowRegistry::SetPopup(PopupUI* popup)
{
  popup->SetParent(GetActiveWindow()->GetMainView());
  _popup = popup;
  for (auto& window: _windows)
    window->CaptureFramebuffer();
}

/*
void
WindowRegistry::SetPopupDeferred(PopupUI* popup)
{
  popup->SetParent(GetActiveWindow()->GetMainView());
  _popup = popup;
  _needCaptureFramebuffers = true;
}
*/

void
WindowRegistry::UpdatePopup()
{
  if (_popup) {
    if (!_popup->IsDone())return;
    _popup->Terminate();
    delete _popup;
  }
  _popup = nullptr;
  SetAllWindowsDirty();
}


// create full screen window
//----------------------------------------------------------------------------
Window*
WindowRegistry::CreateFullScreenWindow(const std::string& name)
{
  Window* window = new Window("Jivaro", GfVec4i(), true);
  AddWindow(window);
  return window;
}

// create standard window
//----------------------------------------------------------------------------
Window*
WindowRegistry::CreateStandardWindow(const std::string& name, const GfVec4i& dimension)
{
  Window* window = new Window(name, dimension, false);
  AddWindow(window);
  return window;
}


// create child window
//----------------------------------------------------------------------------
Window*
WindowRegistry::CreateChildWindow(const std::string& name, const GfVec4i& dimension, Window* parent)
{
  Window* child = new Window(name, dimension, false, parent);
  AddWindow(child);
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