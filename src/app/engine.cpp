
#include "engine.h"
#include "pxr/imaging/hd/engine.h"
#include "pxr/imaging/hd/rendererPlugin.h"
#include "pxr/imaging/hd/rendererPluginRegistry.h"
#include "pxr/imaging/hd/unitTestDelegate.h"
#include "pxr/imaging/hdx/renderTask.h"

#include <iostream>


AMN_NAMESPACE_OPEN_SCOPE

void RunHydra()
{
  // Get the renderer plugin and create a new render delegate and index.
  const pxr::TfToken LoFiRendererPluginId("LoFiRendererPlugin");

  pxr::HdRendererPlugin* rendererPlugin = pxr::HdRendererPluginRegistry::GetInstance()
    .GetRendererPlugin(LoFiRendererPluginId);

  pxr::HdRenderDelegate* renderDelegate = rendererPlugin->CreateRenderDelegate();
  pxr::HdRenderIndex* renderIndex = pxr::HdRenderIndex::New(renderDelegate, {});

  // Construct a new scene delegate to populate the render index.
  pxr::HdUnitTestDelegate* sceneDelegate = new pxr::HdUnitTestDelegate(renderIndex,
    pxr::SdfPath::AbsoluteRootPath());

  // Create a cube.
  sceneDelegate->AddCube(pxr::SdfPath("/MyCube1"), pxr::GfMatrix4f(1));

  // Let's use the HdxRenderTask as an example, and configure it with
  // basic parameters.
  //
  // Another option here could be to create your own task which would
  // look like this :
  //
  // class MyDrawTask final : public HdTask
  // {
  // public:
  //     MyDrawTask(HdRenderPassSharedPtr const &renderPass,
  //                HdRenderPassStateSharedPtr const &renderPassState,
  //                TfTokenVector const &renderTags)
  //     : HdTask(SdfPath::EmptyPath()) { }
  // 
  //     void Sync(HdSceneDelegate* delegate,
  //         HdTaskContext* ctx,
  //         HdDirtyBits* dirtyBits) override { }
  //
  //     void Prepare(HdTaskContext* ctx,
  //         HdRenderIndex* renderIndex) override { }
  //
  //     void Execute(HdTaskContext* ctx) override { }
  // };
  pxr::SdfPath renderTask("/renderTask");
  sceneDelegate->AddTask<pxr::HdxRenderTask>(renderTask);
  sceneDelegate->UpdateTask(renderTask, pxr::HdTokens->params,
    pxr::VtValue(pxr::HdxRenderTaskParams()));
  sceneDelegate->UpdateTask(renderTask,
    pxr::HdTokens->collection,
    pxr::VtValue(pxr::HdRprimCollection(pxr::HdTokens->geometry,
      pxr::HdReprSelector(pxr::HdReprTokens->refined))));

  // Ask Hydra to execute our render task.
  pxr::HdEngine engine;
  pxr::HdTaskSharedPtrVector tasks = { renderIndex->GetTask(renderTask) };
  engine.Execute(renderIndex, &tasks);

  // Destroy the data structures
  delete renderIndex;
  delete renderDelegate;
  delete sceneDelegate;
}

/*
int main(int argc, char *argv[])
{
  TfErrorMark mark;
  RunHydra();

  // If no error messages were logged, return success.
  if (mark.IsClean()) {
    std::cout << "OK" << std::endl;
    return EXIT_SUCCESS;
  }
  else {
    std::cout << "FAILED" << std::endl;
    return EXIT_FAILURE;
  }
}
*/

AMN_NAMESPACE_CLOSE_SCOPE
