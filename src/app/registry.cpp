#include "../app/registry.h"
#include "../app/window.h"
#include "../app/tools.h"

JVR_NAMESPACE_OPEN_SCOPE

// Registry Window singleton
//----------------------------------------------------------------------------
WindowRegistry* WindowRegistry::_singleton=nullptr;

WindowRegistry* WindowRegistry::Get() { 
  if(_singleton==nullptr){
    _singleton = new WindowRegistry();
  }
  return _singleton; 
};

void
WindowRegistry::Update()
{
  _mainWindow->Update();
  for (auto& childWindow : _childWindows)childWindow->Update();
}

void 
WindowRegistry::SetActiveTool(size_t t)
{
  std::cout << "Set Active Tool : " << t << std::endl;
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
WindowRegistry::IsToolInteracting()
{
  Tool* tool = _mainWindow->GetTool();
  return tool->IsInteracting();
}

void
WindowRegistry::AddWindow(Window* window)
{
 _childWindows.push_back(window);
  window->SetGLContext();
}

void 
WindowRegistry::RemoveWindow(Window* window)
{
  std::vector<Window*>::iterator it = _childWindows.begin();
  for (; it < _childWindows.end(); ++it) {
    if(*it == window) {
      _childWindows.erase(it);
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
  _mainWindow->DirtyAllViews(false);
  for(auto& childWindow: _childWindows)
    childWindow->DirtyAllViews(false);
}

// create full screen window
//----------------------------------------------------------------------------
Window*
WindowRegistry::CreateFullScreenWindow(const std::string& name)
{
  _mainWindow = Window::CreateFullScreenWindow(name);
  _activeWindow = _mainWindow;
  return _mainWindow;
}

// create child window
//----------------------------------------------------------------------------
Window*
WindowRegistry::CreateChildWindow(const std::string& name, const GfVec4i& dimension, Window* parent)
{
  Window* childWindow = Window::CreateChildWindow(name, dimension, parent);
  AddWindow(childWindow);
  _activeWindow = childWindow;
  return childWindow;
    
}

// create standard window
//----------------------------------------------------------------------------
Window*
WindowRegistry::CreateStandardWindow(const std::string& name, const GfVec4i& dimension)
{
  _mainWindow = Window::CreateStandardWindow(name, dimension);
  _activeWindow = _mainWindow;
  return _mainWindow;
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