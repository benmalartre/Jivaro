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

  //_engine = new pxr::UsdImagingGLLegacyEngine(pxr::SdfPathVector());
  _engine = new pxr::UsdImagingGLEngine();
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
  std::cout << "ViewportUI Init\n";
  pxr::SdfPathVector excludedPaths;
  pxr::TfToken renderer("HdStormRendererPlugin");
  if (pxr::UsdImagingGLEngine::IsHydraEnabled()) 
  {
    std::cout << "Using HD Renderer.\n";
    _engine = new pxr::UsdImagingGLEngine(
      stage->GetPseudoRoot().GetPath(), excludedPaths);
    if (!_engine->SetRendererPlugin(renderer)) 
    {
        std::cerr << "Couldn't set renderer plugin: " <<
            renderer.GetText() << std::endl;
        exit(-1);
    } 
    else 
    {
      std::cout << "Renderer plugin: " << renderer.GetText()
          << std::endl;
    }
  }
  else
  {
    std::cout << "Using Reference Renderer.\n"; 
    _engine = 
      new pxr::UsdImagingGLEngine(stage->GetPseudoRoot().GetPath(), 
                excludedPaths);
  }
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
        /*
        embree::Vec3fa dist = _camera->from - _camera->to;
        double d = embree::length(dist) * DEGREES_TO_RADIANS * _camera->fov * 0.5;
        dx = -dx/(double)(_parent->GetWidth()/2)*d;
        dy = dy/(double)(_parent->GetHeight()/2)*d;
        _camera->move (dx, dy, 0.0);
        */
        break;
      }
       
      case INTERACTION_DOLLY:
      {
        _camera->Dolly(
          (double)dx / (double)GetWidth(), 
          (double)dy / (double)GetHeight() 
        );
        /*
        if freeCam.orthographic:
            # orthographic cameras zoom by scaling fov
            # fov is the height of the view frustum in world units
            freeCam.fov *= (1 + zoomDelta)
        else:
            # perspective cameras dolly forward or back
            freeCam.AdjustDistance(1 + zoomDelta)
        */
        /*
        _camera->dolly(dy);
        */
        break;
      }
        
      case INTERACTION_ORBIT:
      {
        _camera->Orbit(dx, dy);
        //_camera->Tumble(0.25 * dx, 0.25*dy);
        /*
        _camera->rotateOrbit((float)dx/100, -(float)dy/100);
        */
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
  glViewport(x,y,w,h);
  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  Application* app = GetApplication();
  /*
  _engine->SetFreeCameraMatrices(
    _camera->GetViewMatrix(),
    _camera->GetProjectionMatrix()
  );
  */
  _engine->SetCameraState(
    _camera->GetViewMatrix(),
    _camera->GetProjectionMatrix()
  );
  _renderParams.frame = pxr::UsdTimeCode(app->GetCurrentTime());
  _renderParams.showGuides = true;
  _renderParams.showRender = true;
  _renderParams.showProxy = true;
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
  _engine->Render(app->GetStages()[0]->GetPseudoRoot(), _renderParams);
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