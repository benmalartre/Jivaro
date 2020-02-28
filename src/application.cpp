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

  // recurse prim
  //----------------------------------------------------------------------------
  void RecursePrim(const pxr::UsdPrim& prim)
  {
    pxr::UsdPrimSiblingRange children = prim.GetAllChildren();
    for(auto child : children)
    {
      std::cout << child.GetPrimPath() << std::endl;
      if(child.IsA<pxr::UsdGeomXform>())
      {
        std::cout << "XFORM" << std::endl;
      }
      else if(child.IsA<pxr::UsdGeomMesh>())
      {
        std::cout << "MESH" << std::endl;
        TranslateMesh(g_device, g_scene, pxr::UsdGeomMesh(child), 0);
      }
      RecursePrim(child);
    }
  }

  // traverse all recursive
  //----------------------------------------------------------------------------
  int TraverseAllRecursive(const pxr::UsdStageRefPtr stage)
  {
    RecursePrim(stage->GetPseudoRoot());
    return 0;
  }

  // traverse all prim range
  //----------------------------------------------------------------------------
  int TraverseAllPrimRange(const pxr::UsdStageRefPtr stage)
  {
    pxr::UsdPrimRange range = stage->TraverseAll();
    for(auto prim : range)
    {
        //std::cout << prim.GetPrimPath() << std::endl;
    }
    return 0;
  }

  // traverse stage
  //----------------------------------------------------------------------------
  void TraverseStage()
  {
      
    /*
    std::string filePath = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/Kitchen_set/Kitchen_set.usd";

    pxr::UsdStageRefPtr stage = 
        pxr::UsdStage::Open(filePath, pxr::UsdStage::LoadAll);

    for(int i=0;i<128;++i)
    {
      auto tr = TIMER_DECORATOR(TraverseAllRecursive);
      tr(stage);
      auto tp = TIMER_DECORATOR(TraverseAllPrimRange);
      tp(stage);
    }
    */
    //pxr::UsdPrimRange range = stage.TraverseAll();

  }

  
  // init application
  //----------------------------------------------------------------------------
  void 
  Application::Init()
  {
    View* mainView = _mainWindow->GetMainView();
    _mainWindow->SplitView(mainView, 10, true);
    _mainWindow->SplitView(mainView->GetRight(), 75, true);
    _mainWindow->SplitView(mainView->GetRight()->GetLeft(), 25, false);
    _mainWindow->SplitView(mainView->GetRight()->GetLeft()->GetRight(), 75, false);

    RTCScene scene = DeviceInit((char*)"hello");
    std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usd";
    //std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/kitchen_set/kitchen_set.usd";
    //std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/Kitchen_set/assets/Clock/Clock.usd";
    //std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/UsdSkelExamples/HumanFemale/HumanFemale.usd";
    pxr::UsdStageRefPtr stage = 
        pxr::UsdStage::Open(usdFile, pxr::UsdStage::LoadAll);

    pxr::TfToken upAxis = pxr::UsdGeomGetStageUpAxis (stage);
    if(upAxis == pxr::UsdGeomTokens->y) std::cerr << "### UP AXIS : Y " << std::endl;
    else if(upAxis == pxr::UsdGeomTokens->z) std::cerr << "### UP AXIS : Z " << std::endl;
    else std::cerr << "### UP AXIS : UNDEFINED " << std::endl;

    RecursePrim(stage->GetPseudoRoot());
    CommitScene();
    embree::FileName outputImageFilename("/Users/benmalartre/Documents/RnD/embree/embree-usd/images/img.007.jpg");
    RenderToFile(outputImageFilename);

    _mainWindow->BuildSplittersMap();
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

