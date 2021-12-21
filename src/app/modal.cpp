#include "modal.h"
#include "application.h"
#include "window.h"
#include "../ui/filebrowser.h"
#include "../ui/dummy.h"


JVR_NAMESPACE_OPEN_SCOPE

//==============================================================================
// Base Modal Window
//==============================================================================
BaseModal::BaseModal(int x, int y, int width, int height, const std::string& name)
  : _x(x)
  , _y(y)
  , _width(width)
  , _height(height)
  , _window(NULL)
  , _status(ACTIVE)
  , _title(name)
{
}

BaseModal::~BaseModal()
{
  //if(_ui) delete _ui;
  if(_window) delete _window;
}

void BaseModal::Init()
{
  Application* app = APPLICATION;
  Window* mainWindow = app->GetMainWindow();
  mainWindow->SetIdle(true);

  _window = app->CreateChildWindow(_x, _y, _width, _height, mainWindow, _title);
  _window->Init(app);
}

void BaseModal::Term()
{
  Application* app = APPLICATION;
  Window* mainWindow = app->GetMainWindow();
  
  mainWindow->SetIdle(false);
  mainWindow->SetGLContext();
}

void BaseModal::Loop()
{
  while (!glfwWindowShouldClose(_window->GetGlfwWindow()) && _status == ACTIVE) {
    _window->Draw();
    glfwSwapBuffers(_window->GetGlfwWindow());
    glfwPollEvents();
    _LoopImpl();
  }
}

//==============================================================================
// File Browser Modal Window
//==============================================================================
ModalFileBrowser::ModalFileBrowser(int x, int y, const std::string& title, 
  ModalFileBrowser::Mode mode)
  : BaseModal(x, y, 600, 400, title)
  , _mode( mode )
{
  BaseModal::Init();
  View* view = _window->GetMainView();
  FileBrowserUI* browser = NULL;

  switch(_mode) {
    case Mode::OPEN:
      browser = new FileBrowserUI(view, _title, FileBrowserUI::Mode::OPEN);
      break;
    case Mode::SAVE:
      browser = new FileBrowserUI(view, _title, FileBrowserUI::Mode::SAVE);
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
}

void ModalFileBrowser::_LoopImpl()
{
  std::cout << "FILE BROWSER LOOP START..." << std::endl;
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
  std::cout << "FILE BROWSER LOOP END..." << std::endl;
}

//==============================================================================
// Demo Modal Window
//==============================================================================
ModalDemo::ModalDemo(int x, int y, const std::string& title)
  : BaseModal(x, y, 800, 800, title)
{
  BaseModal::Init();
  View* view = _window->GetMainView();
  _ui = new DummyUI(view, title);
}

void ModalDemo::_LoopImpl()
{
}

//==============================================================================
// Modal Menu Window
//==============================================================================
ModalMenu::ModalMenu(int x, int y, const std::string& title)
  : BaseModal(x, y, 800, 800, title)
{
  BaseModal::Init();
  View* view = _window->GetMainView();
  _ui = new DummyUI(view, title);
}

void ModalMenu::_LoopImpl()
{
}

JVR_NAMESPACE_CLOSE_SCOPE