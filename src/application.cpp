#include "application.h"
#include <unistd.h>
#include <thread>

namespace AMN {

  const char* Application::APPLICATION_NAME = "Usd-Embree View";

  // constructor
  //----------------------------------------------------------------------------
  //Application::Application():_pixels(NULL){};
  Application::Application(unsigned width, unsigned height):
    _mainWindow(NULL)
  {
    _width = width;
    _height = height;
    _mainWindow = CreateStandardWindow(width, height);
  };

  Application::Application(bool fullscreen):
    _mainWindow(NULL)
  {
    _mainWindow = CreateFullScreenWindow();
    glfwGetWindowSize(_mainWindow->GetWindow(), &_width, &_height);

    int N = 7;
    int H = _mainWindow->GetHeight() / N;
    //_mainWindow->AddView()
    for(int i = 0; i< N; ++i)
      _mainWindow->AddSplitter(0, i * H, _mainWindow->GetWidth(), H, 50);
  };

  

  // create full screen window
  //----------------------------------------------------------------------------
  Window*
  Application::CreateFullScreenWindow()
  {
    return Window::CreateFullScreenWindow();
  }

  // create standard window
  //----------------------------------------------------------------------------
  Window*
  Application::CreateStandardWindow(int width, int height)
  {
    return Window::CreateStandardWindow(width, height);
  }
  
  // main loop
  void 
  Application::MainLoop()
  {
    _mainWindow->MainLoop();
   
  }

} // namespace AMN

