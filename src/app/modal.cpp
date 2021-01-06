#include "modal.h"
#include "application.h"
#include "window.h"
#include "../ui/filebrowser.h"


AMN_NAMESPACE_OPEN_SCOPE

Modal::Modal(int width, int height, const std::string& name, Mode mode)
  : _width(width), _height(height), _mode(mode), _window(NULL)
{
}

Modal::~Modal()
{
  if(_window) delete _window;
}

void Modal::Loop()
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
  */
  FileBrowserUI* browser = new FileBrowserUI(view, "FileBrowser", FileBrowserUI::Mode::OPEN);

  // set initial path
  if(!strlen(_folder.c_str()) || !DirectoryExists(_folder)) 
    browser->SetPath( GetInstallationFolder());
  else browser->SetPath(_folder);

  bool browse = true;
  std::string result;
  while(browse) {
    _window->SetGLContext();
    _window->Draw();
    glfwSwapBuffers(_window->GetGlfwWindow());
    glfwPollEvents();
    browse = browser->IsBrowsing();
    if(!browse) {
      if(!browser->IsCanceled()) {
        //result = browser->GetResult();
        result = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usd";
      }
    }
  }
  delete _window;
  mainWindow->SetIdle(false);
  mainWindow->SetGLContext();
  //return result;
}

AMN_NAMESPACE_CLOSE_SCOPE