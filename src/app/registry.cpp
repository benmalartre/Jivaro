#include "../app/registry.h"
#include "../app/window.h"
#include "../app/tools.h"

JVR_NAMESPACE_OPEN_SCOPE

// Registry Window singleton
//----------------------------------------------------------------------------
RegistryWindow* RegistryWindow::_singleton=nullptr;

RegistryWindow* RegistryWindow::Get() { 
  if(_singleton==nullptr){
    _singleton = new RegistryWindow();
  }
  return _singleton; 
};

void
RegistryWindow::Update()
{
  _mainWindow->Update();
  for (auto& childWindow : _childWindows)childWindow->Update();
}

void 
RegistryWindow::SetActiveTool(size_t t)
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
RegistryWindow::IsToolInteracting()
{
  Tool* tool = _mainWindow->GetTool();
  return tool->IsInteracting();
}

void
RegistryWindow::AddWindow(Window* window)
{
 _childWindows.push_back(window);
  window->SetGLContext();
}

void 
RegistryWindow::RemoveWindow(Window* window)
{
  std::vector<Window*>::iterator it = _childWindows.begin();
  for (; it < _childWindows.end(); ++it) {
    if(*it == window) {
      _childWindows.erase(it);
    }
  }
}

void 
RegistryWindow::SetWindowDirty(Window* window)
{
  window->DirtyAllViews(false);
}

void 
RegistryWindow::SetAllWindowsDirty()
{
  _mainWindow->DirtyAllViews(false);
  for(auto& childWindow: _childWindows)
    childWindow->DirtyAllViews(false);
}

// create full screen window
//----------------------------------------------------------------------------
Window*
RegistryWindow::CreateFullScreenWindow(const std::string& name)
{
  _mainWindow = Window::CreateFullScreenWindow(name);
  _activeWindow = _mainWindow;
  return _mainWindow;
}

// create child window
//----------------------------------------------------------------------------
Window*
RegistryWindow::CreateChildWindow(const std::string& name, const GfVec4i& dimension, Window* parent)
{
  Window* childWindow = Window::CreateChildWindow(name, dimension, parent);
  AddWindow(childWindow);
  _activeWindow = childWindow;
  return childWindow;
    
}

// create standard window
//----------------------------------------------------------------------------
Window*
RegistryWindow::CreateStandardWindow(const std::string& name, const GfVec4i& dimension)
{
  _mainWindow = Window::CreateStandardWindow(name, dimension);
  _activeWindow = _mainWindow;
  return _mainWindow;
}


// Registry UI singleton
//----------------------------------------------------------------------------
RegistryUI* RegistryUI::_singleton=nullptr;

RegistryUI* RegistryUI::Get() { 
  if(_singleton==nullptr){
    _singleton = new RegistryUI();
  }
  return _singleton; 
};

JVR_NAMESPACE_CLOSE_SCOPE