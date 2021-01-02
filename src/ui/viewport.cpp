#include "viewport.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/camera.h"
#include "../app/tools.h"
#include "../app/application.h"
#include "../utils/utils.h"
#include "../utils/strings.h"
#include "../utils/glutils.h"

AMN_NAMESPACE_OPEN_SCOPE


// constructor
ViewportUI::ViewportUI(View* parent, VIEWPORT_MODE mode):
BaseUI(parent, "Viewport")
{
  _flags = ImGuiWindowFlags_None
    | ImGuiWindowFlags_NoResize
    | ImGuiWindowFlags_NoTitleBar
    | ImGuiWindowFlags_NoScrollbar
    | ImGuiWindowFlags_NoMove
    | ImGuiWindowFlags_NoDecoration;

  _texture = 0;
  _mode = mode;
  _pixels = nullptr;
  _camera = new Camera("Camera");
  _valid = true;
  _camera->Set(pxr::GfVec3d(12,24,12),
              pxr::GfVec3d(0,0,0),
              pxr::GfVec3d(0,1,0));
  _interact = false;
  _interactionMode = INTERACTION_NONE;
  _engine = nullptr;
  _parent->SetFlag(View::FORCEREDRAW);

  pxr::TfWeakPtr<ViewportUI> me(this);
  pxr::TfNotice::Register(me, &BaseUI::ProcessNewScene);
}

// destructor
ViewportUI::~ViewportUI()
{
  if(_texture) glDeleteTextures(1, &_texture);
  if(_camera) delete _camera;
  if (_engine) delete _engine;
}

void ViewportUI::Init()
{
  std::cout << "INIT VIEWPORT" << std::endl;
  if (_engine)delete _engine;
  pxr::SdfPathVector excludedPaths;
  GLCheckError("INIT VIEWPORT");  

  _engine = new Engine(pxr::SdfPath("/"), excludedPaths);
  switch (_mode) {
  case OPENGL:
  {
    std::cout << "TRY TO SET OPENGL HYDRA BACKEND..." << std::endl;
    bool loaded = _engine->SetRendererPlugin(pxr::TfToken("HdStormRendererPlugin"));
    std::cout << "LOADED ? " << loaded << std::endl;
    break;
  }
  case LOFI:
  {
    _engine->SetRendererPlugin(pxr::TfToken("LoFiRendererPlugin"));
    break;
  }
  case EMBREE:
    _engine->SetRendererPlugin(pxr::TfToken("HdEmbreeRendererPlugin"));
    break;
  }
  
  std::cout << "CURRENT RENDERER : " << _engine->GetCurrentRendererId().GetText() << std::endl;

  pxr::GlfSimpleMaterial material;
  pxr::GlfSimpleLight light;
  light.SetAmbient(pxr::GfVec4f(0.25,0.25,0.25,1));
  light.SetPosition(pxr::GfVec4f(24,32,8,1));
  pxr::GlfSimpleLightVector lights;
  lights.push_back(light);

  material.SetAmbient(pxr::GfVec4f(0.2,0.2,0.2, 1.0));
  auto lightingContext = pxr::GlfSimpleLightingContext::New();

  _engine->SetLightingState(lights,
                            material,
                            pxr::GfVec4f(0.5,0.5,0.5,1.0));

  Resize();

  /*
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
  */
  std::cout << "Hydra Enabled : " << pxr::UsdImagingGLEngine::IsHydraEnabled() << std::endl;
  _initialized = true;
}

void ViewportUI::Update()
{
  _parent->SetDirty();
}

// overrides
void ViewportUI::MouseButton(int button, int action, int mods) 
{
  if (action == GLFW_RELEASE)
  {
    _interactionMode = INTERACTION_NONE;
    _interact = false;
    //RenderToMemory(_camera, false);
    //SetImage();
  }
  else if (action == GLFW_PRESS)
  {
    double x,y;
    Window* window = _parent->GetWindow();
    glfwGetCursorPos(window->GetGlfwWindow(),&x,&y);
    
    _lastX = (int)x;
    _lastY = (int)y;
    if( mods & GLFW_MOD_ALT)
    {
      _interact = true;
      window->SetActiveTool(TOOLS::AMN_TOOL_CAMERA);
      if (button == GLFW_MOUSE_BUTTON_LEFT)
      {
        _interactionMode = INTERACTION_ORBIT;
      }
      else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
      {
        _interactionMode = INTERACTION_WALK;
      }
      else if(button == GLFW_MOUSE_BUTTON_RIGHT)
      {
        _interactionMode = INTERACTION_DOLLY;
      }
    }
    else if(mods & GLFW_MOD_SUPER)
    {
      window->SetActiveTool(TOOLS::AMN_TOOL_CAMERA);
      _interact = true;
      if (button == GLFW_MOUSE_BUTTON_LEFT)
      {
        _interactionMode = INTERACTION_WALK;
      }
      else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
      {
        _interactionMode = INTERACTION_ORBIT;
      }
      else if(button == GLFW_MOUSE_BUTTON_RIGHT)
      {
        _interactionMode = INTERACTION_DOLLY;
      }
    }
    else {
      Pick(x, y);
      window->RestoreLastActiveTool();
    }
/*
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
      _interact = true;
      _interactionMode = INTERACTION_ORBIT;
    }
    else if(button == GLFW_MOUSE_BUTTON_LEFT)
    {
      _interact = true;
      if(mods == GLFW_MOD_ALT)
      {
        _interactionMode = INTERACTION_ORBIT;
      }
      else if(mods == GLFW_MOD_CONTROL)
      {
        _interactionMode = INTERACTION_WALK;
      }
      else if(mods == GLFW_MOD_SHIFT)
      {
        _interactionMode = INTERACTION_DOLLY;
      }
    }
*/
  }
  _parent->SetDirty();
}

void ViewportUI::MouseMove(int x, int y) 
{
  if(_interact)
  {
    double dx = x - _lastX;
    double dy = y - _lastY;
    switch(_interactionMode)
    {
      case INTERACTION_WALK:
      {
        _camera->Walk((double)dx / (double)GetWidth(), (double)dy / (double)GetHeight());
        break;
      }
       
      case INTERACTION_DOLLY:
      {
        _camera->Dolly((double)dx / (double)GetWidth(), (double)dy / (double)GetHeight());
       break;
      }
        
      case INTERACTION_ORBIT:
      {
        _camera->Orbit(dx, dy); break;
      }

      default:
        break;
        
    }
    _lastX = x;
    _lastY = y;
    _parent->SetDirty();
  }
}

void ViewportUI::MouseWheel(int x, int y)
{
  _camera->Dolly((double)x / (double)GetWidth(), (double)x / (double)GetHeight());
  _parent->SetDirty();
}

void ViewportUI::Keyboard(int key, int scancode, int action, int mods)
{
  if (action == GLFW_PRESS) {
    switch (key) {
    case GLFW_KEY_A:
      _camera->FrameSelection(GetApplication()->GetStageBoundingBox());
      break;
    case GLFW_KEY_F:
      _camera->FrameSelection(GetApplication()->GetSelectionBoundingBox());
      break;
    }
  }
}

bool ViewportUI::Draw()
{    
  if (!_initialized)Init();
  if(!_valid)return false;  
  float x = _parent->GetMin()[0];
  float y = _parent->GetMin()[1];
  
  float w = _parent->GetWidth();
  float h = _parent->GetHeight();
  float wh = GetWindowHeight();

  //glEnable(GL_SCISSOR_TEST);
  //glScissor(x, wh - (y + h), w, h);
  //glViewport(x,wh - (y + h), w, h);
  //glViewport(0, 0, w, h);

  Application* app = GetApplication();
  if (app->GetStage() != nullptr) {
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    //_engine->SetRendererAov(pxr::HdAovTokens->color);
    _engine->SetRenderViewport(
      pxr::GfVec4d(
        0.0,
        0.0,
        static_cast<double>(w),
        static_cast<double>(h)));
    /*_engine->SetRenderViewport(
      pxr::GfVec4f(x, y, w , h)
    );*/
    _engine->SetCameraState(
      _camera->GetViewMatrix(),
      _camera->GetProjectionMatrix()
    );
  
    _renderParams.frame = pxr::UsdTimeCode(app->GetTime().GetActiveTime());
    _renderParams.complexity = 1.0f;
    _renderParams.drawMode = pxr::UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH;
    _renderParams.showGuides = true;
    _renderParams.showRender = true;
    _renderParams.showProxy = true;
    _renderParams.forceRefresh = false;
    _renderParams.cullStyle = pxr::UsdImagingGLCullStyle::CULL_STYLE_NOTHING;
    _renderParams.gammaCorrectColors = false;
    _renderParams.enableIdRender = false;
    _renderParams.enableSampleAlphaToCoverage = true;
    _renderParams.highlight = true;
    _renderParams.enableSceneMaterials = true;
    //_renderParams.colorCorrectionMode = ???
    _renderParams.clearColor = pxr::GfVec4f(0.0,0.0,0.0,1.0);


 /*
  const std::vector<pxr::GfVec4f> clipPlanes = _camera->GetClippingPlanes();
  std::cout << "CLIP PLANES : " << clipPlanes.size() << std::endl;
  _renderParams.clipPlanes = {
    pxr::GfVec4d(clipPlanes[0]),
    pxr::GfVec4d(clipPlanes[1]),
    pxr::GfVec4d(clipPlanes[2]),
    pxr::GfVec4d(clipPlanes[3])
  };
  */
  //_engine->PrepareBatch(_stages[0]->GetPseudoRoot());
  //_engine->Render(app->GetStages()[0]->GetPseudoRoot(), _renderParams);
    _drawTarget->Bind();
    glViewport(0, 0, w, h);
    // clear to black
    glClearColor(0.25f, 0.25f, 0.25f, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _engine->Render(app->GetStage()->GetPseudoRoot(), _renderParams);
    _drawTarget->Unbind();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, wh - (y + h), w, h);
    //glViewport(x,wh - (y + h), w, h);
    //glViewport(0, 0, w, h);
    ImGui::Begin(_name.c_str(), NULL, _flags);

    ImGui::SetWindowPos(_parent->GetMin());
    ImGui::SetWindowSize(_parent->GetSize());
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddImage(
      (ImTextureID)(size_t)_drawTarget->GetAttachment("color")->GetGlTextureName(),
      _parent->GetMin(),
      _parent->GetMax(),
      ImVec2(0,1),
      ImVec2(1,0),
      ImColor(255,255,255,255));

    //_engine->SetSelectionColor(pxr::GfVec4f(1, 0, 0, 1));
    glDisable(GL_SCISSOR_TEST);
    /*
    ImGui::PushFont(GetWindow()->GetMediumFont(0));
    std::string msg = "Hello Amnesie!";
    drawList->AddText(
      _parent->GetMin() + ImVec2(20, 20), 
      0xFFFFFFFF, 
      msg.c_str());
    ImGui::PopFont();
    */
    ImGui::End();

    /*
    bool open;
    ImGui::Begin("ViewportOverlay", &open, ImGuiWindowFlags_NoDecoration);
    */
    //ImGui::Text("FPS : %d", this->GetApplication()->GetFramerate());
    //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    //ImGui::End();
    

    return true;
  }
  /*
  if(_pixels)
  {
    glUseProgram(GetScreenSpaceQuadShaderProgram());
    if(_interact)
      CreateOpenGLTexture(_width>>4, _height>>4, _lowPixels, _texture, 0);
    else
      CreateOpenGLTexture(_width, _height, _pixels, _texture, 0);
    glViewport(x, GetWindowHeight()-(y+h), w, h);
    glUniform1i(glGetUniformLocation(GetScreenSpaceQuadShaderProgram(),"tex"),0);
    DrawScreenSpaceQuad();
  }
  else
  {
    float wh = GetWindowHeight();
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, wh - (y + h), w, h);
    glClearColor(RANDOM_0_1,RANDOM_0_1,RANDOM_0_1,1.f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
  }
  */
  return false;
}

// Conform the camera viewport to the camera's aspect ratio,
// and center the camera viewport in the window viewport.
pxr::GfVec4f ViewportUI::ComputeCameraViewport(float cameraAspectRatio)
{
     /*   
  windowPolicy = CameraUtil.MatchVertically
  targetAspect = (
    float(self.size().width()) / max(1.0, self.size().height()))
  if targetAspect < cameraAspectRatio:
      windowPolicy = CameraUtil.MatchHorizontally

  viewport = Gf.Range2d(Gf.Vec2d(0, 0),
                        Gf.Vec2d(self.computeWindowSize()))
  viewport = CameraUtil.ConformedWindow(viewport, windowPolicy, cameraAspectRatio)

  viewport = (viewport.GetMin()[0], viewport.GetMin()[1],
              viewport.GetSize()[0], viewport.GetSize()[1])
  viewport = ViewportMakeCenteredIntegral(viewport)

  return viewport
  */
 return pxr::GfVec4f();
}

void ViewportUI::Resize()
{
  if(_parent->GetWidth() <= 0 || _parent->GetHeight() <= 0)_valid = false;
  else _valid = true;
  double aspectRatio = (double)_parent->GetWidth()/(double)_parent->GetHeight();
  _camera->Get()->SetPerspectiveFromAspectRatioAndFieldOfView(
    aspectRatio,
    _camera->GetFov(),
    pxr::GfCamera::FOVHorizontal
  );

  pxr::GfVec2i renderResolution(GetWidth(), GetHeight());
  _drawTarget = pxr::GlfDrawTarget::New(renderResolution);
  _drawTarget->Bind();

  _drawTarget->AddAttachment("color",
    GL_RGBA, GL_FLOAT, GL_RGBA);
  _drawTarget->AddAttachment("depth",
    GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F);
  _drawTarget->Unbind();
}


pxr::GfFrustum 
ViewportUI::_ComputePickFrustum(int x, int y)
{
  // normalize position and pick size by the viewport size
  pxr::GfVec2d point(
    (double)(x - GetX()) / (double)GetWidth(),
    (double)(y - GetY()) / (double)GetHeight());

  point[0] = (point[0] * 2.0 - 1.0);
  point[1] = -1.0 * (point[1] * 2.0 - 1.0);

  pxr::GfVec2d size(1.0 / (double)GetWidth(), 1.0 / (double)GetHeight());

  // "point" is normalized to the image viewport size, but if the image
  // is cropped to the camera viewport, the image viewport won't fill the
  // whole window viewport.Clicking outside the image will produce
  // normalized coordinates > 1 or < -1; in this case, we should skip
  // picking.
  //inImageBounds = (abs(point[0]) <= 1.0 and abs(point[1]) <= 1.0)

  pxr::GfFrustum cameraFrustum = _camera->_GetFrustum();
  return cameraFrustum.ComputeNarrowedFrustum(point, size);

  /*
  # compute pick frustum
  (gfCamera, cameraAspect) = self.resolveCamera()
    cameraFrustum = gfCamera.frustum

    viewport = self.computeWindowViewport()
    if self._cropImageToCameraViewport:
  viewport = self.computeCameraViewport(cameraAspect)

    # normalize position and pick size by the viewport size
    point = Gf.Vec2d((x - viewport[0]) / float(viewport[2]),
    (y - viewport[1]) / float(viewport[3]))
    point[0] = (point[0] * 2.0 - 1.0)
    point[1] = -1.0 * (point[1] * 2.0 - 1.0)

    size = Gf.Vec2d(1.0 / viewport[2], 1.0 / viewport[3])

# "point" is normalized to the image viewport size, but if the image
    # is cropped to the camera viewport, the image viewport won't fill the
    # whole window viewport.Clicking outside the image will produce
    # normalized coordinates > 1 or < -1; in this case, we should skip
    # picking.
    inImageBounds = (abs(point[0]) <= 1.0 and abs(point[1]) <= 1.0)

    return (inImageBounds, cameraFrustum.ComputeNarrowedFrustum(point, size))
    */
}

bool ViewportUI::Pick(int x, int y)
{
  pxr::GfFrustum pickFrustum = _ComputePickFrustum(x, y);
  pxr::GfVec3d outHitPoint;
  pxr::GfVec3d outHitNormal;
  pxr::SdfPath outHitPrimPath;
  pxr::SdfPath outHitInstancerPath;
  int outHitInstanceIndex;
  pxr::HdInstancerContext outInstancerContext;
  _engine->ClearSelected();

  if (_engine->TestIntersection(
    pickFrustum.ComputeViewMatrix(),
    pickFrustum.ComputeProjectionMatrix(),
    GetApplication()->GetStage()->GetPseudoRoot(),
    _renderParams,
    &outHitPoint,
    &outHitNormal,
    &outHitPrimPath,
    &outHitInstancerPath,
    &outHitInstanceIndex,
    &outInstancerContext)) {
      _engine->AddSelected(outHitPrimPath, -1);
      GetApplication()->AddToSelection(outHitPrimPath);
    return true;
  }
  return false;
}

/*
pxr::HdSelectionSharedPtr
ViewportUI::_Pick(pxr::GfVec2i const& startPos, pxr::GfVec2i const& endPos,
  pxr::TfToken const& pickTarget)
{
  pxr::HdxPickHitVector allHits;
  pxr::HdxPickTaskContextParams p;
  p.resolution = pxr::HdxUnitTestUtils::CalculatePickResolution(
    startPos, endPos, pxr::GfVec2i(4, 4));
  p.pickTarget = pickTarget;
  p.resolveMode = pxr::HdxPickTokens->resolveUnique;
  p.viewMatrix = _camera->GetViewMatrix();
  p.projectionMatrix = pxr::HdxUnitTestUtils::ComputePickingProjectionMatrix(
    startPos, endPos, pxr::GfVec2i(GetWidth(), GetHeight()), _camera->_GetFrustum());
  p.collection = _pickablesCol;
  p.outHits = &allHits;

  pxr::HdTaskSharedPtrVector tasks;
  tasks.push_back(_renderIndex->GetTask(pxr::SdfPath("/pickTask")));
  pxr::VtValue pickParams(p);
  _engine->SetTaskContextData(HdxPickTokens->pickParams, pickParams);
  _engine->Execute(_renderIndex.get(), &tasks);

  return pxr::HdxUnitTestUtils::TranslateHitsToSelection(
    p.pickTarget, pxr::HdSelection::HighlightModeSelect, allHits);
}
*/
AMN_NAMESPACE_CLOSE_SCOPE