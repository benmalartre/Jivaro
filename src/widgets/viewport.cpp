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
  std::cout << "CONSTRUCT VIEWPORT" << std::endl;
  _texture = 0;
  _mode = mode;
  _pixels = NULL;
  _camera = new Camera("Camera");
  _valid = true;
  _camera->Set(pxr::GfVec3d(12,24,12),
              pxr::GfVec3d(0,0,0),
              pxr::GfVec3d(0,1,0));
  _interact = false;
  _interactionMode = INTERACTION_NONE;
  _engine = NULL;

  pxr::TfWeakPtr<ViewportUI> me(this);
  pxr::TfNotice::Register(me, &BaseUI::ProcessNewScene);
  std::cout << "CONSTRUCTED VIEWPORT" << std::endl;
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

  _engine = new pxr::UsdImagingGLEngine(pxr::SdfPath("/"), excludedPaths);
  _engine->SetRendererPlugin(pxr::TfToken("HdStormRendererPlugin"));
  //_engine->SetRendererPlugin(pxr::TfToken("LoFiRendererPlugin"));
  //_engine->SetRendererPlugin(pxr::TfToken("HdEmbreeRendererPlugin"));
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
    else window->RestoreLastActiveTool();
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

bool ViewportUI::Draw()
{    
  if (!_initialized)Init();
  if(!_valid)return false;  
  float x = _parent->GetMin()[0];
  float y = _parent->GetMin()[1];
  
  float w = _parent->GetWidth();
  float h = _parent->GetHeight();
  float wh = GetWindowHeight();

  glEnable(GL_SCISSOR_TEST);
  glScissor(x, wh - (y + h), w, h);
  glViewport(x,wh - (y + h), w, h);

  // clear to black
  glClearColor(0.0, 0.0, 0.0, 1.0 );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Application* app = GetApplication();
  if (app->GetStage() != nullptr) {
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    _engine->SetRenderViewport(
      pxr::GfVec4f(x, y, w , h)
    );
    _engine->SetCameraState(
      _camera->GetViewMatrix(),
      _camera->GetProjectionMatrix()
    );
  
    _renderParams.frame = pxr::UsdTimeCode(app->GetActiveTime());
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
    _renderParams.highlight = false;
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
    _engine->Render(app->GetStage()->GetPseudoRoot(), _renderParams);
    glDisable(GL_SCISSOR_TEST);

    /*
    bool open;
    ImGui::Begin("ViewportOverlay", &open, ImGuiWindowFlags_NoDecoration);
    ImGui::Text("FPS : %d", this->GetApplication()->GetFramerate());
    //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
    */

    return false;
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
}

AMN_NAMESPACE_CLOSE_SCOPE