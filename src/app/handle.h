#ifndef AMN_APPLICATION_HANDLE_H
#define AMN_APPLICATION_HANDLE_H
#pragma once

#include "../common.h"
#include "../app/shape.h"
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/vec4f.h>
#include <pxr/base/gf/plane.h>

AMN_NAMESPACE_OPEN_SCOPE

static const pxr::GfMatrix4f HANDLE_X_MATRIX = {
  0.f, 1.f, 0.f, 0.f,
  1.f, 0.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 0.f, 0.f, 1.f};

static const pxr::GfMatrix4f HANDLE_Y_MATRIX = {
  1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 0.f, 0.f, 1.f};

static const pxr::GfMatrix4f HANDLE_Z_MATRIX = {
  1.f, 0.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 0.f, 1.f};

static const pxr::GfVec4f HANDLE_X_COLOR =  {1.f, 0.25f, 0.5f, 1.f};
static const pxr::GfVec4f HANDLE_Y_COLOR =  {0.5f, 1.f, 0.25f, 1.f};
static const pxr::GfVec4f HANDLE_Z_COLOR =  {0.25f, 0.5f, 1.f, 1.f}; 
static const pxr::GfVec4f HANDLE_HELP_COLOR = {0.66f, 0.66f, 0.66f, 1.f};
static const pxr::GfVec4f HANDLE_HOVERED_COLOR = {1.f, 0.5f, 0.0f, 1.f};
static const pxr::GfVec4f HANDLE_ACTIVE_COLOR = {1.f, 0.75f, 0.25f, 1.f};
static const pxr::GfVec4f HANDLE_MASK_COLOR = {0.f, 0.f, 0.f, 0.f};


class Camera;
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
    AXIS_LAST
  };

  enum PlaneNormal {
    NORMAL_X,
    NORMAL_Y,
    NORMAL_Z,
    NORMAL_CAMERA
  };

  /*
  struct Component {
    short axis;
    size_t startIdx;
    size_t endIdx;
    bool active;
    bool hovered;
    const pxr::GfMatrix4f* offset;
    const pxr::GfVec4f* color;

    Component(short axis, size_t start, size_t end, const pxr::GfVec4f* color,
      const pxr::GfMatrix4f* m) 
      : axis(axis)
      , startIdx(start)
      , endIdx(end)
      , color(color)
      , offset(m)
      , active(false)
      , hovered(false){};
  };
  */

  BaseHandle() 
    : _activeAxis(AXIS_NONE)
    , _activeNormal(NORMAL_CAMERA)
    , _camera(NULL) 
    , _position(pxr::GfVec3d(0.f))
    , _rotation(pxr::GfRotation())
    , _scale(pxr::GfVec3d(1.f))
    , _viewPlaneMatrix(pxr::GfMatrix4f(1.f)) {};

  void SetActiveAxis(short axis);
  void SetCamera(Camera* camera) {_camera = camera;};
  void SetMatrixFromSRT();
  void SetSRTFromMatrix();
  void Resize();
  void AddComponent(Shape::Component& component);
  void AddXYZComponents(Shape::Component& component);
  void AddYZXZXYComponents(Shape::Component& component);
  const pxr::GfVec4f& GetColor(const Shape::Component& comp);
  void UpdatePickingPlane(short axis=NORMAL_CAMERA);

  virtual void Draw() = 0;
  virtual short Pick(float x, float y, float width, float height) = 0;
  virtual void Update(float x, float y) = 0;

protected:
  // geomeyry
  Shape _shape;

  // state
  short _activeNormal;
  short _activeAxis;
  short _hoveredAxis;
  float _size;
  float _distance;
  bool _interacting;

  // data
  pxr::GfVec3d _scale;
  pxr::GfVec3d _position;
  pxr::GfVec3d _offset;
  pxr::GfRotation _rotation;
  pxr::GfMatrix4f _matrix;
  pxr::GfPlane _plane;
  pxr::GfMatrix4f _viewPlaneMatrix;
  pxr::GfMatrix4f _sizeMatrix;

  Camera* _camera;

};

class ScaleHandle : public BaseHandle {
public:
  ScaleHandle();

  void Draw() override;
  short Pick(float x, float y, float width, float height) override;
  void Update(float x, float y) override;
private:
};

class RotateHandle : public BaseHandle {
public:
  RotateHandle();

  void Draw() override;
  short Pick(float x, float y, float width, float height) override;
  void Update(float x, float y) override;
};

class TranslateHandle : public BaseHandle {
public:
  TranslateHandle();

  void Draw() override;
  short Pick(float x, float y, float width, float height) override;
  void Update(float x, float y) override;
private:
  float _radius;
  float _height;
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_APPLICATION_HANDLE_H