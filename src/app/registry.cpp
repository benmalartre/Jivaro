#include "../app/registry.h"

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