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

void BaseHandle::AddXComponent(Shape::Component& component)
{
  component.index = AXIS_X;
  component.color = HANDLE_X_COLOR;
  component.parentMatrix = HANDLE_X_MATRIX;
  _shape.AddComponent(component);
}

void BaseHandle::AddYComponent(Shape::Component& component)
{
  component.index = AXIS_Y;
  component.color = HANDLE_Y_COLOR;
  component.parentMatrix =  HANDLE_Y_MATRIX;
  _shape.AddComponent(component);
}

void BaseHandle::AddZComponent(Shape::Component& component)
{
  component.index = AXIS_Z;
  component.color = HANDLE_Z_COLOR;
  component.parentMatrix =  HANDLE_Z_MATRIX;
  _shape.AddComponent(component);
}

void BaseHandle::AddXYZComponents(Shape::Component& component)
{
  AddXComponent(component);
  AddYComponent(component);
  AddZComponent(component);
}

void BaseHandle::AddYZXZXYComponents(Shape::Component& component)
{
  component.index = AXIS_YZ;
  component.color = HANDLE_X_COLOR;
  component.parentMatrix = HANDLE_X_MATRIX;
  _shape.AddComponent(component);
  component.index = AXIS_XZ;
  component.color = HANDLE_Y_COLOR;
  component.parentMatrix =  HANDLE_Y_MATRIX;
  _shape.AddComponent(component);
  component.index = AXIS_XY;
  component.color = HANDLE_Z_COLOR;
  component.parentMatrix =  HANDLE_Z_MATRIX;
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
}

void BaseHandle::SetHoveredAxis(short axis)
{
  _hoveredAxis = axis;
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
  if(_interacting) {
    if(comp.active) {
      return HANDLE_ACTIVE_COLOR;
    } else {
      return HANDLE_HELP_COLOR;
    }
  } else {
    if(comp.active) {
      return HANDLE_ACTIVE_COLOR;
    } else if(comp.hovered) {
      return HANDLE_HOVERED_COLOR;
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
  
  Shape::Component sphere = _shape.AddIcoSphere(
    AXIS_CAMERA, _radius * 2.f, 1, HANDLE_HELP_COLOR);
  AddComponent(sphere);
  /*
  Shape::Component sphere = _shape.AddSphere(
   AXIS_CAMERA, 0.2, 12, 12);
  _shape.AddComponent(sphere);
*/
/*
  Shape::Component cylinder = _shape.AddCylinder(
    AXIS_X, _radius * 0.2f, _height, 16, 2, HANDLE_X_COLOR, {
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, _radius * 4.f, 0.f, 1.f});
  AddXYZComponents(cylinder);
  
  Shape::Component cone = _shape.AddCone(
    AXIS_X, _radius, _height * 0.2f, 8, HANDLE_X_COLOR, {
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, _radius * 4.f + _height, 0.f, 1.f});
  AddXYZComponents(cone);
  
  float size = _height * 0.2f;
  Shape::Component box = _shape.AddBox(
    AXIS_YZ, size, _radius * 0.1f, size, HANDLE_X_COLOR, {
    1.f, 0.f, 0.f, 0.f,
    0.f, -1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    _radius * 2.f + _height * 0.5f, 0.f, _radius * 2.f + _height * 0.5f, 1.f});
  AddYZXZXYComponents(box);
  
  Shape::Component torus = _shape.AddTorus(
    AXIS_Y, 0.5f, 0.05f, 32, 12, HANDLE_Y_COLOR, {
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f});
  AddXComponent(torus);
  */

  Shape::Component disc = _shape.AddDisc(
    AXIS_X, 0.5f, 90.f, 360.f, 8, HANDLE_X_COLOR, {
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 1.f, 0.f, 1.f});
  AddXYZComponents(disc);

  _shape.Setup();

  _position = pxr::GfVec3d(0, 3, 0);
  _rotation = pxr::GfRotation(
    pxr::GfVec3d(0.f, 1.f, 0.f), 45.f);
  _scale = pxr::GfVec3d(1.0);
  SetMatrixFromSRT();
}

short BaseHandle::Select(float x, float y, float width, float height, 
  bool lock)
{
  size_t hovered = Pick(x, y, width, height);
  _shape.UpdateComponents(hovered, hovered);
  return hovered;
}

short BaseHandle::Pick(float x, float y, float width, float height)
{
  pxr::GfRay ray(
    _camera->GetPosition(), 
    _camera->GetRayDirection(x, y, width, height));

  pxr::GfMatrix4f m = _sizeMatrix * _matrix;
  size_t hovered = _shape.Intersect(ray, m);
  SetHoveredAxis(hovered);
  _shape.UpdateComponents(hovered, _activeAxis);
  return hovered;
}

void BaseHandle::Draw() 
{
  pxr::GfMatrix4f m = _sizeMatrix * _matrix;
  GLint vao;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
  _shape.Bind(SHAPE_PROGRAM);
  for(size_t i=0; i < _shape.GetNumComponents(); ++i) {
    const Shape::Component& component = _shape.GetComponent(i);
    if(component.visible)
      _shape.DrawComponent(i, component.parentMatrix * m, GetColor(component));
  }
  _shape.Unbind();
  glBindVertexArray(vao);
}

void BaseHandle::Update(float x, float y)
{

}

AMN_NAMESPACE_CLOSE_SCOPE