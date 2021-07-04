#include "handle.h"
#include "camera.h"
#include "selection.h"
#include "application.h"
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/transform.h>
#include <pxr/usd/usdGeom/boundable.h>
#include <pxr/usd/usdGeom/xformable.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usdGeom/bboxCache.h>

AMN_NAMESPACE_OPEN_SCOPE

void BaseHandle::ComputeSizeMatrix(float width, float height)
{
  if(_camera) {
    const pxr::GfVec3d delta = _camera->GetPosition() - _position;
    _distance = (float)delta.GetLength();
    if(width > height) {
      _size = _distance * (float)HANDLE_SIZE / width;
    } else {
      _size = _distance * (float)HANDLE_SIZE / height;
    }
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
  if (component.index == AXIS_CAMERA)
    component.SetFlag(Shape::FLAT, true);
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

void BaseHandle::ComputeViewPlaneMatrix()
{
  _viewPlaneMatrix = 
    _sizeMatrix * pxr::GfMatrix4f(_camera->GetTransform());
  _viewPlaneMatrix[3][0] = _position[0];
  _viewPlaneMatrix[3][1] = _position[1];
  _viewPlaneMatrix[3][2] = _position[2];
  _viewPlaneMatrix[3][3] = 1;
}

void BaseHandle::ResetSelection()
{
  Application* app = AMN_APPLICATION;
  
  Selection* selection = app->GetSelection();
  pxr::UsdStageRefPtr stage = app->GetStage();
  float activeTime = app->GetTime().GetActiveTime();
  _xformCache.SetTime(activeTime);

  _targets.clear();
  for(size_t i=0; i<selection->GetNumSelectedItems(); ++i ) {
    const SelectionItem& item = selection->GetItem(i);
    if(stage->GetPrimAtPath(item.path).IsA<pxr::UsdGeomXformable>()) {
      pxr::GfMatrix4f world(_xformCache.GetLocalToWorldTransform(
        stage->GetPrimAtPath(item.path)));
      _targets.push_back({item.path, world, pxr::GfMatrix4f(1.f)});
    }
  }

  _ComputeCOGMatrix(app->GetStage());
  SetSRTFromMatrix();

  pxr::GfMatrix4f invMatrix = _matrix.GetInverse();
  for(auto& target: _targets) {
    target.offset = target.base * invMatrix;
  }
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
 _plane.Set(
    _camera->GetViewPlaneNormal(),
    pxr::GfVec3d(_position)
 );
}

const pxr::GfVec4f& BaseHandle::GetColor(const Shape::Component& comp)
{
  if(_interacting) {
    if(comp.flags &Shape::SELECTED) {
      return HANDLE_ACTIVE_COLOR;
    } else {
      return HANDLE_HELP_COLOR;
    }
  } else {
    if(comp.flags & Shape::SELECTED) {
      return HANDLE_ACTIVE_COLOR;
    } else if(comp.flags & Shape::HOVERED && !(comp.flags & Shape::MASK)) {
      return HANDLE_HOVERED_COLOR;
    } else {
      return comp.color;
    }
  }
}

short BaseHandle::Select(float x, float y, float width, float height, 
  bool lock)
{
  pxr::GfRay ray(
    _camera->GetPosition(), 
    _camera->GetRayDirection(x, y, width, height));

  pxr::GfMatrix4f m = _sizeMatrix * _matrix;
  size_t hovered = _shape.Intersect(ray, m, _viewPlaneMatrix);

  if(hovered) {
    SetActiveAxis(hovered);
    UpdatePickingPlane(hovered);
    ResetSelection();
  }
  _shape.UpdateComponents(hovered, hovered);
  return hovered;
}

short BaseHandle::Pick(float x, float y, float width, float height)
{
  pxr::GfRay ray(
    _camera->GetPosition(), 
    _camera->GetRayDirection(x, y, width, height));

  pxr::GfMatrix4f m = _sizeMatrix * _matrix;
  size_t hovered = _shape.Intersect(ray, m, _viewPlaneMatrix);
  SetHoveredAxis(hovered);
  _shape.UpdateComponents(hovered, AXIS_NONE);
  //_shape.UpdateVisibility(_matrix, pxr::GfVec3f(_camera->GetViewPlaneNormal()));
  return hovered;
}

void BaseHandle::Draw(float width, float height) 
{
  ComputeSizeMatrix(width, height);
  ComputeViewPlaneMatrix();
  pxr::GfMatrix4f m = _sizeMatrix * _matrix;
  GLint vao;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
  _shape.Bind(SHAPE_PROGRAM);
  
  for(size_t i=0; i < _shape.GetNumComponents(); ++i) {
    const Shape::Component& component = _shape.GetComponent(i);
    if(component.GetFlag(Shape::VISIBLE)) {
      if(component.index == AXIS_CAMERA) {
        _shape.DrawComponent(i, _viewPlaneMatrix, GetColor(component));
      } else {
        _shape.DrawComponent(i, component.parentMatrix * m, 
          GetColor(component));
      }
    }
  }
  _shape.Unbind();
  glBindVertexArray(vao);
}

void BaseHandle::Setup()
{
  _shape.Setup();
  SetMatrixFromSRT();
}

void BaseHandle::Update(float x, float y, float width, float height)
{
  if(_interacting) {
    pxr::GfRay ray(
      _camera->GetPosition(), 
      _camera->GetRayDirection(x, y, width, height));

    double distance;
    bool frontFacing;
    if(ray.Intersect(_plane, &distance, &frontFacing)) {
      pxr::GfVec3f offset = 
        pxr::GfVec3f(ray.GetPoint(distance) - pxr::GfVec3d(_position));
      _position = pxr::GfVec3f(ray.GetPoint(distance)) + _offset;
      SetMatrixFromSRT();
    }
  }
}

void BaseHandle::_ComputeCOGMatrix(pxr::UsdStageRefPtr stage)
{
  Application* app = AMN_APPLICATION;
  float activeTime = app->GetTime().GetActiveTime();
  pxr::TfTokenVector purposes = { pxr::UsdGeomTokens->default_ };
  pxr::UsdGeomBBoxCache bboxCache(
    activeTime, purposes, false, false);
  pxr::GfVec3f accumPosition(0.f);
  pxr::GfQuatf accumRotation(1.f);
  pxr::GfVec3f accumSCale(1.f);
  pxr::GfRange3d accumulatedRange;
  size_t numPrims = 0;
  for (auto& target: _targets) {
    const auto& prim(stage->GetPrimAtPath(target.path));
    if(prim.IsValid()) {
      if(prim.IsA<pxr::UsdGeomBoundable>()) {
        pxr::GfBBox3d bbox = bboxCache.ComputeWorldBound(prim);
        const pxr::GfRange3d& range = bbox.GetRange();
        accumulatedRange.UnionWith(range);
      }

      if(prim.IsA<pxr::UsdGeomXformable>()) {
        bool resetsXformStack = false;
        pxr::GfMatrix4f m(_xformCache.GetLocalToWorldTransform(prim));

        accumPosition += pxr::GfVec3f(
          m[3][0],
          m[3][1],
          m[3][2]);
        accumRotation *= m.ExtractRotationQuat();
        ++numPrims;
      }
    }
  }
  if(numPrims) {
     accumPosition *= 1.f / (float)numPrims;
    accumRotation.Normalize();

    _matrix.SetIdentity();
   
    _matrix.SetRotate(accumRotation);
    _matrix[3][0] = accumPosition[0];
    _matrix[3][1] = accumPosition[1];
    _matrix[3][2] = accumPosition[2];
   
  }
}

pxr::GfVec3f BaseHandle::_ConstraintPointToAxis(const pxr::GfVec3f& point, 
  short axis)
{
  pxr::GfVec3f localPoint(_startMatrix.GetInverse().Transform(point));
  switch(_activeAxis) {
    case AXIS_X:
      localPoint[1] = localPoint[2] = 0.f;
      break;
    case AXIS_Y:
      localPoint[0] = localPoint[2] = 0.f;
      break;
    case AXIS_Z:
      localPoint[0] = localPoint[1] = 0.f;
      break;
    case AXIS_XY:
      localPoint[2] = 0.f;
      break;
    case AXIS_XZ:
      localPoint[1] = 0.f;
      break;
    case AXIS_YZ:
      localPoint[0] = 0.f;
      break;
    case AXIS_CAMERA:
      break;
  }
  return _startMatrix.Transform(localPoint);
}

pxr::GfVec3f BaseHandle::_ConstraintPointToPlane(const pxr::GfVec3f& point, 
  short axis)
{
  return pxr::GfVec3f(0.f);
}

void BaseHandle::_UpdateTargets()
{
  
  Application* app = AMN_APPLICATION;
  pxr::UsdStageRefPtr stage = app->GetStage();
  float activeTime = app->GetTime().GetActiveTime();
  Selection* selection = app->GetSelection();
  for(auto& target: _targets) {
    
    pxr::UsdGeomXformable xformable(stage->GetPrimAtPath(target.path));
    pxr::GfMatrix4d invParentMatrix = 
      _xformCache.GetParentToWorldTransform(xformable.GetPrim()).GetInverse();

    bool resetXformOpExists;
    std::vector<pxr::UsdGeomXformOp> xformOps = xformable.GetOrderedXformOps(&resetXformOpExists);
    if (!xformOps.size()) {
      pxr::UsdGeomXformOp xformOp = xformable.AddTransformOp(pxr::UsdGeomXformOp::PrecisionDouble);
      //xformOp.Set(pxr::VtValue(usdMatrix));
      xformOp.Set(pxr::GfMatrix4d(target.offset * _matrix) * invParentMatrix);
    }
    else {
      //xformOps[0].Set(pxr::VtValue(usdMatrix));
      xformOps[0].Set(pxr::GfMatrix4d(target.offset * _matrix) * invParentMatrix);
    }
  }
}

void BaseHandle::BeginUpdate(float x, float y, float width, float height)
{
  Application* app = AMN_APPLICATION;
  pxr::GfRay ray(
    _camera->GetPosition(), 
    _camera->GetRayDirection(x, y, width, height));

  double distance;
  bool frontFacing;
  _startMatrix = _matrix;
  if(ray.Intersect(_plane, &distance, &frontFacing)) {
    _offset = pxr::GfVec3f(pxr::GfVec3d(_position) - ray.GetPoint(distance));
  }
  float activeTime = app->GetTime().GetActiveTime();
  _xformCache.SetTime(activeTime);
  _interacting = true;
}

void BaseHandle::EndUpdate()
{
  _interacting = false;
}

TranslateHandle::TranslateHandle()
 : BaseHandle()
 , _radius(0.05f)
 , _height(1.f)
{  
  Shape::Component center = _shape.AddBox(
    AXIS_CAMERA, _radius * 2.5f, _radius * 2.5f, 0.01f, HANDLE_HELP_COLOR, {
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f});
  AddComponent(center);

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
    AXIS_YZ, size, size * 0.12f, size, HANDLE_X_COLOR, {
    1.f, 0.f, 0.f, 0.f,
    0.f, -1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    _radius * 2.f + _height * 0.5f, 0.f, _radius * 2.f + _height * 0.5f, 1.f});
  AddYZXZXYComponents(box);
}

void TranslateHandle::BeginUpdate(float x, float y, float width, float height)
{
  pxr::GfRay ray(
    _camera->GetPosition(), 
    _camera->GetRayDirection(x, y, width, height));

  double distance;
  bool frontFacing;
  _startMatrix = _matrix;
  if(ray.Intersect(_plane, &distance, &frontFacing)) {
    if(_activeAxis == AXIS_CAMERA) {
      _offset = pxr::GfVec3f(pxr::GfVec3d(_position) - ray.GetPoint(distance));
    } else {
      _offset = pxr::GfVec3f(pxr::GfVec3d(_position) - 
        _ConstraintPointToAxis(pxr::GfVec3f(ray.GetPoint(distance)), _activeAxis));
    }
  }
  _interacting = true;
}

void TranslateHandle::Update(float x, float y, float width, float height)
{
  if(_interacting) {
    pxr::GfRay ray(
      _camera->GetPosition(), 
      _camera->GetRayDirection(x, y, width, height));

    double distance;
    bool frontFacing;
    if(ray.Intersect(_plane, &distance, &frontFacing)) {
      if(_activeAxis == AXIS_CAMERA) {
        _position = pxr::GfVec3f(ray.GetPoint(distance)) + _offset;
      } else {
        _position = 
          _ConstraintPointToAxis(pxr::GfVec3f(ray.GetPoint(distance)),
            _activeAxis) + _offset;
      }
      
      SetMatrixFromSRT();
      _UpdateTargets();
    }
  }
}

RotateHandle::RotateHandle()
 : BaseHandle()
 , _radius(0.75f)
{
  float section = 0.02f;
  Shape::Component mask = _shape.AddDisc(
    AXIS_CAMERA, _radius - section, 32, HANDLE_MASK_COLOR, {
      1.f, 0.f, 0.f, 0.f,
      0.f, 0.f, 1.f, 0.f,
      0.f, -1.f, 0.f, 0.f,
      0.f, 0.f, 0.f, 1.f }
  );
  mask.SetFlag(Shape::MASK, true);
  AddComponent(mask);
  /*
  Shape::Component torus = _shape.AddTorus(
    AXIS_X, _radius, section, 32, 16, HANDLE_X_COLOR);
  */
  Shape::Component axis = _shape.AddTube(
    AXIS_X, _radius + section, _radius, 0.05f, 32, 2, HANDLE_X_COLOR);
  AddXYZComponents(axis);
  
  Shape::Component plane = _shape.AddRing(
    AXIS_CAMERA, _radius * 1.4f, 0.025f, 32, HANDLE_HELP_COLOR, {
      1.f, 0.f, 0.f, 0.f,
      0.f, 0.f, 1.f, 0.f,
      0.f, -1.f, 0.f, 0.f,
      0.f, 0.f, 0.f, 1.f
    });
  AddComponent(plane);

  Shape::Component help = _help.AddDisc(
    AXIS_Y, _radius, 45.f, 128.f, 16, HANDLE_HELP_COLOR, {
      1.f, 0.f, 0.f, 0.f,
      0.f, 0.f, 1.f, 0.f,
      0.f, -1.f, 0.f, 0.f,
      0.f, 0.f, 0.f, 1.f
    });
}

void RotateHandle::Update(float x, float y, float width, float height)
{

}

ScaleHandle::ScaleHandle()
  : BaseHandle()
{
  Shape::Component cylinder = _shape.AddCylinder(
    AXIS_X, 0.02f, 1.f, 16, 2, HANDLE_X_COLOR);
  AddXYZComponents(cylinder);

  Shape::Component box = _shape.AddBox(
    AXIS_X, 0.2f, 0.2f, 0.2f, HANDLE_X_COLOR, {
      1.f, 0.f, 0.f, 0.f,
      0.f, 1.F, 0.f, 0.f,
      0.f, 0.f, 1.f, 0.f,
      0.f, 1.f, 0.f, 1.f });
  AddXYZComponents(box);
  
  // dummy xfos
  std::vector<pxr::GfMatrix4f> xfos = {
    pxr::GfMatrix4f(1.f).SetTranslate(pxr::GfVec3f(-1.f, 0.f, 0.f)),
    pxr::GfMatrix4f(1.f).SetTranslate(pxr::GfVec3f(-0.5f, 1.f, 0.f)),
    pxr::GfMatrix4f(1.f).SetTranslate(pxr::GfVec3f(0.f, 2.f, 0.f)),
    pxr::GfMatrix4f(1.f).SetTranslate(pxr::GfVec3f(0.5f, 1.f, 0.f)),
    pxr::GfMatrix4f(1.f).SetTranslate(pxr::GfVec3f(1.f, 0.f, 0.f)),
    pxr::GfMatrix4f(1.f).SetTranslate(pxr::GfVec3f(0.f, -1.f, 1.f))
  };

  // dummy profile
  std::vector<pxr::GfVec3f> profile = {
    pxr::GfVec3f(-0.1f, 0.f, -0.1f),
    pxr::GfVec3f(0.1f, 0.f, -0.1f),
    pxr::GfVec3f(0.1f, 0.f, 0.1f),
    pxr::GfVec3f(-0.1f, 0.f, 0.1f)
  };

  Shape::Component extrude = _shape.AddExtrusion(
    AXIS_CAMERA, xfos, profile, HANDLE_X_COLOR
  );
  /*
  Shape::AddExtrusion(short index, 
  const std::vector<pxr::GfMatrix4f>& xfos,
  const std::vector<pxr::GfVec3f>& profile, 
  const pxr::GfVec4f& color,
  const pxr::GfMatrix4f& m)
  */
  AddComponent(extrude);
}

void ScaleHandle::Update(float x, float y, float width, float height)
{

}

BrushHandle::BrushHandle()
  : BaseHandle()
{
  
}

void BrushHandle::Update(float x, float y, float width, float height)
{

}


AMN_NAMESPACE_CLOSE_SCOPE