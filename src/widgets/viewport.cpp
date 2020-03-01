#include "viewport.h"
#include "../app/view.h"
#include "../utils/glutils.h"

AMN_NAMESPACE_OPEN_SCOPE

extern AmnUsdEmbreeContext* EMBREE_CTXT;


// constructor
AmnViewportUI::AmnViewportUI(AmnView* parent, VIEWPORT_MODE mode):AmnUI(parent, "viewport")
{
  parent->SetContent(this);
  _texture = 0;
  _mode = mode;
  _pixels = NULL;
}

// destructor
AmnViewportUI::~AmnViewportUI()
{
  if(_texture) glDeleteTextures(1, &_texture);
}

// overrides
void AmnViewportUI::Event() 
{

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

void AmnViewportUI::SetPixels(int width, int height, int* pixels)
{
  _width = width;
  _height = height;
  _pixels=pixels;
}

AMN_NAMESPACE_CLOSE_SCOPE