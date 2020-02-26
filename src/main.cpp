
#include "default.h"
#include "application.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>

void RecursePrim(const pxr::UsdPrim& prim)
{
    pxr::UsdPrimSiblingRange children = prim.GetAllChildren();
    for(auto child : children)
    {
        //std::cout << child.GetPrimPath() << std::endl;
        RecursePrim(child);
    }
}

int TraverseAllRecursive(const pxr::UsdStageRefPtr stage)
{
    RecursePrim(stage->GetPseudoRoot());
    return 0;
}

int TraverseAllPrimRange(const pxr::UsdStageRefPtr stage)
{
    pxr::UsdPrimRange range = stage->TraverseAll();
    for(auto prim : range)
    {
        //std::cout << prim.GetPrimPath() << std::endl;
    }
    return 0;
}

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

// main application entry point
//------------------------------------------------------------------------------
int main(void)
{
  glfwInit();
  AMN::Application app(800,600);
  app.MainLoop();
  //app->cleanUp();
 
    glfwTerminate();
  
  glfwTerminate();
  return 1;
}