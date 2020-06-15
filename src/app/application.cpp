#include "application.h"
#include "../utils/files.h"
#include "../utils/timer.h"
#include "../widgets/viewport.h"
#include "../widgets/menu.h"
#include "../widgets/graph.h"
#include "../widgets/timeline.h"
#include "../widgets/dummy.h"
#include "../widgets/toolbar.h"
#include "../widgets/explorer.h"
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
  _mainWindow(NULL),_minTime(1),_maxTime(101),_startTime(1),
  _endTime(101),_currentTime(1),_speed(1),_fps(24),_loop(false),_playback(false),
  _framerate(0),_frameCount(0),_lastT(0)
{  
  _mainWindow = CreateStandardWindow(width, height);
  _mainWindow->Init(this);
};

Application::Application(bool fullscreen):
  _mainWindow(NULL),_minTime(1),_maxTime(101),_startTime(1),
  _endTime(101),_currentTime(1),_speed(1),_fps(24),_loop(false),_playback(false),
  _framerate(0), _frameCount(0), _lastT(0)
{
  _mainWindow = CreateFullScreenWindow();
  _mainWindow->Init(this);
};

// destructor
//----------------------------------------------------------------------------
Application::~Application()
{
  if(_mainWindow) delete _mainWindow;
};

// create full screen window
//----------------------------------------------------------------------------
Window*
Application::CreateFullScreenWindow()
{
  return Window::CreateFullScreenWindow();
}

// create child window
//----------------------------------------------------------------------------
Window*
Application::CreateChildWindow(int width, int height, Window* parent)
{
  return Window::CreateChildWindow(width, height, parent->GetGlfwWindow());
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
  std::string filename =
    //"E:/Projects/RnD/USD_BUILD/assets/Contour/JackTurbulized.usda";
    //"E:/Projects/RnD/USD/extras/usd/examples/usdGeomExamples/basisCurves.usda";
    "E:/Projects/RnD/USD_BUILD/assets/maneki_anim.usd";
    //"/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usda";
    //"/Users/benmalartre/Documents/RnD/USD_BUILD/assets/UsdSkelExamples/HumanFemale/HumanFemal.usda";
    //"/Users/benmalartre/Documents/RnD/USD_BUILD/assets/Kitchen_set/Kitchen_set.usd";
    //"/Users/benmalartre/Documents/RnD/amnesie/usd/result.usda";

  // build test scene
  //pxr::TestScene(filename);
  // create imaging gl engine
  //TfDebug::Enable(HD_MDI);
  //TfDebug::Enable(HD_ENGINE_PHASE_INFO);
  //TfDebug::Enable(GLF_DEBUG_CONTEXT_CAPS);
  //TfDebug::Enable(HDST_DUMP_SHADER_SOURCEFILE);

  // create window
  _mainWindow->SetGLContext();
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
  
  GraphUI* graph = new GraphUI(graphView, "GraphUI", true);
  _viewport = new ViewportUI(viewportView, OPENGL);
  _timeline = new TimelineUI(timelineView);

  MenuUI* menu = new MenuUI(mainView->GetLeft());
  ToolbarUI* toolbar = new ToolbarUI(middleView->GetRight(), "Toolbar");
  _explorer = new ExplorerUI(explorerView);

  pxr::UsdStageRefPtr stage1 = pxr::UsdStage::Open(filename);
  _stage = stage1;
  _stages.push_back(stage1);
  TestStageUI(graph, _stages);

  _viewport->Init();
  _timeline->Init(this);
  _explorer->Init();
 
  _mainWindow->CollectLeaves();
  
 /*
  Window* childWindow = CreateChildWindow(400, 400, _mainWindow);
  childWindow->Init(this);
  
  _mainWindow->AddChild(childWindow);
  
  ViewportUI* viewport2 = new ViewportUI(childWindow->GetMainView(), LOFI);
  viewport2->Init();
  
  DummyUI* dummy = new DummyUI(childWindow->GetMainView(), "Dummy");
  
  childWindow->CollectLeaves();
  */
}

void Application::Update()
{
  
}

void Application::ComputeFramerate(double T)
{
  _frameCount++;

  if (T - _lastT >= 1.0)
  {
    _framerate = _frameCount;

    _frameCount = 0;
    _lastT = T;
  }
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
  _viewport->Update();
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
  _viewport->Update();
}

void Application::FirstFrame()
{
  _currentTime = _startTime;
  _timeline->Update();
  _viewport->Update();
}

void Application::LastFrame()
{
  _currentTime = _endTime;
  _timeline->Update();
  _viewport->Update();
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
