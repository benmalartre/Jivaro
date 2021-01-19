#include "handle.h"
#include "camera.h"
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/transform.h>

AMN_NAMESPACE_OPEN_SCOPE

void BaseHandle::Resize()
{
  if(_camera) {
    pxr::GfVec3d delta = _camera->GetPosition() - _position;
    _distance = (float)delta.GetLength();
    _size = _distance * 0.06f * (_camera->GetFov() * 2 * DEGREES_TO_RADIANS);
    _sizeMatrix = {
      _size, 0.f, 0.f, 0.f,
      0.f, _size, 0.f, 0.f,
      0.f, 0.f, _size, 0.f,
      0.f, 0.f, 0.f, 1.f
    };
  }
}

void BaseHandle::AddComponent(Shape::Component& component)
{
  _shape.AddComponent(component);
}

void BaseHandle::AddXYZComponents(Shape::Component& component)
{
  component.index = AXIS_X;
  component.color = HANDLE_X_COLOR;
  component.offset = HANDLE_X_MATRIX;
  _shape.AddComponent(component);
  component.index = AXIS_Y;
  component.color = HANDLE_Y_COLOR;
  component.offset =  HANDLE_Y_MATRIX;
  _shape.AddComponent(component);
  component.index = AXIS_Z;
  component.color = HANDLE_Z_COLOR;
  component.offset =  HANDLE_Z_MATRIX;
  _shape.AddComponent(component);
}

void BaseHandle::AddYZXZXYComponents(Shape::Component& component)
{
  component.index = AXIS_YZ;
  component.color = HANDLE_X_COLOR;
  component.offset = HANDLE_X_MATRIX;
  _shape.AddComponent(component);
  component.index = AXIS_XZ;
  component.color = HANDLE_Y_COLOR;
  component.offset =  HANDLE_Y_MATRIX;
  _shape.AddComponent(component);
  component.index = AXIS_XY;
  component.color = HANDLE_Z_COLOR;
  component.offset =  HANDLE_Z_MATRIX;
  _shape.AddComponent(component);
}

void BaseHandle::SetMatrixFromSRT()
{
  pxr::GfTransform transform;
  transform.SetScale(_scale);
  transform.SetRotation(_rotation);
  transform.SetTranslation(_position);

  _matrix = pxr::GfMatrix4f(transform.GetMatrix());
}

void BaseHandle::SetSRTFromMatrix()
{
  pxr::GfTransform transform;
  transform.SetMatrix(pxr::GfMatrix4d(_matrix));
  _scale = transform.GetScale();
  _rotation = transform.GetRotation();
  _position = transform.GetTranslation();
}

void BaseHandle::SetActiveAxis(short axis)
{
  _activeAxis = axis;
  /*
  memset(&_axisState, STATE_DEFAULT, AXIS_LAST * sizeof(short));
  _axisState[_activeAxis] = STATE_ACTIVE;
  if(!_interacting) {
    _axisState[_hoveredAxis] = STATE_HOVERED;
  }
  */
}

void BaseHandle::UpdatePickingPlane(short axis)
{
  switch(axis) {
    case NORMAL_X:
      _plane.Set(
        pxr::GfVec3d(1, 0, 0),
        pxr::GfVec3d(_position));
      break;

    case NORMAL_Y:
      _plane.Set(
        pxr::GfVec3d(0, 1, 0),
        pxr::GfVec3d(_position));
      break;
    
    case NORMAL_Z:
      _plane.Set(
        pxr::GfVec3d(0, 0, 1),
        pxr::GfVec3d(_position));
      break;

    case NORMAL_CAMERA:
    default:
      _plane.Set(
        _camera->GetViewPlaneNormal(),
        pxr::GfVec3d(_position));
      break;
  }
}

const pxr::GfVec4f& BaseHandle::GetColor(const Shape::Component& comp)
{
  if(!_interacting) {
    if(comp.hovered) {
      return HANDLE_HOVERED_COLOR;
    } else if(comp.active) {
      return HANDLE_ACTIVE_COLOR;
    } else {
      return comp.color;
    }
  } else {
    if(comp.hovered) {
      return HANDLE_HOVERED_COLOR;
    } else if(comp.active) {
      return HANDLE_ACTIVE_COLOR;
    } else {
      return comp.color;
    }
  }
}

TranslateHandle::TranslateHandle()
 : BaseHandle()
 , _radius(0.05f)
 , _height(1.f)
{  
  size_t baseIdx = 0;
  
  Shape::Component cylinder = _shape.AddCylinder(
    AXIS_X, _radius * 0.2f, _height, 16, 2, HANDLE_X_COLOR, {
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, _radius * 4.f + _height * 0.5f, 0.f, 1.f});
  AddXYZComponents(cylinder);
    
  Shape::Component cone = _shape.AddCone(
    AXIS_X, _radius, _height * 0.2f, 8, HANDLE_X_COLOR, {
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, _radius * 4.f + _height, 0.f, 1.f});
  AddXYZComponents(cone);

  float size = _height * 0.1f;
  Shape::Component box = _shape.AddBox(
    AXIS_YZ, size, _radius * 0.1f, size, HANDLE_X_COLOR, {
    1.f, 0.f, 0.f, 0.f,
    0.f, -1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    _height * 0.5f, 0.f, _height * 0.5f, 1.f});
  AddYZXZXYComponents(box);

  Shape::Component sphere = _shape.AddIcoSphere(
    AXIS_CAMERA, _radius * 2.f, 1, HANDLE_HELP_COLOR);
  AddComponent(sphere);
  
  _shape.Setup();

  _position = pxr::GfVec3d(0, 3, 0);
  _rotation = pxr::GfRotation(
    pxr::GfVec3d(0.f, 1.f, 0.f), 45.f);
  _scale = pxr::GfVec3d(1.0);
  SetMatrixFromSRT();
}

short TranslateHandle::Pick(float x, float y, float width, float height)
{
  double enterDistance, exitDistance;
  pxr::GfRay ray(
    _camera->GetPosition(), 
    _camera->GetRayDirection(x, y, width, height));

  pxr::GfMatrix4f m = _sizeMatrix * _matrix;
  size_t hovered = _shape.Intersect(ray, m);
  SetActiveAxis(hovered);
  _shape.UpdateComponents(hovered, AXIS_NONE);
  return hovered;
}

void TranslateHandle::Draw() 
{
  pxr::GfMatrix4f m = _sizeMatrix * _matrix;
  
  for(size_t i=0; i < _shape.GetNumComponents(); ++i) {
    const Shape::Component& component = _shape.GetComponent(i);
    _shape.Draw(component.offset * m, GetColor(component), 
      component.start, component.end);
  }
}

void TranslateHandle::Update(float x, float y)
{

}

AMN_NAMESPACE_CLOSE_SCOPE