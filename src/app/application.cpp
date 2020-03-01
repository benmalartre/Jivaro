#include "application.h"
#include <unistd.h>
#include <thread>
#include "../widgets/viewport.h"
#include "../widgets/menu.h"

AMN_NAMESPACE_OPEN_SCOPE

const char* AmnApplication::APPLICATION_NAME = "Amnesie";

// constructor
//----------------------------------------------------------------------------
AmnApplication::AmnApplication(unsigned width, unsigned height):
  _mainWindow(NULL), _context(NULL)
{
  EMBREE_CTXT = new AmnUsdEmbreeContext();
  _width = width;
  _height = height;
  _mainWindow = CreateStandardWindow(width, height);
};

AmnApplication::AmnApplication(bool fullscreen):
  _mainWindow(NULL), _context(NULL)
{
  EMBREE_CTXT = new AmnUsdEmbreeContext();
  _mainWindow = CreateFullScreenWindow();
  glfwGetWindowSize(_mainWindow->GetGlfwWindow(), &_width, &_height);
};

// create full screen window
//----------------------------------------------------------------------------
AmnWindow*
AmnApplication::CreateFullScreenWindow()
{
  return AmnWindow::CreateFullScreenWindow();
}

// create standard window
//----------------------------------------------------------------------------
AmnWindow*
AmnApplication::CreateStandardWindow(int width, int height)
{
  return AmnWindow::CreateStandardWindow(width, height);
}

// init application
//----------------------------------------------------------------------------
void 
AmnApplication::Init()
{
  _mainWindow->SetContext();
  AmnView* mainView = _mainWindow->GetMainView();
  _mainWindow->SplitView(mainView, 10, true);

  AmnMenuUI* menu = new AmnMenuUI(mainView->GetLeft());
  AmnViewportUI* viewport = new AmnViewportUI(mainView->GetRight(), EMBREE);

  /*
  
  _mainWindow->SplitView(mainView->GetRight(), 75, true);
  _mainWindow->SplitView(mainView->GetRight()->GetLeft(), 25, false);
  _mainWindow->SplitView(mainView->GetRight()->GetLeft()->GetRight(), 75, false);
  //std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/kitchen_set/kitchen_set.usd";
  //std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/Kitchen_set/assets/Clock/Clock.usd";
  //std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/UsdSkelExamples/HumanFemale/HumanFemale.usd";
  */
  
  EMBREE_CTXT->Resize(1024, 720);
  EMBREE_CTXT->SetFilePath("/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usd");
  EMBREE_CTXT->InitDevice();
  EMBREE_CTXT->TraverseStage();
  EMBREE_CTXT->CommitDevice();
  
  embree::FileName outputImageFilename("/Users/benmalartre/Documents/RnD/embree/embree-usd/images/img.011.jpg");
  
  //RenderToFile(outputImageFilename);
  RenderToMemory();
  viewport->SetPixels(EMBREE_CTXT->_width, EMBREE_CTXT->_height, EMBREE_CTXT->_pixels);
  
  //_mainWindow->CollectLeaves();
  //_mainWindow->DummyFill();
}

// main loop
void 
AmnApplication::MainLoop()
{
  _mainWindow->MainLoop();
  
}

AMN_NAMESPACE_CLOSE_SCOPE

