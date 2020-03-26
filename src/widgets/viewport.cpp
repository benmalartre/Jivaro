#include "viewport.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/camera.h"
#include "../app/application.h"
#include "../utils/utils.h"
#include "../utils/strings.h"
#include "../utils/glutils.h"
#include "../embree/context.h"

AMN_NAMESPACE_OPEN_SCOPE


// constructor
ViewportUI::ViewportUI(View* parent, VIEWPORT_MODE mode):
BaseUI(parent, "Viewport")
{
  _texture = 0;
  _mode = mode;
  _pixels = NULL;
  _camera = new Camera("Camera");
  _valid = true;
  _camera->Set(pxr::GfVec3d(12,24,12),
              pxr::GfVec3d(0,0,0),
              pxr::GfVec3d(0,1,0));
  _context = NULL;
  _interact = false;
  _interactionMode = INTERACTION_NONE;

  pxr::SdfPathVector excludedPaths;
  _engine = new pxr::UsdImagingGLEngine(pxr::SdfPath("/engine"), excludedPaths);
  _engine->SetRendererPlugin(pxr::TfToken("HdStormRendererPlugin"));
}

// destructor
ViewportUI::~ViewportUI()
{
  if(_texture) glDeleteTextures(1, &_texture);
  if(_camera) delete _camera;
}

void ViewportUI::Init(pxr::UsdStageRefPtr stage)
{

  _stage = pxr::UsdStage::Open("/Users/benmalartre/Documents/RnD/amnesie/usd/board.usda");
  auto cameraPrim = _stage->GetPrimAtPath(pxr::SdfPath("/camera1"));
  _cameraX = pxr::UsdGeomCamera(cameraPrim).GetCamera(pxr::UsdTimeCode::Default());

  _root = _stage->GetPrimAtPath(pxr::SdfPath("/board1"));
  auto xfPrim = _stage->DefinePrim(pxr::SdfPath("/board1/SRT"), pxr::TfToken("Xform"));

  pxr::UsdGeomXformCommonAPI(xfPrim).SetTranslate(pxr::GfVec3f(0,2,0));

  xfPrim.GetReferences().AddReference(stage->GetRootLayer()->GetIdentifier(),
                                      pxr::SdfPath("/maneki"));
  

  _renderParams.frame = 1.0;
  _renderParams.complexity = 1.0f;

  pxr::GlfSimpleMaterial material;
  pxr::GlfSimpleLight light;
  light.SetAmbient(pxr::GfVec4f(0.25,0.25,0.25,1));
  light.SetPosition(pxr::GfVec4f(4,7,0,1));
  pxr::GlfSimpleLightVector lights;
  lights.push_back(light);

  material.SetAmbient(pxr::GfVec4f(0.5,1.0,0.5, 1.0));
  auto lightingContext = pxr::GlfSimpleLightingContext::New();
  lightingContext->SetMaterial(material);
  lightingContext->SetSceneAmbient(pxr::GfVec4f(0.5,0.5, 0.5,1));
  lightingContext->SetLights(lights);
  _engine->SetLightingState(lightingContext);
}

// overrides
void ViewportUI::MouseButton(int button, int action, int mods) 
{
  if (action == GLFW_RELEASE)
  {
    _interactionMode = INTERACTION_NONE;
    _interact = false;
    RenderToMemory(_camera, false);
    SetImage();
  }
  else if (action == GLFW_PRESS)
  {
    double x,y;
    glfwGetCursorPos(_parent->GetWindow()->GetGlfwWindow(),&x,&y);

    _lastX = (int)x;
    _lastY = (int)y;
    if( mods & GLFW_MOD_ALT)
    {
      _interact = true;
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
        _camera->Walk(
          (double)dx / (double)GetWidth(), 
          (double)dy / (double)GetHeight()
        );
       
        break;
      }
       
      case INTERACTION_DOLLY:
      {
        _camera->Dolly(
          (double)dx / (double)GetWidth(), 
          (double)dy / (double)GetHeight() 
        );
       
        break;
      }
        
      case INTERACTION_ORBIT:
      {
        _camera->Orbit(dx, dy);
       
        break;
      }

      default:
        break;
        
    }
    
    RenderToMemory(_camera, true);
    SetImage();
    _lastX = x;
    _lastY = y;
  }
}

void ViewportUI::MouseWheel(int x, int y)
{

  _camera->Dolly(
    (double)x / (double)_parent->GetWidth(), 
    (double)y / (double)_parent->GetHeight()
  );
  RenderToMemory(_camera, true);
  SetImage();
}

void ViewportUI::Draw()
{    
  if(!_valid)return;
  float x = _parent->GetMin()[0];
  float y = _parent->GetMin()[1];
  
  float w = _parent->GetWidth();
  float h = _parent->GetHeight();
  float wh = GetWindowHeight();

  glEnable(GL_SCISSOR_TEST);
  glScissor(x, wh - (y + h), w, h);
  glViewport(x,wh - (y + h),w,h);
  // clear to blue
  glClearColor(0.0, 0.0, 0.0, 1.0 );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);

  Application* app = GetApplication();

  _engine->SetRenderViewport(
    pxr::GfVec4f(x, wh - (y + h), w, h)
  );
  
  _engine->SetCameraState(
    _camera->GetViewMatrix(),
    _camera->GetProjectionMatrix()
  );

  _renderParams.frame = pxr::UsdTimeCode(app->GetCurrentTime());
  _renderParams.drawMode = pxr::UsdImagingGLDrawMode::DRAW_SHADED_FLAT;
  _renderParams.showGuides = true;
  _renderParams.showRender = true;
  _renderParams.showProxy = true;
  _renderParams.forceRefresh = true;
  _renderParams.gammaCorrectColors = false;
  _renderParams.enableIdRender = false;
  _renderParams.enableSampleAlphaToCoverage  = true;
  _renderParams.cullStyle = pxr::UsdImagingGLCullStyle::CULL_STYLE_NOTHING;
  _renderParams.clearColor = pxr::GfVec4f(1.f,0.f,0.f,1.f);
  _renderParams.renderResolution[0] = _parent->GetWidth();
  _renderParams.renderResolution[1] = _parent->GetHeight();


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
  _engine->Render(GetApplication()->GetStages()[0]->GetPseudoRoot(), _renderParams);
  glDisable(GL_SCISSOR_TEST);
  

  
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

void ViewportUI::Resize()
{
  if(_parent->GetWidth() <= 0 || _parent->GetHeight() <= 0)_valid = false;
  else _valid = true;
  if(_mode == EMBREE && _context)
  {
    
    _context->Resize(_parent->GetWidth(), _parent->GetHeight());
    _camera->SetWindow(
      _parent->GetX(),
      _parent->GetY(),
      _parent->GetWidth(),
      _parent->GetHeight()
    );
    
    RenderToMemory(_camera, false);
    SetImage();
    
  }   

}

void ViewportUI::SetImage()
{
  if(_context){
    _pixels = _context->_pixels;
    _lowPixels = _context->_lowPixels;
    _width = _context->_width;
    _height = _context->_height;
  }  
}

AMN_NAMESPACE_CLOSE_SCOPE