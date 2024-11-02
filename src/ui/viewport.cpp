#include <pxr/imaging/hd/light.h>
#include <pxr/imaging/hdx/shadowTask.h>
#include <pxr/imaging/hdx/pickTask.h>
#include <pxr/imaging/cameraUtil/conformWindow.h>
#include <pxr/usdImaging/usdImaging/sceneIndices.h>
#include <pxr/usdImaging/usdImaging/stageSceneIndex.h>


#include "../utils/strings.h"
#include "../utils/glutils.h"
#include "../utils/keys.h"
#include "../ui/utils.h"
#include "../ui/fonts.h"
#include "../ui/viewport.h"
#include "../ui/menu.h"
#include "../geometry/shape.h"
#include "../command/command.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/camera.h"
#include "../app/tools.h"
#include "../app/handle.h"
#include "../app/application.h"

JVR_NAMESPACE_OPEN_SCOPE

extern bool LEGACY_OPENGL;

ImGuiWindowFlags ViewportUI::_flags =
ImGuiWindowFlags_None |
ImGuiWindowFlags_NoMove |
ImGuiWindowFlags_NoResize |
ImGuiWindowFlags_NoTitleBar |
ImGuiWindowFlags_NoCollapse |
ImGuiWindowFlags_NoNav |
ImGuiWindowFlags_NoScrollWithMouse |
ImGuiWindowFlags_NoScrollbar;/* |
  ImGuiWindowFlags_NoBackground;*/


static void _BlitFramebufferFromTarget(GlfDrawTargetRefPtr target, 
  int x, int y, int width, int height)
{
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, target->GetFramebufferId());

  glBlitFramebuffer(0, 0, target->GetSize()[0], target->GetSize()[1],
    x, y, width, height,
    GL_COLOR_BUFFER_BIT,
    GL_NEAREST);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

// constructor
ViewportUI::ViewportUI(View* parent)
  : BaseUI(parent, UIType::VIEWPORT)
  , _texture(0)
  , _drawMode(0)
  , _pickMode(Selection::OBJECT)
  , _camera(new Camera("Camera"))
  , _valid(true)
  , _interactionMode(INTERACTION_NONE)
  , _engine(nullptr)
  , _rendererIndex(0)
  , _rendererNames(NULL)
  , _highlightSelection(true)
  , _lightingContext(nullptr)
  , _aov(HdAovTokens->color)
{
  _camera->Set(GfVec3d(12,24,12),
              GfVec3d(0,0,0),
              GfVec3d(0,1,0));
  
  const GfVec2i resolution(GetWindow()->GetResolution());
  _BuildDrawTargets(resolution);

}

// destructor
ViewportUI::~ViewportUI()
{
  Application* app = Application::Get();
  if(_rendererNames)delete[] _rendererNames;
  if(_texture) glDeleteTextures(1, &_texture);
  if(_camera) delete _camera;
  if (_engine) {
    app->GetModel()->RemoveEngine(_engine);
    delete _engine;
  }
}

void ViewportUI::_BuildDrawTargets(const pxr::GfVec2i &resolution)
{
  _drawTarget = GlfDrawTarget::New(resolution, false);
  _drawTarget->Bind();
  _drawTarget->AddAttachment("color", GL_RGBA, GL_FLOAT, GL_RGBA);
  _drawTarget->AddAttachment("depth", GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F);
  auto color = _drawTarget->GetAttachment("color");
  _drawTexId = color->GetGlTextureName();
  _drawTarget->Unbind();

  _toolTarget = GlfDrawTarget::New(resolution, false);
  _toolTarget->Bind();
  _toolTarget->AddAttachment("color", GL_RGBA, GL_FLOAT, GL_RGBA);
  _toolTarget->AddAttachment("depth", GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F);
   color = _toolTarget->GetAttachment("color");
  _toolTexId = color->GetGlTextureName();
  _toolTarget->Unbind();
}

void ViewportUI::UpdateLighting()
{
  /*
  if(!_lightingContext) 
    _lightingContext = GlfSimpleLightingContext::New();

  GlfSimpleLight light;
  light.SetHasShadow(true);
  light.SetDiffuse(GfVec4f(1,1,1,1));
  light.SetAmbient(GfVec4f(0,0,0,1));
  light.SetSpecular(GfVec4f(1,1,1,1));

  const float t = Time::Get()->GetActiveTime();
  light.SetPosition(GfVec4f(pxr::GfSin(t), 10.f, pxr::GfCos(t), 1));

  GlfSimpleMaterial material;
  material.SetAmbient(GfVec4f(0.2, 0.2, 0.2, 1.0));
  material.SetDiffuse(GfVec4f(0.8, 0.8, 0.8, 1.0));
  material.SetSpecular(GfVec4f(0,0,0,1));
  material.SetShininess(0.0001f);

  _lightingContext->SetLights({ light });
  _lightingContext->SetMaterial(material);
  _lightingContext->SetSceneAmbient(GfVec4f(0.2,0.2,0.2,1.0));
  _engine->SetLightingState(_lightingContext);
  */
  //_engine->ActivateShadows(true);
  
}

void ViewportUI::Init()
{
  Application* app = Application::Get();
  if (_engine) {
    app->GetModel()->RemoveEngine(_engine);
    delete _engine;
    _engine = nullptr;
  }

  TfTokenVector rendererTokens = _engine->GetRendererPlugins();
  if (_rendererNames) delete[] _rendererNames;
  _numRenderers = rendererTokens.size();
  _rendererNames = new const char* [_numRenderers];
  for (short rendererIndex = 0; rendererIndex < _numRenderers; ++rendererIndex) {
    _rendererNames[rendererIndex] = rendererTokens[rendererIndex].GetText();
  }

  auto editableSceneIndex = app->GetModel()->GetEditableSceneIndex();

  //TfToken plugin = Engine::GetDefaultRendererPlugin();
  TfToken plugin = TfToken(_rendererNames[_rendererIndex]);
  _engine = new Engine(app->GetModel()->GetFinalSceneIndex(), plugin);

  app->GetModel()->AddEngine(_engine);

  //_engine->SetRendererPlugin(TfToken(_rendererNames[_rendererIndex]));


  UpdateLighting();

  Resize();


  /*
  glEnable(GL_DEPTH_TEST);
  size_t imageWidth = 512;
  size_t imageHeight = 512;
  std::string imagePath = "E:/Projects/RnD/Amnesie/build/src/Release/";
  GfVec2i renderResolution(imageWidth, imageHeight);

  GlfDrawTargetRefPtr drawTarget = GlfDrawTarget::New(renderResolution);
  drawTarget->Bind();

  drawTarget->AddAttachment("color",
    GL_RGBA, GL_FLOAT, GL_RGBA);
  drawTarget->AddAttachment("depth",
    GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F);

  glViewport(0, 0, imageWidth, imageHeight);
  */
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
  Tool* tool = window->GetTool();


  glfwGetCursorPos(window->GetGlfwWindow(), &x, &y);

  const float width = GetWidth();
  const float height = GetHeight();

  if (action == GLFW_RELEASE)
  {
    _interactionMode = INTERACTION_NONE;
    SetInteracting(false);

    if (!(mods & GLFW_MOD_ALT) && !(mods & GLFW_MOD_SUPER)) {
      if (tool->IsInteracting()) {
        tool->EndUpdate(x - GetX(), y - GetY(), width, height);
      }
      else {
        Select(x, y, mods);
      }
    }
  }
  else if (action == GLFW_PRESS)
  {
    _lastX = (int)x;
    _lastY = (int)y;
    Application::Get()->GetModel()->SetActiveEngine(_engine);
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
    else if (tool->IsActive() ) {
      
      tool->Select(x - GetX(), y - GetY(), width, height, false);
      tool->BeginUpdate(x - GetX(), y - GetY(), width, height);
    }
    else {

    }
  }
  _parent->SetDirty();

  Application* app = Application::Get();
  Model* model = app->GetModel();
  if(model->GetExec()) 
    model->SendExecViewEvent(_MouseButtonEventData(button, action, mods, x, y));
  
}

void ViewportUI::MouseMove(int x, int y) 
{
  Window* window = GetWindow();
  Tool* tool = window->GetTool();
  tool->SetCamera(_camera);
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
        break;
      }
       
      case INTERACTION_DOLLY:
      {
        _camera->Dolly(
          dx / static_cast<double>(GetWidth()), 
          dy / static_cast<double>(GetHeight())
        );
        break;
      }
        
      case INTERACTION_ORBIT:
      {
        _camera->Orbit(dx, dy);
        break;
      }

      default:
      {
        tool->Update(x - GetX(), y - GetY(), GetWidth(), GetHeight());
        break;
      }
    }
    _parent->SetDirty();
  } else {
    tool->Pick(x - GetX(), y - GetY(), GetWidth(), GetHeight());
  }

  _lastX = static_cast<double>(x);
  _lastY = static_cast<double>(y);
}

void ViewportUI::MouseWheel(int x, int y)
{
  Application* app = Application::Get();
  _camera->Dolly(
    static_cast<double>(x) / static_cast<double>(GetWidth()), 
    static_cast<double>(x) / static_cast<double>(GetHeight())
  );
  _parent->SetDirty();
}

void ViewportUI::Keyboard(int key, int scancode, int action, int mods)
{
  Application* app = Application::Get();
  int mappedKey = GetMappedKey(key);
  if (action == GLFW_PRESS) {
    switch (mappedKey) {
      case GLFW_KEY_A:
      {
        _camera->FrameSelection(Application::Get()->GetModel()->GetStageBoundingBox());
        break;
      }
      case GLFW_KEY_F:
      {
        Model* model = app->GetModel();
        if (model->GetSelection()->IsEmpty())return;
        _camera->FrameSelection(model->GetSelectionBoundingBox());
        break;
      }
      case GLFW_KEY_S:
      {
        RegistryWindow::Get()->SetActiveTool(Tool::SCALE);
        break;
      }
      case GLFW_KEY_R:
      {
        RegistryWindow::Get()->SetActiveTool(Tool::ROTATE);
        break;
      }
      case GLFW_KEY_T:
      {
        RegistryWindow::Get()->SetActiveTool(Tool::TRANSLATE);
        break;
      }
    }
  }
}

void ViewportUI::Render()
{
  Application* app = Application::Get();
  Window* window = GetWindow();

  //_engine->PollForAsynchronousUpdates();
  _engine->SetRenderViewport(
    pxr::GfVec4i(0, window->GetHeight() - GetHeight(), GetWidth(), GetHeight()));

  _engine->SetCameraMatrices(
    _camera->GetViewMatrix(),
    _camera->GetProjectionMatrix()
  );

  // clear to grey
  _drawTarget->Bind();
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.5,0.5,0.5,1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (_model->GetStage()->HasDefaultPrim()) {

    _engine->Prepare();
    _engine->Render();
  }
    
  _drawTarget->Unbind();
}

void 
ViewportUI::_DrawPickMode()
{ 
  static const size_t numPickModes = 3;
  static const char *pickModeStr[numPickModes] = {
    ICON_FA_HAND_POINTER " Prim",
    ICON_FA_HAND_POINTER " Model",
    ICON_FA_HAND_POINTER " Assembly"
  };

  Selection* selection = Application::Get()->GetModel()->GetSelection();
  if (UI::AddComboWidget("Pick", pickModeStr, numPickModes, _pickMode, 250)) {
    selection->SetMode((Selection::Mode)_pickMode);
    GetView()->SetFlag(View::DISCARDMOUSEBUTTON);
  }
  ImGui::SameLine();
}


void 
ViewportUI::_DrawAov()
{ 
  ImGui::SetNextItemWidth(32);
  const size_t numAovs = 6;
  static const TfToken aovTokens[numAovs] = {
    HdAovTokens->color,
    HdAovTokens->depth,
    HdAovTokens->depthStencil,
    HdAovTokens->cameraDepth,
    HdAovTokens->primId,
    HdAovTokens->instanceId
  };
  
  if (UI::AddComboWidget("Aov", aovTokens, numAovs, _aov, 140)) {
    Init();
    GetView()->SetFlag(View::DISCARDMOUSEBUTTON);
  }
  ImGui::SameLine();
}


bool ViewportUI::Draw()
{    
  Application* app = Application::Get();
  Window* window = GetWindow();
  if (!_initialized)Init();
  if(!_valid)return false;  


  
  if (_model->GetStage() != nullptr) {
    if(!(Time::Get()->IsPlaying() && !app->IsPlaybackView(_parent)))
      Render();
    
    Tool* tool = window->GetTool();
    
    const bool shouldDrawTool = tool->IsActive() && this->GetView()->GetFlag(View::ACTIVE);
   
    if (shouldDrawTool) {
      _toolTarget->Bind();
      // clear to black
      glClearColor(0.0f, 0.0f, 0.0f, 0.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      const float& wh = window->GetHeight();
      const float& h = GetHeight();
      const float& w = GetWidth();
      tool->SetViewport(GfVec4f(0, wh - (h), w, h));
      tool->SetCamera(_camera);
      tool->Draw();
      _toolTarget->Unbind();
    }
    
    const GfVec2f min(GetX(), GetY());
    const GfVec2f size(GetWidth(), GetHeight());
    const float u = (float)GetWidth() / (float)window->GetWidth();
    const float v = 1.f - (float)GetHeight() / (float)window->GetHeight();

    ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0, 0, 0, 1 });

    ImGui::SetNextWindowPos(min);
    ImGui::SetNextWindowSize(size);

    ImGui::Begin(_name.c_str(), NULL, _flags);
    
  
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    if (_drawTexId) {
       drawList->AddImage(

         (ImTextureID)(size_t)_drawTexId, 
         min, min + size, ImVec2(0, 1), 
         ImVec2(u, v));
    } 
    
    if( shouldDrawTool && _toolTexId) {
      drawList->AddImage(
        (ImTextureID)(uintptr_t)_toolTexId,
        min, min + size, ImVec2(0, 1), ImVec2(1, 0), ImColor(255, 255, 255, 255));
    }
    
    std::string msg = _message;
    
    drawList->AddText(
      ImVec2(min[0] + 20, (min[1] + size[1]) - 20), 
      0xFFFFFFFF, 
      msg.c_str());

    msg = "fps : "+ std::to_string(Time::Get()->IsPlaying() ?  ImGui::GetIO().Framerate : 0.f);
    drawList->AddText(
      ImVec2((min[0] + size[0]) - 128.f, (min[1] + size[1]) - 20),
      0xFFFFFFFF,
      msg.c_str());
    
    // renderer
    ImGui::SetCursorPosX(0);
    DiscardEventsIfMouseInsideBox(GfVec2f(0, 0), GfVec2f(GetWidth(), 24));
    if (UI::AddComboWidget("Renderer", _rendererNames, _numRenderers, _rendererIndex, 250)) {
      Init();
      GetView()->SetFlag(View::DISCARDMOUSEBUTTON);
    }
    ImGui::SameLine();

    Engine::RenderParams* params = _engine->GetRenderParams();
    // shaded mode
    if (UI::AddComboWidget("Draw", DRAW_MODE_NAMES, IM_ARRAYSIZE(DRAW_MODE_NAMES), params->drawMode, 250)) {
      GetView()->SetFlag(View::DISCARDMOUSEBUTTON);
    }
    ImGui::SameLine();

    _DrawPickMode();
    _DrawAov();

    // engine
    //ImGui::Text("%s", _engine->GetRendererDisplayName(_engine->GetCurrentRendererId()).c_str());
    

    //ImGui::PopFont();
    
    ImGui::End();
    ImGui::PopStyleColor();
  
    return ImGui::IsAnyItemActive() || GetView()->IsInteracting();
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
GfVec4f ViewportUI::ComputeCameraViewport(float cameraAspectRatio)
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
 return GfVec4f();
}

void ViewportUI::Resize()
{
  Window* window = GetWindow();
  if(!_initialized)return;

  if(GetWidth() <= 0 || GetHeight() <= 0)_valid = false;
  else _valid = true;
  
  window->SetGLContext();
  double aspectRatio = (double)GetWidth()/(double)GetHeight();
  _camera->Get()->SetPerspectiveFromAspectRatioAndFieldOfView(
    aspectRatio,
    _camera->GetFov(),
    GfCamera::FOVHorizontal
  );


  const GfVec2i& targetSize = _drawTarget->GetSize();
  if (window->GetWidth() != targetSize[0] || window->GetHeight() != targetSize[1]) {
    _BuildDrawTargets(pxr::GfVec2i(window->GetWidth(), window->GetHeight()));
  }
  GetView()->SetDirty();
}

static GfVec4i _ViewportMakeCenteredIntegral(GfVec4f& viewport)
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
  return GfVec4i(left, bottom, width, height);
}

 
GfFrustum 
ViewportUI::_ComputePickFrustum(int x, int y)
{
  const float targetAspectRatio = float(GetWidth()) / float(GfMax(1, GetHeight()));
  // normalize position and pick size by the viewport size
  GfVec2d point(
    (double)(x - GetX()) / (double)GetWidth(),
    (double)(y - GetY()) / (double)GetHeight());

  point[0] = (point[0] * 2.0 - 1.0);
  point[1] = -1.0 * (point[1] * 2.0 - 1.0);

  GfVec2d size(1.0 / (double)GetWidth(), 1.0 / (double)GetHeight());

  GfCamera camera = *(_camera->Get());
  CameraUtilConformWindow(
    &camera,
    CameraUtilConformWindowPolicy::CameraUtilFit,
    targetAspectRatio
  );

  GfFrustum cameraFrustum = camera.GetFrustum();
  return cameraFrustum.ComputeNarrowedFrustum(point, size);
}

bool ViewportUI::Pick(int x, int y, int mods)
{
  if (y - GetY() < 32) return false;

  GfFrustum pickFrustum = _ComputePickFrustum(x, y);

  return _engine->TestIntersection(pickFrustum.ComputeViewMatrix(), 
    pickFrustum.ComputeProjectionMatrix(), pxr::SdfPath("/"), &_pickHit);
}

bool ViewportUI::Select(int x, int y, int mods)
{
  if (y - GetY() < 32) return false;
  Application* app = Application::Get();
  Model* model = app->GetModel();
  Selection* selection = model->GetSelection();
  UsdStageRefPtr stage = model->GetWorkStage();
  if (!stage)return false;

  GfFrustum pickFrustum = _ComputePickFrustum(x, y);
  Engine::PickHit hit;
  
  if(_engine->TestIntersection(pickFrustum.ComputeViewMatrix(), 
    pickFrustum.ComputeProjectionMatrix(), pxr::SdfPath("/"), &hit)) {
      while (!selection->IsPickablePath(*stage, hit.objectId)) {
        hit.objectId = hit.objectId.GetParentPath();
      }

      if (mods & GLFW_MOD_CONTROL && mods & GLFW_MOD_SHIFT) {
        model->ToggleSelection({ hit.objectId });
      }
      else if (mods & GLFW_MOD_SHIFT) {
        model->AddToSelection({ hit.objectId });
      }
      else {
        model->SetSelection({ hit.objectId });
      }
    return true;
  } else {
    model->ClearSelection();
    return false;
  }
  
  return false;
}

JVR_NAMESPACE_CLOSE_SCOPE