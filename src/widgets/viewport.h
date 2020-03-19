#pragma once

#include "../default.h"
#include "../embree/context.h"
#include "../embree/device.h"
#include "../app/ui.h"
#include "../utils/utils.h"

AMN_NAMESPACE_OPEN_SCOPE

enum InteractionMode{
  INTERACTION_NONE,
  INTERACTION_PAN,
  INTERACTION_ORBIT,
  INTERACTION_DOLLY
  /*
  INTERACTION_PICKSELECT,
  INTERACTION_RECTANGLESELECT,
  INTERACTION_TRANSLATE,
  INTERACTION_ROTATE,
  INTERACTION_SCALE
  */
};

enum VIEWPORT_MODE {
  OPENGL,
  HYDRA,
  EMBREE
};
class AmnUsdEmbreeContext;
class AmnViewportUI : public AmnUI
{
  public:
    AmnViewportUI(AmnView* parent, VIEWPORT_MODE mode);
    ~AmnViewportUI();
    void SetMode(VIEWPORT_MODE mode){_mode=mode;};
    void SetContext(AmnUsdEmbreeContext* ctxt);
    AmnCamera* GetCamera(){return _camera;};

    // overrides
    void MouseButton(int button, int action, int mods) override;
    void MouseMove(int x, int y) override;
    void MouseWheel(int x, int y) override;
    void Draw() override;
    void Resize() override;
    
    
  private:
    VIEWPORT_MODE         _mode;
    GLuint                _texture;
    int*                  _pixels;
    int*                  _lowPixels;
    int                   _width;
    int                   _height;
    AmnUsdEmbreeContext*  _context;
    AmnCamera*            _camera;
    int                   _lastX;
    int                   _lastY;
    bool                  _interact;
    InteractionMode       _interactionMode;
};
AMN_NAMESPACE_CLOSE_SCOPE