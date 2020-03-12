#include "application.h"
#include "../widgets/viewport.h"
#include "../widgets/menu.h"
#include "../widgets/graph.h"
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/usd/usdGeom/sphere.h>
#include "../tests/stageGraph.h"
#include "../tests/stageUI.h"

AMN_NAMESPACE_OPEN_SCOPE

PXR_NAMESPACE_USING_DIRECTIVE
const char* AmnApplication::APPLICATION_NAME = "Amnesie";

// constructor
//----------------------------------------------------------------------------
AmnApplication::AmnApplication(unsigned width, unsigned height):
  _mainWindow(NULL), _context(NULL)
{
  _context = new AmnUsdEmbreeContext();
  EMBREE_CTXT = _context;
  _width = width;
  _height = height;
  _mainWindow = CreateStandardWindow(width, height);
};

AmnApplication::AmnApplication(bool fullscreen):
  _mainWindow(NULL), _context(NULL)
{
  _context = new AmnUsdEmbreeContext();
  EMBREE_CTXT = _context;
  _mainWindow = CreateFullScreenWindow();
  glfwGetWindowSize(_mainWindow->GetGlfwWindow(), &_width, &_height);
};

// destructor
//----------------------------------------------------------------------------
AmnApplication::~AmnApplication()
{
  if(_context)delete _context;
  if(_mainWindow) delete _mainWindow;
  if(_test) delete _test;
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
  std::string filename = 
    "/Users/benmalartre/Documents/RnD/amnesie/usd/result.usda";

  // build test scene
  pxr::TestScene(filename);

  // create window
  _mainWindow->SetContext();
  AmnView* mainView = _mainWindow->GetMainView();
  _mainWindow->SplitView(mainView, 50, true);

  AmnGraphUI* graph = new AmnGraphUI(mainView->GetRight(), "GraphUI");
  AmnViewportUI* viewport = new AmnViewportUI(mainView->GetLeft(), EMBREE);

  pxr::UsdStageRefPtr stage1 = pxr::UsdStage::Open(filename);
  _stages.push_back(stage1);
  TestStageUI(graph, _stages);

  /*
  _mainWindow->SplitView(mainView->GetRight(), 75, true);
  _mainWindow->SplitView(mainView->GetRight()->GetLeft(), 25, false);
  _mainWindow->SplitView(mainView->GetRight()->GetLeft()->GetRight(), 75, false);
  //std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/kitchen_set/kitchen_set.usd";
  //std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/Kitchen_set/assets/Clock/Clock.usd";
  //std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/UsdSkelExamples/HumanFemale/HumanFemale.usd";
  */
  EMBREE_CTXT->Resize(viewport->GetWidth(), viewport->GetHeight());
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
  _test->Draw();
}

AMN_NAMESPACE_CLOSE_SCOPE

