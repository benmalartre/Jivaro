
#include "../ui/utils.h"
#include "../ui/viewport.h"
#include "../ui/menu.h"
#include "../geometry/shape.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/camera.h"
#include "../app/tools.h"
#include "../command/command.h"
#include "../app/handle.h"
#include "../app/application.h"
#include "../utils/strings.h"
#include "../utils/glutils.h"
#include "../utils/keys.h"

#include <pxr/imaging/cameraUtil/conformWindow.h>

PXR_NAMESPACE_OPEN_SCOPE

extern bool LEGACY_OPENGL;

ImGuiWindowFlags ViewportUI::_flags = 
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoScrollbar |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoDecoration;

// constructor
ViewportUI::ViewportUI(View* parent):
  HeadedUI(parent, "Viewport")
{
  _texture = 0;
  _drawMode = (int)pxr::UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH;
  _pixels = nullptr;
  _camera = new Camera("Camera");
  _valid = true;
  _camera->Set(pxr::GfVec3d(12,24,12),
              pxr::GfVec3d(0,0,0),
              pxr::GfVec3d(0,1,0));
  _interactionMode = INTERACTION_NONE;
  _engine = nullptr;
  _rendererIndex = 0;
  _rendererNames = NULL;
}

// destructor
ViewportUI::~ViewportUI()
{
  if(_rendererNames)delete[] _rendererNames;
  if(_texture) glDeleteTextures(1, &_texture);
  if(_camera) delete _camera;
  if(_engine) delete _engine;
}

void ViewportUI::Init()
{
  Application* app = GetApplication();
  if (_engine) {
    app->RemoveEngine(_engine);
    delete _engine;
  }
  pxr::SdfPathVector excludedPaths;

  _engine = new Engine(pxr::SdfPath("/"), excludedPaths);
  app->AddEngine(_engine);

  pxr::TfTokenVector rendererTokens = _engine->GetRendererPlugins();
  if (_rendererNames) delete[] _rendererNames;
  _numRenderers = rendererTokens.size();
  _rendererNames = new const char* [_numRenderers];
  for (short rendererIndex = 0; rendererIndex < _numRenderers; ++rendererIndex) {
    _rendererNames[rendererIndex] = rendererTokens[rendererIndex].GetText();
    std::cout << rendererTokens[rendererIndex].GetText() << std::endl;
  }
  
  if (LEGACY_OPENGL) {
    _engine->SetRendererPlugin(pxr::TfToken("LoFiRendererPlugin"));
  } else {
    _engine->SetRendererPlugin(pxr::TfToken(_rendererNames[_rendererIndex]));
  }

  pxr::GlfSimpleMaterial material;
  pxr::GlfSimpleLight light;
  light.SetAmbient({ 0.2, 0.2, 0.2, 1.0 });
  light.SetDiffuse({ 1.0, 1.0, 1.0, 1.f });
  light.SetSpecular({ 0.2, 0.2, 0.2, 1.f });
  light.SetPosition({ 200, 200, 200, 1.0 });
  pxr::GlfSimpleLightVector lights;
  lights.push_back(light);

  material.SetAmbient({ 0.0, 0.0, 0.0, 1.f });
  material.SetDiffuse({ 1.0, 1.0, 1.0, 1.f });
  material.SetSpecular({ 0.2, 0.2, 0.2, 1.f });
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
  double x, y;
  Window* window = _parent->GetWindow();
  Tool* tools = GetApplication()->GetTools();
  glfwGetCursorPos(window->GetGlfwWindow(), &x, &y);

  const float width = GetWidth();
  const float height = GetHeight();

  if (action == GLFW_RELEASE)
  {
    _interactionMode = INTERACTION_NONE;
    SetInteracting(false);

    if (!(mods & GLFW_MOD_ALT) && !(mods & GLFW_MOD_SUPER)) {
      if (tools->IsInteracting()) {
        tools->EndUpdate(x - GetX(), y - GetY(), width, height);
      }
      else {
        Pick(x, y, mods);
      }
    }
  }
  else if (action == GLFW_PRESS)
  {
    _lastX = (int)x;
    _lastY = (int)y;
    SetInteracting(true);
    if (mods & GLFW_MOD_ALT) {
      if (button == GLFW_MOUSE_BUTTON_LEFT) {
        _interactionMode = INTERACTION_ORBIT;
      }
      else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        _interactionMode = INTERACTION_WALK;
      }
      else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        _interactionMode = INTERACTION_DOLLY;
      }
    }
    else if (mods & GLFW_MOD_SUPER) {
      if (button == GLFW_MOUSE_BUTTON_LEFT) {
        _interactionMode = INTERACTION_WALK;
      }
      else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        _interactionMode = INTERACTION_ORBIT;
      }
      else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        _interactionMode = INTERACTION_DOLLY;
      }
    }
    else if (tools->IsActive()) {
      
      tools->Select(x - GetX(), y - GetY(), width, height, false);
      tools->BeginUpdate(x - GetX(), y - GetY(), width, height);
    }
    else {

    }
  }

  _parent->SetDirty();
}

void ViewportUI::MouseMove(int x, int y) 
{
  Application* app = GetApplication();
  Tool* tools = app->GetTools();
  tools->SetCamera(_camera);
  
  if(_interacting)
  {
    double dx = static_cast<double>(x) - _lastX;
    double dy = static_cast<double>(y) - _lastY;
    switch(_interactionMode)
    {
      case INTERACTION_WALK:
      {
        _camera->Walk(
          dx / static_cast<double>(GetWidth()), 
          dy / static_cast<double>(GetHeight())
        );
        _parent->SetFlag(View::FORCEREDRAW);
        break;
      }
       
      case INTERACTION_DOLLY:
      {
        _camera->Dolly(
          dx / static_cast<double>(GetWidth()), 
          dy / static_cast<double>(GetHeight())
        );
        _parent->SetFlag(View::FORCEREDRAW);
        break;
      }
        
      case INTERACTION_ORBIT:
      {
        _camera->Orbit(dx, dy);
        _parent->SetFlag(View::FORCEREDRAW);
        break;
      }

      default:
      {
        tools->Update(x - GetX(), y - GetY(), GetWidth(), GetHeight());
        _parent->SetFlag(View::FORCEREDRAW);
        break;
      }
        
    }
    _parent->SetDirty();
  } else {
    tools->Pick(x - GetX(), y - GetY(), GetWidth(), GetHeight());
    _parent->SetFlag(View::FORCEREDRAW);
  }


  _lastX = static_cast<double>(x);
  _lastY = static_cast<double>(y);
}

void ViewportUI::MouseWheel(int x, int y)
{
  Application* app = GetApplication();
  _camera->Dolly(
    static_cast<double>(x) / static_cast<double>(GetWidth()), 
    static_cast<double>(x) / static_cast<double>(GetHeight())
  );
  _parent->SetFlag(View::FORCEREDRAW);
  _parent->SetDirty();
}

void ViewportUI::Keyboard(int key, int scancode, int action, int mods)
{
  Application* app = GetApplication();
  int mappedKey = GetMappedKey(key);
  if (action == GLFW_PRESS) {
    switch (mappedKey) {
      case GLFW_KEY_A:
      {
        _camera->FrameSelection(GetApplication()->GetStageBoundingBox());
        _parent->SetFlag(View::FORCEREDRAW);
        break;
      }
      case GLFW_KEY_F:
      {
        if (app->GetSelection()->IsEmpty())return;
        _camera->FrameSelection(GetApplication()->GetSelectionBoundingBox());
        _parent->SetFlag(View::FORCEREDRAW);
        break;
      }
      case GLFW_KEY_S:
      {
        app->SetActiveTool(TOOL_SCALE);
        break;
      }
      case GLFW_KEY_R:
      {
        app->SetActiveTool(TOOL_ROTATE);
        break;
      }
      case GLFW_KEY_T:
      {
        app->SetActiveTool(TOOL_TRANSLATE);
        break;
      }
    }
  }
}
/*
static void DrawToolCallback(const ImDrawList* parent_list, const ImDrawCmd* cmd) {

  ViewportUI* viewport = (ViewportUI*)cmd->UserCallbackData;
  Camera* camera = viewport->GetCamera();
  View* view = viewport->GetView();

  // get current state
  GLint currentViewport[4];
  glGetIntegerv(GL_VIEWPORT, &currentViewport[0]);

  GLint currentProgram;
  glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
  
  // resize viewport
  float scaleX, scaleY;
  glfwGetWindowContentScale(view->GetWindow()->GetGlfwWindow(), &scaleX, &scaleY);
  float x = viewport->GetX();
  float y = viewport->GetY();
  float w = viewport->GetWidth();
  float h = viewport->GetHeight();
  float wh = viewport->GetWindowHeight();
  glViewport(x * scaleX,(wh - (y + h)) * scaleY, w * scaleX, h * scaleY);
  
  glEnable(GL_DEPTH_TEST);
  glClear(GL_DEPTH_BUFFER_BIT);

  Tool* tools = GetApplication()->GetTools();
  tools->SetViewport(pxr::GfVec4f(x, y, w, h));
  tools->SetCamera(camera);
  tools->Draw(w, h);

  // restore viewport
  glViewport(
    currentViewport[0],
    currentViewport[1],
    currentViewport[2],
    currentViewport[3]
  );
  
  // restore material
  glUseProgram(currentProgram);
  glDisable(GL_DEPTH_TEST);
}
*/

bool ViewportUI::Draw()
{    
  if (!_initialized)Init();
  if(!_valid)return false;  

  float x = GetX();
  float y = GetY();
  
  float w = GetWidth();
  float h = GetHeight();
  float wh = GetWindowHeight();

  //glEnable(GL_SCISSOR_TEST);
  //glScissor(x, wh - (y + h), w, h);
  //glViewport(x,wh - (y + h), w, h);
  //glViewport(0, 0, w, h);

  Application* app = GetApplication();
  if (app->GetStage() != nullptr) {
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    _engine->SetRendererAov(pxr::HdAovTokens->color);
    _engine->SetRenderViewport(
      pxr::GfVec4d(
        0.0,
        0.0,
        static_cast<double>(w),
        static_cast<double>(h)));

    _engine->SetCameraState(
      _camera->GetViewMatrix(),
      _camera->GetProjectionMatrix()
    );
  
    _renderParams.frame = pxr::UsdTimeCode(app->GetTime().GetActiveTime());
    _renderParams.complexity = 1.0f;
    _renderParams.drawMode = (pxr::UsdImagingGLDrawMode)_drawMode;
    _renderParams.showGuides = true;
    _renderParams.showRender = true;
    _renderParams.showProxy = true;
    _renderParams.forceRefresh = false;
    _renderParams.cullStyle = pxr::UsdImagingGLCullStyle::CULL_STYLE_NOTHING;
    _renderParams.gammaCorrectColors = false;
    _renderParams.enableIdRender = false;
    _renderParams.enableSampleAlphaToCoverage = true;
    _renderParams.highlight = true;
    _renderParams.enableSceneMaterials = false;
    //_renderParams.colorCorrectionMode = ???
    _renderParams.clearColor = pxr::GfVec4f(0.5,0.5,0.5,1.0);

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
   

    if ( _parent->GetFlag(View::FORCEREDRAW) || !_engine->IsConverged() ) {
      
      _drawTarget->Bind();
      glViewport(0, 0, w, h);

      // clear to black
      glClearColor(0.25f, 0.25f, 0.25f, 1.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      _engine->Render(app->GetStage()->GetPseudoRoot(), _renderParams);
      
      _parent->ClearFlag(View::FORCEREDRAW);

      Tool* tools = GetApplication()->GetTools();
      tools->SetViewport(pxr::GfVec4f(0, 0, w, h));
      tools->SetCamera(_camera);
      tools->Draw();
      _drawTarget->Unbind();
      _drawTarget->Resolve();
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, wh - (y + h), w, h);

    const pxr::GfVec2f min(GetX(), GetY());
    const pxr::GfVec2f size(GetWidth(), GetHeight());
    const pxr::GfVec2f max(min + size);

    ImGui::Begin(_name.c_str(), NULL, _flags);

    ImGui::SetWindowPos(min);
    ImGui::SetWindowSize(size);
   
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddImage(
      (ImTextureID)(size_t)_drawTarget->GetAttachment("color")->GetGlTextureName(),
      min,
      max,
      ImVec2(0,1),
      ImVec2(1,0),
      ImColor(255,255,255,255));

    _engine->SetSelectionColor(pxr::GfVec4f(1, 0, 0, 0.25));
    glDisable(GL_SCISSOR_TEST);

    // tool drawing
    //drawList->AddCallback(DrawToolCallback, this);
  
    ImGui::PushFont(GetWindow()->GetRegularFont(0));
    std::string msg = "Hello Jivaro!";
    drawList->AddText(
      ImVec2(min[0] + 20, max[1] - 20), 
      0xFFFFFFFF, 
      msg.c_str());

    msg = "FPS : "+ std::to_string(app->GetTime().GetFramerate());
    drawList->AddText(
      ImVec2(max[0] - 128.f, max[1] - 20),
      0xFFFFFFFF,
      msg.c_str());

    // renderer
    ImGui::SetCursorPosX(0);

    int rendererIndex = _rendererIndex;
    ImGui::Text("Renderer");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(300);
    ImGui::Combo("##Renderer", &rendererIndex, _rendererNames, _numRenderers);
    if (rendererIndex != _rendererIndex) {
      _rendererIndex = rendererIndex;
      Init();
    }
    ImGui::SameLine();

    // shaded mode
    ImGui::Text("Mode");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(300);
    ImGui::Combo("##DrawMode", &_drawMode, DRAW_MODE_NAMES, IM_ARRAYSIZE(DRAW_MODE_NAMES));
    ImGui::PopFont();

    ImGui::End();

    return GetView()->IsInteracting();
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
  if(GetWidth() <= 0 || GetHeight() <= 0)_valid = false;
  else _valid = true;
  
  double aspectRatio = (double)GetWidth()/(double)GetHeight();
  _camera->Get()->SetPerspectiveFromAspectRatioAndFieldOfView(
    aspectRatio,
    _camera->GetFov(),
    pxr::GfCamera::FOVHorizontal
  );
  
  pxr::GfVec2i renderResolution(GetWidth(), GetHeight());
  _drawTarget = pxr::GlfDrawTarget::New(renderResolution, true);
  _drawTarget->Bind();

  _drawTarget->AddAttachment("color",
    GL_RGBA, GL_FLOAT, GL_RGBA);
  _drawTarget->AddAttachment("depth",
    GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F);
  _drawTarget->Unbind();
}

static pxr::GfVec4i _ViewportMakeCenteredIntegral(pxr::GfVec4f& viewport)
{
  // The values are initially integraland containing the
  // the given rect
  int left = int(floor(viewport[0]));
  int bottom = int(floor(viewport[1]));
  int right = int(ceil(viewport[0] + viewport[2]));
  int top = int(ceil(viewport[1] + viewport[3]));

  int width = right - left;
  int height = top - bottom;

  // Compare the integral height to the original height
  // and do a centered 1 pixel adjustment if more than
  // a pixel off.
  if ((height - viewport[3]) > 1.0) {
    bottom += 1;
    height -= 2;
  }

  // Compare the integral width to the original width
  // and do a centered 1 pixel adjustment if more than
  // a pixel off.
  if ((width - viewport[2]) > 1.0) {
    left += 1;
    width -= 2;
  }
  return pxr::GfVec4i(left, bottom, width, height);
}

 
pxr::GfFrustum 
ViewportUI::_ComputePickFrustum(int x, int y)
{
  const float targetAspectRatio = float(GetWidth()) / float(pxr::GfMax(1, GetHeight()));
  // normalize position and pick size by the viewport size
  pxr::GfVec2d point(
    (double)(x - GetX()) / (double)GetWidth(),
    (double)(y - GetY()) / (double)GetHeight());

  point[0] = (point[0] * 2.0 - 1.0);
  point[1] = -1.0 * (point[1] * 2.0 - 1.0);

  pxr::GfVec2d size(1.0 / (double)GetWidth(), 1.0 / (double)GetHeight());

  pxr::GfCamera camera = *(_camera->Get());
  pxr::CameraUtilConformWindow(
    &camera,
    pxr::CameraUtilConformWindowPolicy::CameraUtilFit,
    targetAspectRatio
  );

  pxr::GfFrustum cameraFrustum = camera.GetFrustum();
  return cameraFrustum.ComputeNarrowedFrustum(point, size);
}

bool ViewportUI::Pick(int x, int y, int mods)
{
  Application* app = GetApplication();
  pxr::GfFrustum pickFrustum = _ComputePickFrustum(x, y);
  pxr::GfVec3d outHitPoint;
  pxr::GfVec3d outHitNormal;
  pxr::SdfPath outHitPrimPath;
  pxr::SdfPath outHitInstancerPath;
  int outHitInstanceIndex;
  pxr::HdInstancerContext outInstancerContext;

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
      if (mods & GLFW_MOD_CONTROL && mods & GLFW_MOD_SHIFT) {
        app->ToggleSelection({ outHitPrimPath });
      }
      else if (mods & GLFW_MOD_SHIFT) {
        app->AddToSelection({ outHitPrimPath });
      }
      else {
        app->SetSelection({ outHitPrimPath });
      }
    return true;
  } else {
    app->ClearSelection();
    return false;
  }
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
PXR_NAMESPACE_CLOSE_SCOPE