#include "application.h"
#include "../widgets/viewport.h"
#include "../widgets/menu.h"
#include "../widgets/graph.h"
#include "../widgets/dummy.h"
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/usd/usdGeom/sphere.h>
#include "../tests/stageGraph.h"
#include "../tests/stageUI.h"

AMN_NAMESPACE_OPEN_SCOPE

PXR_NAMESPACE_USING_DIRECTIVE
const char* Application::APPLICATION_NAME = "Amnesie";

// constructor
//----------------------------------------------------------------------------
Application::Application(unsigned width, unsigned height):
  _mainWindow(NULL), _context(NULL)
{  
  _context = new UsdEmbreeContext();
  EMBREE_CTXT = _context;
  _width = width;
  _height = height;
  _mainWindow = CreateStandardWindow(width, height);
  _test = NULL;
};

Application::Application(bool fullscreen):
  _mainWindow(NULL), _context(NULL)
{
  _context = new UsdEmbreeContext();
  EMBREE_CTXT = _context;
  _mainWindow = CreateFullScreenWindow();
  glfwGetWindowSize(_mainWindow->GetGlfwWindow(), &_width, &_height);
  _test = NULL;
};

// destructor
//----------------------------------------------------------------------------
Application::~Application()
{
  if(_context)delete _context;
  if(_mainWindow) delete _mainWindow;
  if(_test) delete _test;
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
  std::string filename = 
    "/Users/benmalartre/Documents/RnD/amnesie/usd/result.usda";

  // build test scene
  //pxr::TestScene(filename);

  // create window
  _mainWindow->SetContext();
  View* mainView = _mainWindow->GetMainView();
  _mainWindow->SplitView(mainView, 0.1, true);
  _mainWindow->SplitView(mainView->GetRight(), 0.7, true);
  View* middleView = mainView->GetRight()->GetLeft();
  View* graphView = mainView->GetRight()->GetRight();

  _mainWindow->SplitView(middleView, 0.8, false);
  _mainWindow->SplitView(middleView->GetLeft(), 0.33, false);
  //_mainWindow->SplitView(middleView->GetRight(), 50, false);

/*
  GraphUI* graph = new GraphUI(graphView, "GraphUI");
  ViewportUI* viewport = new ViewportUI(middleView, EMBREE);
  //DummyUI* dummy = new DummyUI(mainView->GetLeft(), "DummyUI");
  
  //MenuUI* menu = new MenuUI(mainView->GetLeft());
  _mainWindow->CollectLeaves();

  pxr::UsdStageRefPtr stage1 = pxr::UsdStage::Open(filename);
  _stages.push_back(stage1);
  TestStageUI(graph, _stages);
 
  EMBREE_CTXT->Resize(viewport->GetWidth(), viewport->GetHeight());
  std::cout << "EMBREE CTXT : " << EMBREE_CTXT->_width << "," << EMBREE_CTXT->_height << std::endl;
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
  */
  //_mainWindow->CollectLeaves();
  //_mainWindow->DummyFill();
}

// main loop
void 
Application::MainLoop()
{
  _mainWindow->MainLoop();

}

AMN_NAMESPACE_CLOSE_SCOPE

