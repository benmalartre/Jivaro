#include "application.h"
#include <unistd.h>
#include <thread>

namespace AMN {

  const char* Application::APPLICATION_NAME = "Usd-Embree View";

  // constructor
  //----------------------------------------------------------------------------
  //Application::Application():_pixels(NULL){};
  Application::Application(unsigned width, unsigned height):
    _mainWindow(NULL), _context(NULL)
  {
    _width = width;
    _height = height;
    _mainWindow = CreateStandardWindow(width, height);
  };

  Application::Application(bool fullscreen):
    _mainWindow(NULL), _context(NULL)
  {
    _mainWindow = CreateFullScreenWindow();
    glfwGetWindowSize(_mainWindow->GetWindow(), &_width, &_height);
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

  
  // init application
  //----------------------------------------------------------------------------
  void 
  Application::Init()
  {
    View* mainView = _mainWindow->GetMainView();
    UI* viewport = new ViewportUI(mainView, EMBREE);
    _mainWindow->SplitView(mainView, 10, true);
    _mainWindow->BuildSplittersMap();
    /*
    
    _mainWindow->SplitView(mainView->GetRight(), 75, true);
    _mainWindow->SplitView(mainView->GetRight()->GetLeft(), 25, false);
    _mainWindow->SplitView(mainView->GetRight()->GetLeft()->GetRight(), 75, false);
    //std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/kitchen_set/kitchen_set.usd";
    //std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/Kitchen_set/assets/Clock/Clock.usd";
    //std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/UsdSkelExamples/HumanFemale/HumanFemale.usd";
    */
    EMBREE_CTXT = new UsdEmbreeContext(1024, 720);
    EMBREE_CTXT->SetFilePath("/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usd");
    EMBREE_CTXT->InitDevice();
    EMBREE_CTXT->TraverseStage();
    EMBREE_CTXT->CommitDevice();
    
    embree::FileName outputImageFilename("/Users/benmalartre/Documents/RnD/embree/embree-usd/images/img.007.jpg");
    RenderToFile(outputImageFilename);
    
    _mainWindow->CollectLeaves();
    _mainWindow->DummyFill();
  }

  // main loop
  void 
  Application::MainLoop()
  {
    _mainWindow->MainLoop();
   
  }

} // namespace AMN

