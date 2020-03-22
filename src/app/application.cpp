#include "application.h"
#include "../widgets/viewport.h"
#include "../widgets/menu.h"
#include "../widgets/graph.h"
#include "../widgets/dummy.h"
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
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

void _RecurseSplitView(View* view, int depth, bool horizontal)
{
  if(depth < 3)
  {
    view->Split(0.5, horizontal);
    _RecurseSplitView(view->GetLeft(), depth + 1, horizontal);
    _RecurseSplitView(view->GetRight(), depth + 1, horizontal);
    view->SetPerc(0.5);
  }
}

// init application
//----------------------------------------------------------------------------
void 
Application::Init()
{
  std::string filename = 
    //"/Users/benmalartre/Documents/RnD/USD_BUILD/assets/UsdSkelExamples/HumanFemale/HumanFemal.usda";
    //"/Users/benmalartre/Documents/RnD/USD_BUILD/assets/Kitchen_set/Kitchen_set.usd";
    "/Users/benmalartre/Documents/RnD/amnesie/usd/result.usda";

  // build test scene
  //pxr::TestScene(filename);

  // create window
  _mainWindow->SetContext();
  View* mainView = _mainWindow->GetMainView();
  _RecurseSplitView(mainView, 0, true);
   
   _mainWindow->CollectLeaves();

   _mainWindow->Resize(_width, _height);

  /*
  _mainWindow->SplitView(mainView, 0.1, true);
  _mainWindow->SplitView(mainView->GetRight(), 0.7, true);
  View* middleView = mainView->GetRight()->GetLeft();
  View* graphView = mainView->GetRight()->GetRight();

  _mainWindow->SplitView(middleView, 0.5, false);
  _mainWindow->SplitView(middleView->GetLeft(), 0.5, false);
  _mainWindow->SplitView(middleView->GetRight(), 0.5, false);
  _mainWindow->Resize(_width, _height);
  */
  /*
  View* viewportView = middleView->GetLeft()->GetRight();
  _mainWindow->CollectLeaves();
  GraphUI* graph = new GraphUI(graphView, "GraphUI");
  ViewportUI* viewport = new ViewportUI(viewportView, EMBREE);
  //DummyUI* dummy = new DummyUI(mainView->GetLeft(), "DummyUI");
  
  MenuUI* menu = new MenuUI(mainView->GetLeft());
  */
/*

  pxr::UsdStageRefPtr stage1 = pxr::UsdStage::Open(filename);
  _stages.push_back(stage1);
  TestStageUI(graph, _stages);

  if(pxr::UsdGeomGetStageUpAxis	(stage1) ==  pxr::UsdGeomTokens->z)
  {
    std::cout << "############ Z is UP !!!! ################" << std::endl;
    pxr::UsdPrim root = stage1->GetPseudoRoot();
    pxr::UsdGeomXformCommonAPI xformApi(root);
    xformApi.SetRotate(pxr::GfVec3f(90, 90, 90));
  }	
 
  _context->Resize(viewport->GetWidth(), viewport->GetHeight());
  _context->SetFilePath(filename);
  _context->InitDevice(viewport->GetCamera());
  _context->TraverseStage();
  _context->CommitDevice();
  
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
