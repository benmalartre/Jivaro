
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

#include <pxr/imaging/cameraUtil/conformWindow.h>

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
  , _drawMode((int)UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH)
  , _camera(new Camera("Camera"))
  , _valid(true)
  , _interactionMode(INTERACTION_NONE)
  , _engine(nullptr)
  , _rendererIndex(0)
  , _rendererNames(NULL)
  , _highlightSelection(true)
{
  _camera->Set(GfVec3d(12,24,12),
              GfVec3d(0,0,0),
              GfVec3d(0,1,0));
  
  const GfVec2i resolution(GetWindow()->GetResolution());
  {
    _drawTarget = GlfDrawTarget::New(resolution, false);
    _drawTarget->Bind();
    _drawTarget->AddAttachment("color", GL_RGBA, GL_FLOAT, GL_RGBA);
    _drawTarget->AddAttachment("depth", GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F);
    auto color = _drawTarget->GetAttachment("color");
    _drawTexId = color->GetGlTextureName();
    _drawTarget->Unbind();
  }
  
  {
    _toolTarget = GlfDrawTarget::New(resolution, false);
    _toolTarget->Bind();
    _toolTarget->AddAttachment("color", GL_RGBA, GL_FLOAT, GL_RGBA);
    _toolTarget->AddAttachment("depth", GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F);
     auto color = _toolTarget->GetAttachment("color");
    _toolTexId = color->GetGlTextureName();
    _toolTarget->Unbind();
  }
}

// destructor
ViewportUI::~ViewportUI()
{
  Application* app = Application::Get();
  if(_rendererNames)delete[] _rendererNames;
  if(_texture) glDeleteTextures(1, &_texture);
  if(_camera) delete _camera;
  if (_engine) {
    app->RemoveEngine(_engine);
    delete _engine;
  }
}

void ViewportUI::Init()
{
  Application* app = Application::Get();
  if (_engine) {
    app->RemoveEngine(_engine);
    delete _engine;
  }
  SdfPathVector excludedPaths;
  _engine = new Engine(SdfPath("/"), excludedPaths);
  app->AddEngine(_engine);

  TfTokenVector rendererTokens = _engine->GetRendererPlugins();
  if (_rendererNames) delete[] _rendererNames;
  _numRenderers = rendererTokens.size();
  _rendererNames = new const char* [_numRenderers];
  for (short rendererIndex = 0; rendererIndex < _numRenderers; ++rendererIndex) {
    _rendererNames[rendererIndex] = rendererTokens[rendererIndex].GetText();
  }
  if (LEGACY_OPENGL) {
    _engine->SetRendererPlugin(TfToken("LoFiRendererPlugin"));
  } else {
    _engine->SetRendererPlugin(TfToken(_rendererNames[_rendererIndex]));
  }

  GlfSimpleMaterial material;
  GlfSimpleLight simpleLight;
  simpleLight.SetAmbient({ 0.5, 0.3, 0.4, 1.0 });
  simpleLight.SetDiffuse({ 1.0, 1.0, 1.0, 1.f });
  simpleLight.SetSpecular({ 0.2, 0.2, 0.2, 1.f });
  simpleLight.SetPosition({ 200, 200, 200, 1.0 });
  //simpleLight.SetIsDomeLight(true);
  GlfSimpleLightVector lights;
  lights.push_back(simpleLight);

  material.SetAmbient({ 0.2f, 0.2f, 0.2f, 1.f });
  material.SetDiffuse({ 1.0, 1.f, 1.f, 1.f });
  material.SetSpecular({ 0.5f, 0.5f, 0.5f, 1.f });
  auto lightingContext = GlfSimpleLightingContext::New();

  _engine->SetLightingState(lights,
                            material,
                            GfVec4f(0.5,0.5,0.5,1.0));

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
        Pick(x, y, mods);
      }
    }
  }
  else if (action == GLFW_PRESS)
  {
    _lastX = (int)x;
    _lastY = (int)y;
    Application::Get()->SetActiveEngine(_engine);
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
  _engine->SetDirty(true);
  _parent->SetDirty();

  Application* app = Application::Get();
  if(app->GetExec()) 
    app->SendExecViewEvent(&_MouseButtonEventData(button, action, mods, x, y));
  
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
    _engine->SetDirty(true);
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
  _engine->SetDirty(true);
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
        _camera->FrameSelection(Application::Get()->GetStageBoundingBox());
        _engine->SetDirty(true);
        break;
      }
      case GLFW_KEY_F:
      {
        if (app->GetSelection()->IsEmpty())return;
        _camera->FrameSelection(Application::Get()->GetSelectionBoundingBox());
        _engine->SetDirty(true);
        break;
      }
      case GLFW_KEY_S:
      {
        app->SetActiveTool(Tool::SCALE);
        break;
      }
      case GLFW_KEY_R:
      {
        app->SetActiveTool(Tool::ROTATE);
        break;
      }
      case GLFW_KEY_T:
      {
        app->SetActiveTool(Tool::TRANSLATE);
        break;
      }
    }
  }
}

static bool ComboWidget(const char* label, BaseUI* ui, 
  const char** names, const size_t count, int& last, size_t width=300)
{
  int current = last;
  ImGui::Text("%s", label);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(300);
  std::string name("##");
  name += label;
  if (ImGui::BeginCombo(name.c_str(), names[current], ImGuiComboFlags_PopupAlignLeft))
  {
    for (int n = 0; n < count; ++n)
    {
      const bool is_selected = (current == n);
      if (ImGui::Selectable(names[n], is_selected))
      {
        current = n;
      }

      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ui->GetView()->SetFlag(View::DISCARDMOUSEBUTTON);
    ImGui::EndCombo();
  }
  if (current != last) {
    last = current;
    ui->GetView()->ClearFlag(View::DISCARDMOUSEBUTTON);
    ImGui::SameLine();
    return true;
  }
  ImGui::SameLine();
  return false;
}

void ViewportUI::Render()
{
  Application* app = Application::Get();
  Window* window = GetWindow();

  const float& wh = window->GetHeight();
  const float& h = GetHeight();
  const float& w = GetWidth();

  _engine->SetRendererAov(HdAovTokens->color);

  _engine->SetRenderViewport(
    GfVec4d(0, wh-(h), w, h));

  _engine->SetCameraState(
    _camera->GetViewMatrix(),
    _camera->GetProjectionMatrix()
  );

  _engine->SetSelectionColor(GfVec4f(0.75, 0.75, 0, 0.5));

  _renderParams.frame = UsdTimeCode(Time::Get()->GetActiveTime());
  _renderParams.complexity = 1.0f;
  _renderParams.drawMode = (UsdImagingGLDrawMode)_drawMode;
  _renderParams.showGuides = true;
  _renderParams.showRender = true;
  _renderParams.showProxy = true;
  _renderParams.forceRefresh = true;
  _renderParams.cullStyle = UsdImagingGLCullStyle::CULL_STYLE_BACK_UNLESS_DOUBLE_SIDED;
  _renderParams.gammaCorrectColors = false;
  _renderParams.enableIdRender = false;
  _renderParams.enableSampleAlphaToCoverage = true;
  _renderParams.highlight = true;
  _renderParams.enableSceneMaterials = true;
  _renderParams.enableSceneLights = true;
  //_renderParams.colorCorrectionMode = ???
  _renderParams.clearColor = GfVec4f(0.25,0.25,0.25,1.0);

  if (_highlightSelection) {
    Selection* selection = app->GetSelection();
    if (!selection->IsEmpty() && selection->IsObject()) {
      _engine->SetSelected(selection->GetSelectedPaths());
    } else {
      _engine->ClearSelected();
    }
  } else {
    _engine->ClearSelected();
  }

  // clear to black
  _drawTarget->Bind();
  glEnable(GL_DEPTH_TEST);
  glClearColor(_renderParams.clearColor[0], 
               _renderParams.clearColor[1], 
               _renderParams.clearColor[2],
               _renderParams.clearColor[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (app->GetDisplayStage()->HasDefaultPrim()) {
    _engine->Render(app->GetDisplayStage()->GetDefaultPrim(), _renderParams);
  }
  _drawTarget->Unbind();
  
  _engine->SetDirty(false);
}

void 
ViewportUI::_DrawPickMode()
{ 
  ImGui::SameLine();
  static const char *pickModeStr[5] = {
    ICON_FA_HAND_POINTER "    Assembly",
    ICON_FA_HAND_POINTER "       Model",
    ICON_FA_HAND_POINTER "       Group",
    ICON_FA_HAND_POINTER "   Component",
    ICON_FA_HAND_POINTER "SubComponent",
  };
  Selection* selection = Application::Get()->GetSelection();
  if (ImGui::BeginCombo("##Pick mode", pickModeStr[int(selection->GetMode())], ImGuiComboFlags_NoArrowButton)) {
    if (ImGui::Selectable(pickModeStr[0])) {
      selection->SetMode(Selection::Mode::ASSEMBLY);
    }
    if (ImGui::Selectable(pickModeStr[1])) {
      selection->SetMode(Selection::Mode::MODEL);
    }
    if (ImGui::Selectable(pickModeStr[2])) {
      selection->SetMode(Selection::Mode::GROUP);
    }
    if (ImGui::Selectable(pickModeStr[3])) {
      selection->SetMode(Selection::Mode::COMPONENT);
    }
    if (ImGui::Selectable(pickModeStr[4])) {
      selection->SetMode(Selection::Mode::SUBCOMPONENT);
    }
    ImGui::EndCombo();
  }
}

bool ViewportUI::Draw()
{    
  Application* app = Application::Get();
  Window* window = GetWindow();
  if (!_initialized)Init();
  if(!_valid)return false;  
  
  if (app->GetDisplayStage() != nullptr) {
    if ( _engine->IsDirty() || !_engine->IsConverged()) {
      Render();
    }

    Tool* tool = window->GetTool();
    const bool shouldDrawTool = tool->IsActive();
   
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

    msg = "fps : "+ std::to_string(ImGui::GetIO().Framerate);
    drawList->AddText(
      ImVec2((min[0] + size[0]) - 128.f, (min[1] + size[1]) - 20),
      0xFFFFFFFF,
      msg.c_str());
    
    // renderer
    ImGui::SetCursorPosX(0);
    DiscardEventsIfMouseInsideBox(GfVec2f(0, 0), GfVec2f(GetWidth(), 24));
    if (ComboWidget("Renderer", this, _rendererNames, _numRenderers, _rendererIndex, 300)) {
      Init();
    }

    // shaded mode
    if (ComboWidget("Mode", this, DRAW_MODE_NAMES, IM_ARRAYSIZE(DRAW_MODE_NAMES), _drawMode, 300)) {
      _engine->SetDirty(true);
    }

    _DrawPickMode();

    // engine
    ImGui::Text("%s", _engine->GetRendererDisplayName(
      _engine->GetCurrentRendererId()).c_str());
    

    //ImGui::PopFont();
    
    ImGui::End();
    ImGui::PopStyleColor();
  
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
    _drawTarget->Bind();
    _drawTarget->SetSize(GfVec2i(window->GetWidth(), window->GetHeight()));
    _drawTarget->Unbind();

    _toolTarget->Bind();
    _toolTarget->SetSize(GfVec2i(window->GetWidth(), window->GetHeight()));
    _toolTarget->Unbind();
  }

  _engine->SetDirty(true);
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
  std::cout << "Viewport UI PICK CALLED!!!" << std::endl;
  if (y - GetY() < 32) return false;
  Application* app = Application::Get();
  Selection* selection = app->GetSelection();
  UsdStageRefPtr stage = app->GetWorkStage();
  if (!stage)return false;

  GfFrustum pickFrustum = _ComputePickFrustum(x, y);
  GfVec3d outHitPoint;
  GfVec3d outHitNormal;
  SdfPath outHitPrimPath;
  SdfPath outHitInstancerPath;
  int outHitInstanceIndex;
  HdInstancerContext outInstancerContext;

  if (_engine->TestIntersection(
    pickFrustum.ComputeViewMatrix(),
    pickFrustum.ComputeProjectionMatrix(),
    Application::Get()->GetDisplayStage()->GetPseudoRoot(),
    _renderParams,
    &outHitPoint,
    &outHitNormal,
    &outHitPrimPath,
    &outHitInstancerPath,
    &outHitInstanceIndex,
    &outInstancerContext)) {
      while (!selection->IsPickablePath(*stage, outHitPrimPath)) {
        outHitPrimPath = outHitPrimPath.GetParentPath();
      }

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

JVR_NAMESPACE_CLOSE_SCOPE