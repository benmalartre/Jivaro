#include "modal.h"
#include "application.h"
#include "window.h"
#include "../ui/filebrowser.h"


AMN_NAMESPACE_OPEN_SCOPE

BaseModal::BaseModal(int width, int height, const std::string& name)
  : _width(width)
  , _height(height)
  , _window(NULL)
  , _status(ACTIVE)
{
}

BaseModal::~BaseModal()
{
  if(_window) delete _window;
}

void BaseModal::Init()
{
  Application* app = AMN_APPLICATION;
  Window* mainWindow = app->GetMainWindow();
  mainWindow->SetIdle(true);

  _window = app->CreateChildWindow(_width, _height, mainWindow, _title);
  _window->Init(app);

  View* view = _window->GetMainView();
  /*
  switch(_mode) {
    case Mode::FILE
  }
  
  FileBrowserUI* browser = new FileBrowserUI(view, "FileBrowser", FileBrowserUI::Mode::OPEN);

  // set initial path
  if(!strlen(_folder.c_str()) || !DirectoryExists(_folder)) 
    browser->SetPath( GetInstallationFolder());
  else browser->SetPath(_folder);

  _ui = browser;
  */
}

void BaseModal::Term()
{
  delete _window;

  Application* app = AMN_APPLICATION;
  Window* mainWindow = app->GetMainWindow();
  
  mainWindow->SetIdle(false);
  mainWindow->SetGLContext();
}

bool BaseModal::Loop()
{
  FileBrowserUI* browser = (FileBrowserUI*)_ui;
  std::string result;
  while (!glfwWindowShouldClose(_window->GetGlfwWindow()) && _status == ACTIVE) {
    _window->SetGLContext();
    _window->Draw();
    glfwSwapBuffers(_window->GetGlfwWindow());
    glfwPollEvents();
    bool browse = browser->IsBrowsing();
    if(!browse) {
      if(!browser->IsCanceled()) {
        //result = browser->GetResult();
        result = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usd";
      }
    }
  }
  
  return true;//result;
}

AMN_NAMESPACE_CLOSE_SCOPE