#pragma once

#include "../default.h"
#include "../ui.h"
#include "../utils.h"

namespace AMN {
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
      void SetPixels(unsigned* pixels){_pixels=pixels;};

      void OnKeyboard() override;
      void OnMouseMove() override;
      void OnClick() override;
      void OnEnter() override;
      void OnLeave() override;
      void OnDraw() override;
    private:
      VIEWPORT_MODE       _mode;
      GLuint              _texture;
      unsigned*           _pixels;
  };
} // namespace AMN
