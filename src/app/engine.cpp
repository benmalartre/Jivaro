
#include "engine.h"
#include "pxr/imaging/glf/glew.h"
#include "pxr/imaging/glf/drawTarget.h"
#include "pxr/imaging/hd/engine.h"
#include "pxr/imaging/hd/tokens.h"
#include "pxr/imaging/hd/camera.h"
#include "pxr/imaging/hd/rendererPlugin.h"
#include "pxr/imaging/hd/rendererPluginRegistry.h"
#include "pxr/imaging/hd/unitTestDelegate.h"
#include "pxr/imaging/hdx/renderTask.h"
#include "pxr/imaging/cameraUtil/conformWindow.h"


#include <iostream>


AMN_NAMESPACE_OPEN_SCOPE
/*
void
SetCamera(pxr::GfMatrix4d const &modelViewMatrix,
  pxr::GfMatrix4d const &projectionMatrix,
  pxr::GfVec4d const &viewport)
{
  _sceneDelegate->UpdateCamera(
    _cameraId, HdCameraTokens->worldToViewMatrix, VtValue(modelViewMatrix));
  _sceneDelegate->UpdateCamera(
    _cameraId, HdCameraTokens->projectionMatrix, VtValue(projectionMatrix));
  // Baselines for tests were generated without constraining the view
  // frustum based on the viewport aspect ratio.
  _sceneDelegate->UpdateCamera(
    _cameraId, HdCameraTokens->windowPolicy,
    VtValue(CameraUtilDontConform));

  pxr::HdSprim const *cam = _renderIndex->GetSprim(HdPrimTypeTokens->camera,
    _cameraId);
  TF_VERIFY(cam);
  _renderPassState->SetCameraAndViewport(
    dynamic_cast<pxr::HdCamera const *>(cam), viewport);
}
*/
void RunHydra()
{
  // Get the renderer plugin and create a new render delegate and index.
  const pxr::TfToken rendererPluginId("LoFiRendererPlugin");
  //const pxr::TfToken rendererPluginId("HdStormRendererPlugin");
  //const pxr::TfToken rendererPluginId("HdEmbreeRendererPlugin");

  pxr::HdRendererPlugin* rendererPlugin = 
    pxr::HdRendererPluginRegistry::GetInstance().GetRendererPlugin(rendererPluginId);
  pxr::HdRenderDelegate* renderDelegate = rendererPlugin->CreateRenderDelegate();
  pxr::HdRenderIndex* renderIndex = pxr::HdRenderIndex::New(renderDelegate, {});
  // Construct a new scene delegate to populate the render index.
  pxr::HdUnitTestDelegate* sceneDelegate = new pxr::HdUnitTestDelegate(renderIndex,
    pxr::SdfPath::AbsoluteRootPath());
  pxr::SdfPath camera("/camera");
  sceneDelegate->AddCamera(camera);
  
  pxr::GfMatrix4d viewMatrix = pxr::GfMatrix4d().SetIdentity();
  
  viewMatrix *= pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(0.0, 1.0, 0.0), -45.0));
  viewMatrix *= pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(1.0, 0.0, 0.0), 45.0));
  viewMatrix *= pxr::GfMatrix4d().SetTranslate(pxr::GfVec3d(0.0, 0.0, -4.0));

  pxr::GfFrustum frustum;
  frustum.SetPerspective(45, true, 1, 1.0, 10000.0);
  pxr::GfMatrix4d projMatrix = frustum.ComputeProjectionMatrix();

  //SetCamera(viewMatrix, projMatrix, GfVec4d(0, 0, 512, 512));

  sceneDelegate->UpdateCamera(
    camera, pxr::HdCameraTokens->worldToViewMatrix, pxr::VtValue(viewMatrix));
  sceneDelegate->UpdateCamera(
    camera, pxr::HdCameraTokens->projectionMatrix, pxr::VtValue(projMatrix));
  // Baselines for tests were generated without constraining the view
  // frustum based on the viewport aspect ratio.
  sceneDelegate->UpdateCamera(
    camera, pxr::HdCameraTokens->windowPolicy,
    pxr::VtValue(pxr::CameraUtilDontConform));

  pxr::HdSprim const *cam = renderIndex->GetSprim(pxr::HdPrimTypeTokens->camera,
    camera);
  /*
  _renderPassState->SetCameraAndViewport(
    dynamic_cast<pxr::HdCamera const *>(cam), viewport);*/
  //sceneDelegate->UpdateCamera()
  // Create a cube.
  sceneDelegate->AddGridWithFaceColor(pxr::SdfPath("/MyGrid1"), 12,12,pxr::GfMatrix4f(1));


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

  pxr::HdxRenderTaskParams params;
  params.camera = camera;
  params.viewport = pxr::GfVec4f(0, 0, 512, 512);
  params.enableLighting = true;

  sceneDelegate->UpdateTask(renderTask, pxr::HdTokens->params, pxr::VtValue(params));
  sceneDelegate->UpdateTask(renderTask,
    pxr::HdTokens->collection,
    pxr::VtValue(pxr::HdRprimCollection(pxr::HdTokens->geometry,
      pxr::HdReprSelector(pxr::HdReprTokens->smoothHull))));

  // Ask Hydra to execute our render task.
  pxr::HdEngine engine;
  pxr::HdTaskSharedPtrVector tasks = { renderIndex->GetTask(renderTask) };

  glEnable(GL_DEPTH_TEST);
  size_t imageWidth = 512;
  size_t imageHeight = 512;
  std::string imagePath = "E:/Projects/RnD/Amnesie/build/src/Release/";
  pxr::GfVec2i renderResolution(imageWidth, imageHeight);

  pxr::GlfDrawTargetRefPtr drawTarget = pxr::GlfDrawTarget::New(renderResolution);
  drawTarget->Bind();

  drawTarget->AddAttachment("color",
    GL_RGBA, GL_FLOAT, GL_RGBA);
  drawTarget->AddAttachment("depth",
    GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F);

  glViewport(0, 0, imageWidth, imageHeight);

  /*
  const GLfloat CLEAR_DEPTH[1] = { 1.0f };
  const pxr::UsdPrim& pseudoRoot = stage->GetPseudoRoot();

  do {
    glClearBufferfv(GL_COLOR, 0, CLEAR_COLOR.data());
    glClearBufferfv(GL_DEPTH, 0, CLEAR_DEPTH);
    _imagingEngine.Render(pseudoRoot, renderParams);
  } while (!_imagingEngine.IsConverged());
  */

  engine.Execute(renderIndex, &tasks);

  drawTarget->Unbind();
  drawTarget->WriteToFile("color", imagePath + "color.png");
  drawTarget->WriteToFile("depth", imagePath + "depth.png");
  

  // Destroy the data structures
  delete renderIndex;
  delete renderDelegate;
  delete sceneDelegate;
}

AMN_NAMESPACE_CLOSE_SCOPE
