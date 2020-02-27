
#include "default.h"
#include "application.h"
#include "device.h"
#include "mesh.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/points.h>

void RecursePrim(const pxr::UsdPrim& prim)
{
    std::cout << "RECURSE PRIM "<< std::endl;
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
          std::cout << "MESH" << g_context._device << "," <<g_context._scene << std::endl;
          embree::TranslateMesh(g_context._device, g_context._scene, pxr::UsdGeomMesh(child), 0);
        }
        RecursePrim(child);
    }
}

int TraverseAllRecursive( const pxr::UsdStageRefPtr stage)
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
  UsdEmbreeContext* ctxt = new UsdEmbreeContext();
  ctxt.Initialize();

  //embree::device_init(ctxt, (char*)"hello");
  embree::device_init((char*)"hello");
  std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usda";

  pxr::UsdStageRefPtr stage = 
      pxr::UsdStage::Open(usdFile, pxr::UsdStage::LoadAll);

  RecursePrim(stage->GetPseudoRoot());
  embree::device_term();
  
  embree::FileName outputImageFilename("/Users/benmalartre/Documents/RnD/embree/embree-usd/images/img.005.jpg");
  //embree::renderToFile(ctxt, outputImageFilename);
  embree::renderToFile(outputImageFilename);

  glfwInit();
  AMN::Application app(800,600);
  app.MainLoop();
  //app->cleanUp();
   
  glfwTerminate();
  return 1;
}