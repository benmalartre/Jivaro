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
  // get monitors info
  GetMonitors();
  
  _context = new AmnUsdEmbreeContext();
  EMBREE_CTXT = _context;
  _width = width;
  _height = height;
  _mainWindow = CreateStandardWindow(width, height);
  _test = NULL;
};

AmnApplication::AmnApplication(bool fullscreen):
  _mainWindow(NULL), _context(NULL)
{
  _context = new AmnUsdEmbreeContext();
  EMBREE_CTXT = _context;
  _mainWindow = CreateFullScreenWindow();
  glfwGetWindowSize(_mainWindow->GetGlfwWindow(), &_width, &_height);
  _test = NULL;
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

  _mainWindow->CollectLeaves();

  pxr::UsdStageRefPtr stage1 = pxr::UsdStage::Open(filename);
  _stages.push_back(stage1);
  TestStageUI(graph, _stages);
 
  EMBREE_CTXT->Resize(viewport->GetWidth(), viewport->GetHeight());
  EMBREE_CTXT->SetFilePath(filename);
  EMBREE_CTXT->InitDevice(viewport->GetCamera());
  EMBREE_CTXT->TraverseStage();
  EMBREE_CTXT->CommitDevice();
  
  std::string imageDirectory = "/Users/benmalartre/Documents/RnD/amnesie/images";
  int imageId = FilesInDirectory(imageDirectory.c_str()) + 1;
  std::string imagePath = imageDirectory + "/img.";
  std::string imageExt = ".jpg";
  embree::FileName outputImageFilename(imagePath + std::to_string(imageId) + imageExt);

  viewport->GetCamera()->ComputeFrustum();

  RenderToFile(outputImageFilename, viewport->GetCamera(), 2048, 1024);
  RenderToMemory(viewport->GetCamera());
  viewport->SetContext(EMBREE_CTXT);
    
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

