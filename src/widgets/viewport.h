#pragma once

#include "../default.h"
#include "../embree/context.h"
#include "../app/ui.h"
#include "../utils/utils.h"

AMN_NAMESPACE_OPEN_SCOPE

enum VIEWPORT_MODE {
  OPENGL,
  HYDRA,
  EMBREE
};

class AmnViewportUI : public AmnUI
{
  public:
    AmnViewportUI(AmnView* parent, VIEWPORT_MODE mode);
    ~AmnViewportUI();
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

AMN_NAMESPACE_CLOSE_SCOPE