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
  , _title(name)
{
}

BaseModal::~BaseModal()
{
  std::cout << "DELETE WINDOW" << std::endl;
  if(_window) delete _window;
    std::cout << "DONE" << std::endl;
}

void BaseModal::Init()
{
  Application* app = AMN_APPLICATION;
  Window* mainWindow = app->GetMainWindow();
  mainWindow->SetIdle(true);

  _window = app->CreateChildWindow(_width, _height, mainWindow, _title);
  _window->Init(app);
}

void BaseModal::Term()
{
  Application* app = AMN_APPLICATION;
  Window* mainWindow = app->GetMainWindow();
  
  mainWindow->SetIdle(false);
  mainWindow->SetGLContext();
}

void BaseModal::Loop()
{
  while (!glfwWindowShouldClose(_window->GetGlfwWindow()) && _status == ACTIVE) {
    _window->SetGLContext();
    _window->Draw();
    glfwSwapBuffers(_window->GetGlfwWindow());
    glfwPollEvents();
    _LoopImpl();
  }
}

ModalFileBrowser::ModalFileBrowser(const std::string& title, 
  ModalFileBrowser::Mode mode)
  : BaseModal(600, 400, title)
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

AMN_NAMESPACE_CLOSE_SCOPE