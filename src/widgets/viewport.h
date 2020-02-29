#pragma once

#include "../default.h"
#include "../ui.h"
#include "../utils.h"

namespace AMN {
  // screen-space-quad 
  extern GLuint SCREENSPACEQUAD_VAO;
  extern GLuint SCREENSPACEQUAD_VBO;

  extern GLuint SCREENSPACEQUAD_VERTEX_SHADER;
  extern GLuint SCREENSPACEQUAD_FRAGMENT_SHADER;
  extern GLuint SCREENSPACEQUAD_PROGRAM_SHADER;


  enum VIEWPORT_MODE {
    OPENGL,
    HYDRA,
    EMBREE
  };

  class ViewportUI : public UI
  {
    public:
      ViewportUI(View* parent, VIEWPORT_MODE mode);
      ~ViewportUI();
      void SetMode(VIEWPORT_MODE mode){_mode=mode;};
      void SetPixels(int w, int h, int* pixels);
      void OnKeyboard() override;
      void OnMouseMove() override;
      void OnClick() override;
      void OnEnter() override;
      void OnLeave() override;
      void OnDraw() override;
    private:
      VIEWPORT_MODE       _mode;
      GLuint              _texture;
      int*                _pixels;
      int                 _width;
      int                 _height;
  };
} // namespace AMN
