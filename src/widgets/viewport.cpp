#include "viewport.h"
#include "../view.h"
#include "../glutils.h"

namespace AMN {

  // constructor
  ViewportUI::ViewportUI(View* parent, VIEWPORT_MODE mode):UI(parent, "viewport")
  {
    parent->SetContent(this);
    _texture = 0;
    _mode = mode;
    _pixels = NULL;
  }

  // destructor
  ViewportUI::~ViewportUI()
  {
    if(_texture) glDeleteTextures(1, &_texture);
  }

  // overrides
  void ViewportUI::Event() 
  {

  }
  void ViewportUI::Draw()
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

  void ViewportUI::SetPixels(int width, int height, int* pixels)
  {
    _width = width;
    _height = height;
    _pixels=pixels;
  }
} // namespace AMN