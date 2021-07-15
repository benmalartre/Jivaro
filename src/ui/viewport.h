#pragma once

#include "../common.h"
#include "../ui/ui.h"
#include "../app/camera.h"
#include "../app/notice.h"
#include "../app/handle.h"
#include "../utils/utils.h"
#include <pxr/pxr.h>

#include <pxr/imaging/garch/glApi.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdLux/domeLight.h>
#include <pxr/base/gf/camera.h>
#include "pxr/imaging/glf/simpleMaterial.h"
#include "pxr/imaging/glf/simpleLight.h"
#include "pxr/imaging/glf/simpleLightingContext.h"
#include "pxr/imaging/glf/drawTarget.h"
#include "../app/engine.h"


AMN_NAMESPACE_OPEN_SCOPE

enum InteractionMode{
  INTERACTION_NONE,
  INTERACTION_WALK,
  INTERACTION_ORBIT,
  INTERACTION_DOLLY,
  INTERACTION_PICKSELECT,
  INTERACTION_RECTANGLESELECT,
  INTERACTION_FREESELECT,
  INTERACTION_TRANSLATE,
  INTERACTION_ROTATE,
  INTERACTION_SCALE
};

enum VIEWPORT_MODE {
  OPENGL,
  LOFI,
  EMBREE
};

static const char* DRAW_MODE_NAMES[] = { 
  "Points", 
  "Wireframe", 
  "Wireframe On Surface", 
  "Shaded Flat", 
  "Shaded Smooth", 
  "Geom Only",
  "Geom Flat",
  "Geom Smooth"
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

    double GetLastMouseX(){return _lastX;};
    double GetLastMouseY(){return _lastY;};

    // overrides
    void MouseButton(int button, int action, int mods) override;
    void MouseMove(int x, int y) override;
    void MouseWheel(int x, int y) override;
    void Keyboard(int key, int scancode, int action, int mods) override;
    bool Draw() override;
    void Resize() override;
    void Update();
    pxr::GfFrustum _ComputePickFrustum(int x, int y);
    bool Pick(int x, int y);
    /*
    pxr::HdSelectionSharedPtr _Pick(pxr::GfVec2i const& startPos,
      pxr::GfVec2i const& endPos, pxr::TfToken const& pickTarget);*/
    
    
  private:
    short                 _mode;
    GLuint                _texture;
    int*                  _pixels;
    int*                  _lowPixels;
    int                   _width;
    int                   _height;
    Camera*               _camera;
    double                _lastX;
    double                _lastY;
    bool                  _interact;
    short                 _interactionMode;
    bool                  _valid;

    // usd imaging engine
    Engine*                           _engine;
    pxr::UsdImagingGLRenderParams     _renderParams;
    pxr::UsdPrim                      _root;
    pxr::UsdLuxDomeLight              _light;
    pxr::GlfDrawTargetRefPtr          _drawTarget;
    int                               _drawMode;
    //pxr::UsdStageRefPtr               _stage;
};
AMN_NAMESPACE_CLOSE_SCOPE