#ifndef JVR_APPLICATION_HANDLE_H
#define JVR_APPLICATION_HANDLE_H

#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/quaternion.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/vec4f.h>
#include <pxr/base/gf/plane.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>

#include "../common.h"
#include "../geometry/shape.h"
#include "../geometry/utils.h"

JVR_NAMESPACE_OPEN_SCOPE

static const GfMatrix4f HANDLE_X_MATRIX = {
  0.f, 1.f, 0.f, 0.f,
  1.f, 0.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 0.f, 0.f, 1.f
};

static const GfMatrix4f HANDLE_Y_MATRIX = {
  1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 0.f, 0.f, 1.f
};

static const GfMatrix4f HANDLE_Z_MATRIX = {
  1.f, 0.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 0.f, 1.f
};

static const GfVec4f HANDLE_X_COLOR =  {1.f, 0.25f, 0.5f, 1.f};
static const GfVec4f HANDLE_Y_COLOR =  {0.5f, 1.f, 0.25f, 1.f};
static const GfVec4f HANDLE_Z_COLOR =  {0.25f, 0.5f, 1.f, 1.f}; 
static const GfVec4f HANDLE_HELP_COLOR = {0.66f, 0.66f, 0.66f, 0.5f};
static const GfVec4f HANDLE_HOVERED_COLOR = {1.f, 0.5f, 0.0f, 1.f};
static const GfVec4f HANDLE_ACTIVE_COLOR = {1.f, 0.75f, 0.25f, 1.f};
static const GfVec4f HANDLE_MASK_COLOR = {0.f, 0.f, 0.f, 0.f};
static float HANDLE_SIZE = 100.f;

class Camera;
class Geometry;

struct HandleTargetGeometryDesc {
  Geometry* geometry;
  std::vector<int> elements;
  std::vector<float> weights;
};

typedef std::vector<HandleTargetGeometryDesc> HandleTargetGeometryDescList;


void _EnsureXformCommonAPI(UsdPrim prim, const UsdTimeCode& timeCode);


class BaseHandle {
public:
  enum State {
    STATE_DEFAULT,
    STATE_HOVERED,
    STATE_ACTIVE
  };

  enum Mode {
    MODE_LOCAL = 1,
    MODE_GLOBAL = 2,
    MODE_REFERENCE = 4,
    MODE_COG = 8
  };

  enum Axis {
    AXIS_NONE,
    AXIS_X,
    AXIS_Y,
    AXIS_Z,
    AXIS_XY,
    AXIS_XZ,
    AXIS_YZ,
    AXIS_XYZ,
    AXIS_CAMERA,
    AXIS_NORMAL,
    AXIS_RAY,
    AXIS_LAST
  };

  enum PlaneNormal {
    NORMAL_X,
    NORMAL_Y,
    NORMAL_Z,
    NORMAL_CAMERA
  };

  BaseHandle(bool compensate=true) 
    : _activeAxis(AXIS_NONE)
    , _hoveredAxis(AXIS_NONE)
    , _activeNormal(NORMAL_CAMERA)
    , _activeMask(0b1111111111)
    , _camera(NULL)
    , _interacting(false)
    , _compensate(compensate)
    , _mode(MODE_LOCAL | MODE_COG)
    , _position(GfVec3d(0.f))
    , _rotation(GfQuatf(1.f))
    , _scale(GfVec3d(1.f))
    , _matrix(GfMatrix4f(1.f))
    , _startMatrix(GfMatrix4f(1.f))
    , _viewPlaneMatrix(GfMatrix4f(1.f)) {};
  virtual ~BaseHandle(){};
  
  void SetActiveAxis(short axis);
  void SetHoveredAxis(short axis);
  void SetCamera(Camera* camera) {_camera = camera;};
  void SetMatrixFromSRT();
  void SetSRTFromMatrix();
  void ResetSelection();
  void AddComponent(Shape::Component& component);
  void AddXComponent(Shape::Component& component);
  void AddYComponent(Shape::Component& component);
  void AddZComponent(Shape::Component& component);
  void AddXYZComponents(Shape::Component& component);
  void AddYZXZXYComponents(Shape::Component& component);
  void AddHelperComponent(Shape::Component& component);
  void UpdateCamera(const GfMatrix4f& view,
    const GfMatrix4f& proj);
  void UpdatePickingPlane(short axis=NORMAL_CAMERA);
  void ComputeSizeMatrix(float width, float height);
  void ComputeViewPlaneMatrix();
  void ComputePickFrustum();
  
  const GfVec4f& GetColor(const Shape::Component& comp);
  short GetActiveAxis(){return _activeAxis;};

  virtual void SetVisibility(short axis, short mask);
  virtual void Setup();
  virtual void SetProgram(GLSLProgram* pgm);
  virtual void Draw(float width, float height);
  virtual short Select(float x, float y, float width, float height, bool lock);
  virtual short Pick(float x, float y, float width, float height);
  virtual void BeginUpdate(float x, float y, float width, float height);
  virtual void Update(float x, float y, float width, float height);
  virtual void EndUpdate();

protected:
  virtual void _DrawShape(Shape* shape, const GfMatrix4f& m = GfMatrix4f(1.f));
  virtual void _ComputeCOGMatrix();
  virtual void _UpdateTargets(bool interacting) = 0;
  GfVec3f _ConstraintPointToAxis(const GfVec3f& point, short axis);
  GfVec3f _ConstraintPointToPlane(const GfVec3f& point, short axis);
  GfVec3f _ConstraintPointToCircle(const GfVec3f& center, const GfVec3f& normal,
    const GfRay& ray, short axis, float radius);
  GfMatrix4f _ExtractRotationAndTranslateFromMatrix();
  virtual void _UpdateActiveMask();

  // handle transformation flags
  short                   _mode;

  // targets
  ManipTargetDescList     _targets;
  
  // geometry
  Shape                   _shape;
  Shape                   _help;

  // viewport
  Camera*                 _camera;

  // state
  short                   _activeNormal;
  short                   _activeMask;
  short                   _activeAxis;
  short                   _hoveredAxis;
  short                   _lastActiveAxis;
  float                   _size;
  float                   _distance;
  bool                    _compensate;
  bool                    _interacting;
  bool                    _needUpdate;
  UsdGeomXformCache  _xformCache;

  // data
  GfVec3f            _scale;
  GfVec3f            _position;
  GfVec3f            _offset;
  GfVec3f            _normal;
  GfQuatf            _rotation;
  GfPlane            _plane;
  GfMatrix4f         _matrix;
  GfMatrix4f         _displayMatrix;
  GfMatrix4f         _startMatrix;
  GfMatrix4f         _viewPlaneMatrix;
  GfMatrix4f         _normalPlaneMatrix;
  GfMatrix4f         _sizeMatrix;
};

class SelectHandle : public BaseHandle {
  enum Mode {
    RECTANGLE,
    LASSO
  };

public:
  SelectHandle();

  void BeginUpdate(float x, float y, float width, float height) override;
  void Update(float x, float y, float width, float height) override;
  void EndUpdate() override;
  void _DrawShape(Shape* shape, const GfMatrix4f& m = GfMatrix4f(1.f)) override;
  void SetVisibility(short axis, short mask) override;

protected:
  void _UpdateTargets(bool interacting) override;

private:
  
};

class ScaleHandle : public BaseHandle {
public:
  ScaleHandle();

  void BeginUpdate(float x, float y, float width, float height) override;
  void Update(float x, float y, float width, float height) override;
  void EndUpdate() override;
  void _DrawShape(Shape* shape, const GfMatrix4f& m = GfMatrix4f(1.f)) override;
  void SetVisibility(short axis, short mask) override;

protected:
  void _UpdateTargets(bool interacting) override;

private:
  GfVec3f _GetScaleOffset(size_t axis);
  GfVec3f _GetTranslateOffset(size_t axis);
  void         _SetMaskMatrix(size_t axis);

  GfVec3f    _offsetScale;
  GfVec3f    _baseScale;
  GfMatrix4f _maskMatrix;
};

class RotateHandle : public BaseHandle {
public:
  RotateHandle();

  void BeginUpdate(float x, float y, float width, float height) override;
  void Update(float x, float y, float width, float height) override;
  void SetVisibility(short axis, short mask) override;

protected:
  void _UpdateTargets(bool interacting) override;

private:
  GfVec3f      _ContraintPointToRotationPlane(const GfRay& ray);
  float             _radius;
  GfQuatf      _base;
};

class TranslateHandle : public BaseHandle {
public:
  TranslateHandle();

  void BeginUpdate(float x, float y, float width, float height) override;
  void Update(float x, float y, float width, float height) override;
  void SetVisibility(short axis, short mask) override;

protected:
  void _UpdateTargets(bool interacting) override;
  void _UpdateActiveMask() override;

private:
  float _radius;
  float _height;
};

/*
class PivotHandle : public BaseHandle {
public:
  PivotHandle();

  void BeginUpdate(float x, float y, float width, float height) override;
  void Update(float x, float y, float width, float height) override;
  void SetVisibility(short axis, short mask) override;

protected:
  void _UpdateTargets(bool interacting) override;

private:
  float _radius;
  float _height;
};
*/

class Scene;
class BrushHandle : public BaseHandle {
public:
enum Mode {
    ADD,
    REMOVE,
    MOVE,
    SCALE,
    BRUSH
  };
  BrushHandle();

  void Draw(float width, float height) override;
  void BeginUpdate(float x, float y, float width, float height) override;
  void EndUpdate() override;
  short Pick(float x, float y, float width, float height) override;
  void Update(float x, float y, float width, float height) override;
  short Select(float x, float y, float width, float height, bool lock) override;

protected:
  void _UpdateTargets(bool interacting) override {};

private:
  void _BuildStroke(bool replace);

  float                         _minRadius;
  float                         _maxRadius;
  HandleTargetGeometryDescList  _geometries;
  std::vector<GfVec3f>          _path;
  GfVec4f                       _color;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_HANDLE_H