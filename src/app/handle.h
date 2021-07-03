#ifndef AMN_APPLICATION_HANDLE_H
#define AMN_APPLICATION_HANDLE_H
#pragma once

#include "../common.h"
#include "../geometry/shape.h"
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/vec4f.h>
#include <pxr/base/gf/plane.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/stage.h>

AMN_NAMESPACE_OPEN_SCOPE

static const pxr::GfMatrix4f HANDLE_X_MATRIX = {
  0.f, 1.f, 0.f, 0.f,
  1.f, 0.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 0.f, 0.f, 1.f
};

static const pxr::GfMatrix4f HANDLE_Y_MATRIX = {
  1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 0.f, 0.f, 1.f
};

static const pxr::GfMatrix4f HANDLE_Z_MATRIX = {
  1.f, 0.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 0.f, 1.f
};

static const pxr::GfVec4f HANDLE_X_COLOR =  {1.f, 0.25f, 0.5f, 1.f};
static const pxr::GfVec4f HANDLE_Y_COLOR =  {0.5f, 1.f, 0.25f, 1.f};
static const pxr::GfVec4f HANDLE_Z_COLOR =  {0.25f, 0.5f, 1.f, 1.f}; 
static const pxr::GfVec4f HANDLE_HELP_COLOR = {0.66f, 0.66f, 0.66f, 0.5f};
static const pxr::GfVec4f HANDLE_HOVERED_COLOR = {1.f, 0.5f, 0.0f, 1.f};
static const pxr::GfVec4f HANDLE_ACTIVE_COLOR = {1.f, 0.75f, 0.25f, 1.f};
static const pxr::GfVec4f HANDLE_MASK_COLOR = {0.f, 0.f, 0.f, 0.f};
static float HANDLE_SIZE = 100.f;

class Camera;

struct TargetDesc {
  pxr::SdfPath path;
  pxr::GfMatrix4f base;
  pxr::GfMatrix4f offset;
};

class BaseHandle {
public:
  enum State {
    STATE_DEFAULT,
    STATE_HOVERED,
    STATE_ACTIVE
  };

  enum Axis {
    AXIS_NONE,
    AXIS_X,
    AXIS_Y,
    AXIS_Z,
    AXIS_XY,
    AXIS_XZ,
    AXIS_YZ,
    AXIS_CAMERA,
    AXIS_NORMAL,
    AXIS_LAST
  };

  enum PlaneNormal {
    NORMAL_X,
    NORMAL_Y,
    NORMAL_Z,
    NORMAL_CAMERA
  };

  BaseHandle() 
    : _activeAxis(AXIS_NONE)
    , _hoveredAxis(AXIS_NONE)
    , _activeNormal(NORMAL_CAMERA)
    , _camera(NULL)
    , _interacting(false)
    , _position(pxr::GfVec3d(0.f))
    , _rotation(pxr::GfRotation())
    , _scale(pxr::GfVec3d(1.f))
    , _matrix(pxr::GfMatrix4f(1.f))
    , _startMatrix(pxr::GfMatrix4f(1.f))
    , _viewPlaneMatrix(pxr::GfMatrix4f(1.f)) {};

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
  void UpdatePickingPlane(short axis=NORMAL_CAMERA);
  void ComputeSizeMatrix(float width, float height);
  void ComputeViewPlaneMatrix();

  const pxr::GfVec4f& GetColor(const Shape::Component& comp);
  short GetActiveAxis(){return _activeAxis;};

  virtual void Setup();
  virtual void Draw(float width, float height);
  virtual short Select(float x, float y, float width, float height, bool lock);
  virtual short Pick(float x, float y, float width, float height);
  virtual void BeginUpdate(float x, float y, float width, float height);
  virtual void Update(float x, float y, float width, float height);
  virtual void EndUpdate();

  virtual void _ComputeCOGMatrix(pxr::UsdStageRefPtr stage);
  virtual void _UpdateTargets();
  pxr::GfVec3f _ConstraintPointToAxis(const pxr::GfVec3f& point, short axis);
  pxr::GfVec3f _ConstraintPointToPlane(const pxr::GfVec3f& point, short axis);

protected:
  // targets
  std::vector<TargetDesc> _targets;
  
  // geometry
  Shape _shape;

  // viewport
  Camera* _camera;

  // state
  short _activeNormal;
  short _activeAxis;
  short _hoveredAxis;
  short _lastActiveAxis;
  float _size;
  float _distance;
  bool _interacting;

  // data
  pxr::GfVec3d _scale;
  pxr::GfVec3d _position;
  pxr::GfVec3d _offset;
  pxr::GfRotation _rotation;
  pxr::GfMatrix4f _matrix;
  pxr::GfMatrix4f _startMatrix;
  pxr::GfPlane _plane;
  pxr::GfMatrix4f _viewPlaneMatrix;
  pxr::GfMatrix4f _sizeMatrix;
};

class ScaleHandle : public BaseHandle {
public:
  ScaleHandle();

  void Update(float x, float y, float width, float height) override;

private:
};

class RotateHandle : public BaseHandle {
public:
  RotateHandle();

  void Update(float x, float y, float width, float height) override;

private:
  float _radius;
};

class TranslateHandle : public BaseHandle {
public:
/*static bool VISIBILITIES[8][10] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  } */
  TranslateHandle();

  void BeginUpdate(float x, float y, float width, float height) override;
  void Update(float x, float y, float width, float height) override;

private:
  float _radius;
  float _height;
};

class BrushHandle : public BaseHandle {
public:
  BrushHandle();

  //void BeginUpdate(float x, float y, float width, float height) override;
  void Update(float x, float y, float width, float height) override;

private:
  float _minRadius;
  float _maxRadius;
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_APPLICATION_HANDLE_H