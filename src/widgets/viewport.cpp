#include "viewport.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/camera.h"
#include "../utils/utils.h"
#include "../utils/glutils.h"
#include "../embree/context.h"


AMN_NAMESPACE_OPEN_SCOPE

extern AmnUsdEmbreeContext* EMBREE_CTXT;

// constructor
AmnViewportUI::AmnViewportUI(AmnView* parent, VIEWPORT_MODE mode):
AmnUI(parent, "Viewport")
{
  _texture = 0;
  _mode = mode;
  _pixels = NULL;
  _camera = new AmnCamera("Camera");
  pxr::GfCamera* camera = _camera->Get();
  pxr::GfMatrix4d m(1);
  m[3][0] = 0;
  m[3][1] = 0;
  m[3][2] = 32;
  camera->SetTransform(m);
  camera->SetFocusDistance(32);

  _interact = false;
  _interactionMode = INTERACTION_NONE;
}

// destructor
AmnViewportUI::~AmnViewportUI()
{
  if(_texture) glDeleteTextures(1, &_texture);
  if(_camera) delete _camera;
}

// overrides
void AmnViewportUI::MouseButton(int button, int action, int mods) 
{
  if (action == GLFW_RELEASE)
  {
    _interactionMode = INTERACTION_NONE;
    _interact = false;
    RenderToMemory(_camera, false);
    SetContext(EMBREE_CTXT);
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
        _interactionMode = INTERACTION_DOLLY;
      }
      else if(button == GLFW_MOUSE_BUTTON_RIGHT)
      {
        _interactionMode = INTERACTION_PAN;
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
        _interactionMode = INTERACTION_PAN;
      }
      else if(mods == GLFW_MOD_SHIFT)
      {
        _interactionMode = INTERACTION_DOLLY;
      }
    }
*/
  }
}

void AmnViewportUI::MouseMove(int x, int y) 
{
  if(_interact)
  {
    double dx = x - _lastX;
    double dy = y - _lastY;
    switch(_interactionMode)
    {
      case INTERACTION_PAN:
      {
        std::cout << "PAN..." << std::endl;
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
        double zoomDelta = -.002 * (dx + dy);
        //_camera->AdjustDistance(1 + zoomDelta);
        std::cout << "DOLLY..." << std::endl;
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
        std::cout << "ORBIT..." << std::endl;
        _camera->Tumble(0.25 * dx, 0.25*dy);
        /*
        _camera->rotateOrbit((float)dx/100, -(float)dy/100);
        */
        break;
      }

      default:
        break;
        
    }
    _camera->ComputeFrustum();
    RenderToMemory(_camera, true);
    SetContext(EMBREE_CTXT);
    _lastX = x;
    _lastY = y;
  }
}

void AmnViewportUI::MouseWheel(int x, int y)
{
  std::cout << "AmnViewportUI MOUSE WHEEL EVENET ..." << std::endl;
}

void AmnViewportUI::Draw()
{    
  float x = _parent->GetMin()[0];
  float y = _parent->GetMin()[1];
  
  float w = _parent->GetWidth();
  float h = _parent->GetHeight();
      
  if(_pixels)
  {
    glUseProgram(EMBREE_CTXT->_screenSpaceQuadPgm);
    if(_interact)
      CreateOpenGLTexture(_width*0.1, _height*0.1, _lowPixels, _texture, 0);
    else
      CreateOpenGLTexture(_width, _height, _pixels, _texture, 0);
    glViewport(x, GetWindowHeight()-(y+h), w, h);
    glUniform1i(glGetUniformLocation(EMBREE_CTXT->_screenSpaceQuadPgm,"tex"),0);
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
  
}

void AmnViewportUI::Resize()
{
  if(_mode == EMBREE)
  {
    EMBREE_CTXT->Resize(_parent->GetWidth(), _parent->GetHeight());
    //_camera->ComputeFrustum();
    RenderToMemory(_camera, false);
    SetContext(EMBREE_CTXT);
  }   
}

void AmnViewportUI::SetContext(AmnUsdEmbreeContext* ctxt)
{
  _context = ctxt;
  _pixels = _context->_pixels;
  _lowPixels = _context->_lowPixels;
  _width = _context->_width;
  _height = _context->_height;
}

AMN_NAMESPACE_CLOSE_SCOPE