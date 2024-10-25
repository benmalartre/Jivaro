#ifndef JVR_UI_VIEWPORT_H
#define JVR_UI_VIEWPORT_H

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

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"
#include "../app/camera.h"
#include "../app/notice.h"
#include "../app/handle.h"
#include "../app/engine.h"

JVR_NAMESPACE_OPEN_SCOPE

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
    ViewportUI(View* parent);
    ~ViewportUI();
    void Init();

    Engine* GetEngine(){return _engine;};
    Camera* GetCamera(){return _camera;};
    GfVec4f ComputeCameraViewport(float cameraAspectRatio);
    double GetLastMouseX(){return _lastX;};
    double GetLastMouseY(){return _lastY;};

    void UpdateLighting();
    void Render();

    // overrides
    void MouseButton(int button, int action, int mods) override;
    void MouseMove(int x, int y) override;
    void MouseWheel(int x, int y) override;
    void Keyboard(int key, int scancode, int action, int mods) override;
    bool Draw() override;
    void Resize() override;
    void Update();
    
    GfFrustum _ComputePickFrustum(int x, int y);
    bool Pick(int x, int y, int mods);

    void SetMessage(const std::string& message){_message = message;};

    /*
    HdSelectionSharedPtr _Pick(GfVec2i const& startPos,
      GfVec2i const& endPos, TfToken const& pickTarget);*/

  private:
    void                                _DrawAov();
    void                                _DrawPickMode();
    GLuint                              _texture;
    //int*                                _pixels;
    //int*                                _lowPixels;
    int                                 _width;
    int                                 _height;
    Camera*                             _camera;
    double                              _lastX;
    double                              _lastY;
    short                               _interactionMode;
    bool                                _valid;
    bool                                _highlightSelection;
    // usd imaging engine
    TfToken                             _aov;
    Engine*                             _engine;
    UsdPrim                             _root;
    UsdLuxDomeLight                     _light;
    GlfSimpleLightingContextRefPtr      _lightingContext;
    GlfDrawTargetRefPtr                 _drawTarget;
    GLuint                              _drawTexId = 0;
    GlfDrawTargetRefPtr                 _toolTarget;
    GLuint                              _toolTexId = 0;
    int                                 _drawMode;
    int                                 _pickMode;
    int                                 _rendererIndex;
    static ImGuiWindowFlags             _flags;

    const char**                        _rendererNames;
    int                                 _numRenderers;
    CameraUtilConformWindowPolicy  _conformWindowPolicy;



    std::string                         _message;

};
JVR_NAMESPACE_CLOSE_SCOPE

#endif //JVR_UI_VIEWPORT_H