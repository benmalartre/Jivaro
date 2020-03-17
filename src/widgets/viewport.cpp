#include "viewport.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../utils/glutils.h"
#include "../embree/context.h"
#include "../embree/camera.h"

AMN_NAMESPACE_OPEN_SCOPE

extern AmnUsdEmbreeContext* EMBREE_CTXT;

// constructor
AmnViewportUI::AmnViewportUI(AmnView* parent, VIEWPORT_MODE mode):
AmnUI(parent, "Viewport")
{
  _texture = 0;
  _mode = mode;
  _pixels = NULL;
  _camera = new embree::Camera();
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

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
      _interact = true;

    }
    else if(button == GLFW_MOUSE_BUTTON_LEFT)
    {
      _interact = true;
      /*
      if(mods == GLFW_MOD_ALT)
      {
        _interactionMode = INTERACTION_PANTILT;
      }
      else if(mods == GLFW_MOD_CONTROL)
      {
        _interactionMode = INTERACTION_TRUCK;
      }
      else if(mods == GLFW_MOD_SHIFT)
      {
        _interactionMode = INTERACTION_TRUCK;
      }
      */
    }
      /*
      if (button == GLFW_MOUSE_BUTTON_LEFT && mods == GLFW_MOD_SHIFT)
          _mouseMode = 1;
      else if (button == GLFW_MOUSE_BUTTON_LEFT && mods == GLFW_MOD_CONTROL ) 
        _mouseMode = 3;
      else if (button == GLFW_MOUSE_BUTTON_LEFT) 
        _mouseMode = 4;
      */
  }
}

void AmnViewportUI::MouseMove(int x, int y) 
{
  if(_interact)
  {
    int dx = x - _lastX;
    int dy = y - _lastY;
    _camera->rotateOrbit((float)dx/100, -(float)dy/100);
    RenderToMemory(_camera, true);
    SetContext(EMBREE_CTXT);
    _lastX = x;
    _lastY = y;
  }
  

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