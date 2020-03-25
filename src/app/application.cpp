#include "application.h"
#include "../utils/files.h"
#include "../widgets/viewport.h"
#include "../widgets/menu.h"
#include "../widgets/graph.h"
#include "../widgets/timeline.h"
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
  _mainWindow(NULL), _context(NULL),_minTime(1),_maxTime(101),_startTime(1),
  _endTime(101),_currentTime(1),_speed(1),_fps(24),_loop(false),_playback(false)
{  
  _context = new UsdEmbreeContext();
  _mainWindow = CreateStandardWindow(width, height);
  _mainWindow->Init(this);
  _test = NULL;
};

Application::Application(bool fullscreen):
  _mainWindow(NULL), _context(NULL),_minTime(1),_maxTime(101),_startTime(1),
  _endTime(101),_currentTime(1),_speed(1),_fps(24),_loop(false),_playback(false)
{
  _context = new UsdEmbreeContext();
  _mainWindow = CreateFullScreenWindow();
  _mainWindow->Init(this);
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
    view->Split(0.5, horizontal, false);
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
  SetEmbreeContext(_context);
  std::string filename = 
    "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usd";
    //"/Users/benmalartre/Documents/RnD/USD_BUILD/assets/UsdSkelExamples/HumanFemale/HumanFemal.usda";
    //"/Users/benmalartre/Documents/RnD/USD_BUILD/assets/Kitchen_set/Kitchen_set.usd";
    //"/Users/benmalartre/Documents/RnD/amnesie/usd/result.usda";

  // build test scene
  //pxr::TestScene(filename);
  // create imaging gl engine
  

  // create window
  _mainWindow->SetContext();
  int width, height;
  glfwGetWindowSize(_mainWindow->GetGlfwWindow(), &width, &height);
  View* mainView = _mainWindow->GetMainView();
  _mainWindow->SplitView(mainView, 0.075, true);
  View* bottomView = _mainWindow->SplitView(mainView->GetRight(), 0.9, true);
  View* timelineView = bottomView->GetRight();
  View* centralView = _mainWindow->SplitView(bottomView->GetLeft(), 0.6, true);
  View* middleView = centralView->GetLeft();

  _mainWindow->SplitView(middleView, 0.9, false);
  View* workingView = _mainWindow->SplitView(middleView->GetLeft(), 0.15, false);
  View* explorerView = workingView->GetLeft();
  View* viewportView = workingView->GetRight();  
  View* graphView = centralView->GetRight();
  _mainWindow->Resize(width, height);

  _mainWindow->CollectLeaves();

  //GraphUI* graph = new GraphUI(graphView, "GraphUI");
  _viewport = new ViewportUI(viewportView, EMBREE);
  _viewport->SetContext(_context);
  _timeline = new TimelineUI(timelineView);
  _timeline->Init(this);
  //DummyUI* dummy = new DummyUI(timelineView, "Dummy");
  MenuUI* menu = new MenuUI(mainView->GetLeft());

  pxr::UsdStageRefPtr stage1 = pxr::UsdStage::Open(filename);
  _stages.push_back(stage1);
  //TestStageUI(graph, _stages);

  _viewport->Init(stage1);
 
  _context->Resize(_viewport->GetWidth(), _viewport->GetHeight());
  _context->SetFilePath(filename);
  _context->InitDevice(_viewport->GetCamera());
  _context->TraverseStage();
  _context->CommitDevice();
  
  std::string imageDirectory = "/Users/benmalartre/Documents/RnD/amnesie/images";
  int imageId = NumFilesInDirectory(imageDirectory.c_str()) + 1;
  std::string imagePath = imageDirectory + "/img.";
  std::string imageExt = ".jpg";
  embree::FileName outputImageFilename(imagePath + std::to_string(imageId) + imageExt);

  _viewport->GetCamera()->ComputeFrustum();

  RenderToFile(outputImageFilename, _viewport->GetCamera(), 2048, 1024);
  RenderToMemory(_viewport->GetCamera());
  _viewport->SetContext(_context);

}

void Application::Update()
{
  
}

// time
void Application::PreviousFrame()
{
  float currentTime = _currentTime - _speed;
  if(currentTime < _startTime)
  {
    if(_loop)_currentTime = _endTime;
    else _currentTime = _startTime;
  }
  else _currentTime = currentTime;
  
  _timeline->Update();
}

void Application::NextFrame()
{
 float currentTime = _currentTime + _speed;
  if(currentTime > _endTime)
  {
    if(_loop)_currentTime = _startTime;
    else _currentTime = _endTime;
  }
  else _currentTime = currentTime;
  
  _timeline->Update();
}

void Application::FirstFrame()
{
  _currentTime = _startTime;
  _timeline->Update();
}

void Application::LastFrame()
{
  _currentTime = _endTime;
  _timeline->Update();
}

void Application::StartPlayBack(bool backward)
{
  _stopWatch.Reset();
  _playback = true;
  _stopWatch.Start();
  _playForwardOrBackward = backward;
  PlayBack();
}

void Application::StopPlayBack()
{
  _stopWatch.Stop();
  _playback=false;
}

void Application::PlayBack()
{
  _stopWatch.Stop();
  if(_stopWatch.GetMilliseconds()>1000/_fps)
  {
    if(_playForwardOrBackward)PreviousFrame();
    else NextFrame();
    _stopWatch.Reset();
    _stopWatch.Start();
  }
  
  Update();
}

// main loop
void 
Application::MainLoop()
{
  _mainWindow->MainLoop();
}

AMN_NAMESPACE_CLOSE_SCOPE
