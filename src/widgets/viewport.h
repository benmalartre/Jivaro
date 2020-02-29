#pragma once

#include "../default.h"
#include "../context.h"
#include "../ui.h"
#include "../utils.h"

namespace AMN {
  extern UsdEmbreeContext* EMBREE_CTXT;
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

      // overrides
      void Event() override;
      void Draw() override;
      
    private:
      VIEWPORT_MODE       _mode;
      GLuint              _texture;
      int*                _pixels;
      int                 _width;
      int                 _height;
  };
} // namespace AMN
