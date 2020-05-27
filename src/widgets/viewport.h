#pragma once

#include "../common.h"
#include "../app/ui.h"
#include "../app/camera.h"
#include "../utils/ui.h"
#include "../utils/utils.h"
#include <pxr/pxr.h>

#include <pxr/imaging/glf/glew.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdLux/domeLight.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/base/gf/camera.h>
#include "pxr/imaging/glf/simpleMaterial.h"
#include "pxr/imaging/glf/simpleLight.h"
#include "pxr/imaging/glf/simpleLightingContext.h"


AMN_NAMESPACE_OPEN_SCOPE

enum InteractionMode{
  INTERACTION_NONE,
  INTERACTION_WALK,
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
  LOFI,
  EMBREE
};

class ViewportUI : public BaseUI
{
  public:
    ViewportUI(View* parent, VIEWPORT_MODE mode);
    ~ViewportUI();
    void Init();

    void SetMode(VIEWPORT_MODE mode){_mode=mode;};
    Camera* GetCamera(){return _camera;};
    pxr::GfVec4f ComputeCameraViewport(float cameraAspectRatio);

    // overrides
    void MouseButton(int button, int action, int mods) override;
    void MouseMove(int x, int y) override;
    void MouseWheel(int x, int y) override;
    void Draw() override;
    void Resize() override;
    void Update();
    
    
  private:
    VIEWPORT_MODE         _mode;
    GLuint                _texture;
    int*                  _pixels;
    int*                  _lowPixels;
    int                   _width;
    int                   _height;
    Camera*               _camera;
    int                   _lastX;
    int                   _lastY;
    bool                  _interact;
    InteractionMode       _interactionMode;
    bool                  _valid;

    // usd imaging engine
    pxr::UsdImagingGLEngine*          _engine;
    pxr::UsdImagingGLRenderParams     _renderParams;
    pxr::UsdPrim                      _root;
    pxr::UsdLuxDomeLight              _light;
    pxr::UsdStageRefPtr               _stage;
    pxr::GfCamera                     _cameraX;
};
AMN_NAMESPACE_CLOSE_SCOPE