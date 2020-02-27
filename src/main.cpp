
#include "main.h"

namespace embree {

  RTCScene g_scene = NULL;
  Vec3fa* face_colors = NULL; 
  Vec3fa* vertex_colors = NULL;
  RTCDevice g_device = NULL;
  bool g_changed = 0;
  float g_debug = 0.f;

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
        embree::TranslateMesh(g_device, g_scene, pxr::UsdGeomMesh(child), 0);
      }
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

}

// main application entry point
//------------------------------------------------------------------------------
int main(void)
{
  RTCScene scene = embree::device_init((char*)"hello");
  std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usd";

  pxr::UsdStageRefPtr stage = 
      pxr::UsdStage::Open(usdFile, pxr::UsdStage::LoadAll);

  embree::RecursePrim(stage->GetPseudoRoot());
  embree::commit_scene();
  embree::FileName outputImageFilename("/Users/benmalartre/Documents/RnD/embree/embree-usd/images/img.010.jpg");
  embree::renderToFile(outputImageFilename);

  glfwInit();
  AMN::Application app(800,600);
  app.MainLoop();
  //app->cleanUp();  
  glfwTerminate();
  return 1;
}