#include "../utils/nfd.hpp"
#include "../utils/files.h"
#include "../widgets/viewport.h"
#include "../widgets/menu.h"
#include "../widgets/graph.h"
#include "../widgets/timeline.h"
#include "../widgets/dummy.h"
#include "../widgets/toolbar.h"
#include "../widgets/explorer.h"
#include "../widgets/property.h"

#include <pxr/base/tf/debug.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/imaging/hd/renderPassState.h>
#include <pxr/imaging/LoFi/debugCodes.h>
#include <pxr/usd/usdAnimX/fileFormat.h>
#include <pxr/usd/usdAnimX/fCurve.h>
#include <pxr/usd/usdAnimX/fCurveOp.h>
#include "../tests/stageGraph.h"
#include "../tests/stageUI.h"

#include "application.h"
#include "notice.h"
#include "engine.h"

#include <iostream>

AMN_NAMESPACE_OPEN_SCOPE

Application* AMN_APPLICATION = nullptr;
const char* Application::APPLICATION_NAME = "Amnesie";

// constructor
//----------------------------------------------------------------------------
Application::Application(unsigned width, unsigned height):
  _mainWindow(NULL),_minTime(1),_maxTime(101),_startTime(1),
  _endTime(101),_activeTime(1),_speed(1),_fps(24),_loop(false),_playback(false),
  _framerate(0),_frameCount(0),_lastT(0),_stage(nullptr)
{  
  _mainWindow = CreateStandardWindow(width, height);
  _mainWindow->Init(this);
};

Application::Application(bool fullscreen):
  _mainWindow(NULL),_minTime(1),_maxTime(101),_startTime(1),
  _endTime(101),_activeTime(1),_speed(1),_fps(24),_loop(false),_playback(false),
  _framerate(0), _frameCount(0), _lastT(0),_stage(nullptr)
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

static void
TestAnimX()
{
  pxr::SdfFileFormatConstPtr fileFormat = 
      pxr::SdfFileFormat::FindByExtension("animx");
  pxr::SdfLayerRefPtr animLayer = 
      pxr::SdfLayer::CreateNew("E:/Projects/RnD/USD_BUILD/assets/animX/test.animx");
  std::cout << "ANIM LAYER : " << animLayer << std::endl;
  pxr::UsdStageRefPtr animStage = pxr::UsdStage::Open(animLayer);
  
  animStage->OverridePrim(pxr::SdfPath("/test"));
  pxr::UsdAnimXFCurveOp op = 
      pxr::UsdAnimXFCurveOp::Define(animStage, pxr::SdfPath("/test/translateOp"));

  pxr::UsdAttribute attr = op.CreateAttributeNameAttr();
  attr.Set(pxr::TfToken("xformOp:translate"));
  attr = op.CreateElementCountAttr();
  attr.Set(3);
  attr = op.CreateDataTypeAttr();
  attr.Set(pxr::TfToken("double"));

  pxr::UsdAnimXFCurve x = 
      pxr::UsdAnimXFCurve::Define(animStage, pxr::SdfPath("/test/translateOp/x"));
  x.CreateKeyframesAttr();
  for (size_t i = 0; i < 12; ++i) {
    double value = (double)rand() / (double)RAND_MAX;
    std::cout << "ANIM X CURVE KEY : " << i * 4 << ":" << value << std::endl;
    x.SetKeyframe((double)(i * 4), value);
  }

  pxr::UsdAnimXFCurve y =
    pxr::UsdAnimXFCurve::Define(animStage, pxr::SdfPath("/test/translateOp/y"));
  y.CreateKeyframesAttr();
  for (size_t i = 0; i < 12; ++i) {
    double value = (double)rand() / (double)RAND_MAX;
    std::cout << "ANIM Y CURVE KEY : " << i * 4 << ":" << value << std::endl;
    y.SetKeyframe((double)(i * 4), value);
  }

  pxr::UsdAnimXFCurve z =
    pxr::UsdAnimXFCurve::Define(animStage, pxr::SdfPath("/test/translateOp/z"));
  z.CreateKeyframesAttr();
  for (size_t i = 0; i < 12; ++i) {
    double value = (double)rand() / (double)RAND_MAX;
    std::cout << "ANIM Z CURVE KEY : " << i * 4 << ":" << value << std::endl;
    z.SetKeyframe((double)(i * 4), value);
  }

  animLayer->Save();
  std::cout << "TEXT ANIM X SAVED!!!" << std::endl;


}

// init application
//----------------------------------------------------------------------------
void 
Application::Init()
{
  //pxr::TfErrorMark mark;
  //RunHydra();
  
  TestAnimX();
  // If no error messages were logged, return success.
  
  /*
  if (mark.IsClean()) {
    std::cout << "HYDRA SCENE DELEGATE OK" << std::endl;
  }
  else {
    for (auto& error : mark)std::cout << error.GetErrorCodeAsString() << std::endl;
    std::cout << "HYDRA SCENE DELEGATE FAILED" << std::endl;
  }
  */
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
  //pxr::TfDebug::Enable(pxr::HD_DIRTY_LIST);
  //pxr::TfDebug::Enable(pxr::HD_COLLECTION_CHANGED);
  //pxr::TfDebug::Enable(pxr::LOFI_REGISTRY);

  // create window
  _mainWindow->SetGLContext();
  int width, height;
  glfwGetWindowSize(_mainWindow->GetGlfwWindow(), &width, &height);
  View* mainView = _mainWindow->SplitView(_mainWindow->GetMainView(), 0.5, true, View::LFIXED, 65);
  View* bottomView = _mainWindow->SplitView(mainView->GetRight(), 0.9, true, false);
  
  //bottomView->Split(0.9, true, true);
  View* timelineView = bottomView->GetRight();
  View* centralView = _mainWindow->SplitView(bottomView->GetLeft(), 0.6, true);
  View* middleView = centralView->GetLeft();
  View* topView = _mainWindow->SplitView(mainView->GetLeft(), 0.5, true, View::LFIXED, 24);

  _mainWindow->SplitView(middleView, 0.9, false);
  
  View* workingView = _mainWindow->SplitView(middleView->GetLeft(), 0.15, false);
  View* propertyView = middleView->GetRight();
  View* explorerView = workingView->GetLeft();
  View* viewportView = workingView->GetRight();  
  View* graphView = centralView->GetRight();
  _mainWindow->Resize(width, height);

  std::cout << "SPLITTED FUCKIN VIEWS... " << std::endl;

  //GraphUI* graph = new GraphUI(graphView, "Graph", true);
  
  _viewport = new ViewportUI(viewportView, OPENGL);

  std::cout << "CREATE FUCKIN VIEWPORT !!!" << std::endl;
  
  _timeline = new TimelineUI(timelineView);

  MenuUI* menu = new MenuUI(topView->GetLeft());
  ToolbarUI* toolbar = new ToolbarUI(topView->GetRight(), "Toolbar");
  _explorer = new ExplorerUI(explorerView);

  _property = new PropertyUI(propertyView, "Property");

  _stage = pxr::UsdStage::Open(filename);
  //_stages.push_back(stage1);
  //TestStageUI(graph, _stages);
 
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
  float currentTime = _activeTime - _speed;
  if(currentTime < _startTime)
  {
    if(_loop)_activeTime = _endTime;
    else _activeTime = _startTime;
  }
  else _activeTime = currentTime;
  
  _timeline->Update();
  _viewport->Update();
}

void Application::NextFrame()
{
 float currentTime = _activeTime + _speed;
  if(currentTime > _endTime)
  {
    if(_loop)_activeTime = _startTime;
    else _activeTime = _endTime;
  }
  else _activeTime = currentTime;
  
  _timeline->Update();
  _viewport->Update();
}

void Application::FirstFrame()
{
  _activeTime = _startTime;
  _timeline->Update();
  _viewport->Update();
}

void Application::LastFrame()
{
  _activeTime = _endTime;
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

void Application::OpenScene(const std::string& filename)
{
  NFD::Guard nfdGuard;
  NFD::UniquePath outPath;
  nfdfilteritem_t filterItem[2] = { { "Usd File", "usd,usdc,usda,usdz" } };
  nfdresult_t result = NFD::OpenDialog(outPath, filterItem, 2);
  if (result == NFD_OKAY)
  {
    _stage = pxr::UsdStage::Open(outPath.get());
    OnNewScene();
    std::cout << "SET PROPERTY PRIM : " << _stage->GetDefaultPrim().GetName() << std::endl;
    _property->SetPrim(_stage->GetDefaultPrim());
  }
}

// main loop
void 
Application::MainLoop()
{
  _mainWindow->MainLoop();
}

AMN_NAMESPACE_CLOSE_SCOPE
