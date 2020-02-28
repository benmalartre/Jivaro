#include "viewport.h"
#include "../view.h"
#include "../glutils.h"

namespace AMN {
  // constructor
  ViewportUI::ViewportUI(View* parent, VIEWPORT_MODE mode):UI(parent, "viewport")
  {
    _mode = mode;
    _pixels = NULL;
    std::cerr << "CREATE VIEWPORT UI : PARENT = " << _parent << std::endl;
  }

  // destructor
  ViewportUI::~ViewportUI()
  {
    if(_texture) glDeleteTextures(1, &_texture);
    std::cerr << "DESTROY VIEWPORT UI" << std::endl;
  }

  // overrides
  void ViewportUI::OnKeyboard() 
  {

  }
  void ViewportUI::OnMouseMove()
  {

  }
  void ViewportUI::OnClick() 
  {

  }
  void ViewportUI::OnEnter() 
  {

  }
  void ViewportUI::OnLeave() 
  {

  }
  void ViewportUI::OnDraw()
  {
    std::cerr << "VIEWPORT DRAW..." << std::endl;
    float x = _parent->GetMin()[0];
    float y = _parent->GetMin()[1];
    float w = _parent->GetWidth();
    float h = _parent->GetHeight();
    std::cerr << x << "," << y << "," << "," << w << "," << h << std::endl;
    if(_pixels)
    {
      CreateOpenGLTexture(w, h, _pixels, _texture, 0);
      glViewport(x, y, w, h);

      glUseProgram(SCREENSPACEQUAD_PROGRAM_SHADER);
      glUniform1i(glGetUniformLocation(SCREENSPACEQUAD_PROGRAM_SHADER,"tex"),0);
      DrawScreenSpaceQuad();
    }
    else
    {
      glEnable(GL_SCISSOR_TEST);
      glScissor(x, y, w, h);
      glClearColor(RANDOM_0_1,RANDOM_0_1,RANDOM_0_1,1.f);
      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
      glDisable(GL_SCISSOR_TEST);
    }
    
  }
} // namespace AMN