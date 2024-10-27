#include "modal.h"
#include "application.h"
#include "window.h"
#include "../ui/fileBrowser.h"
#include "../ui/demo.h"


JVR_NAMESPACE_OPEN_SCOPE

//==============================================================================
// Base Modal Window
//==============================================================================
ModalBase::ModalBase(int x, int y, int width, int height, const std::string& name)
  : _x(x)
  , _y(y)
  , _width(width)
  , _height(height)
  , _window(NULL)
  , _status(ACTIVE)
  , _title(name)
{
}

ModalBase::~ModalBase()
{
}

void ModalBase::Init()
{
  Application* app = Application::Get();
  Window* mainWindow = app->GetMainWindow();
  mainWindow->SetIdle(true);

  _window = app->CreateChildWindow(_title, GfVec4i(_x, _y, _width, _height), mainWindow);
  _window->SetDesiredLayout(_window->GetLayout());
}

void ModalBase::Term()
{
  if(_window) delete _window;
  
  Application* app = Application::Get();
  Window* mainWindow = app->GetMainWindow();
  
  mainWindow->SetIdle(false);
  mainWindow->SetGLContext();
}

void ModalBase::Loop()
{
  while (!glfwWindowShouldClose(_window->GetGlfwWindow()) && _status == ACTIVE) {
    _window->Draw(false);
    glfwPollEvents();
    _LoopImpl();
  }
}

//==============================================================================
// File Browser Modal Window
//==============================================================================
ModalFileBrowser::ModalFileBrowser(int x, int y, const std::string& title, 
  ModalFileBrowser::Mode mode)
  : ModalBase(x, y, 600, 400, title)
  , _mode( mode )
{
  ModalBase::Init();
  View* view = _window->GetMainView();
  FileBrowserUI* browser = NULL;

  switch(_mode) {
    case Mode::OPEN:
      browser = new FileBrowserUI(view, FileBrowserUI::Mode::OPEN);
      break;
    case Mode::SAVE:
      browser = new FileBrowserUI(view, FileBrowserUI::Mode::SAVE);
      break;
    case Mode::SELECT:
    case Mode::MULTI: 
      break;
  }
  if(browser) {
    // set initial path
    if(!strlen(_folder.c_str()) || !DirectoryExists(_folder)) 
      browser->SetPath( GetInstallationFolder());
    else browser->SetPath(_folder);

    _ui = browser;
  }
  _window->SetActiveView(view);
}

void ModalFileBrowser::_LoopImpl()
{
  FileBrowserUI* browser = (FileBrowserUI*)_ui;
  bool browse = browser->IsBrowsing();
  if(!browse) {
    if(browser->IsCanceled()) {
      _status = Status::CANCEL;
      _result = "";
    } else {
      if(browser->GetResult(_result)) {
        _status = Status::OK;
      } else {
        _status = Status::FAIL;
      }
    } 
  }
}

//==============================================================================
// Demo Modal Window
//==============================================================================
ModalDemo::ModalDemo(int x, int y, const std::string& title)
  : ModalBase(x, y, 800, 800, title)
{
  ModalBase::Init();
  View* view = _window->GetMainView();
  _ui = new DemoUI(view);
}

void ModalDemo::_LoopImpl()
{

}

JVR_NAMESPACE_CLOSE_SCOPE