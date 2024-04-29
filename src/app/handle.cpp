#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/transform.h>
#include <pxr/base/gf/matrix3f.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/boundable.h>
#include <pxr/usd/usdGeom/xformable.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usdGeom/xformOp.h>
#include <pxr/usd/usdGeom/bboxCache.h>

#include "../app/notice.h"
#include "../app/handle.h"
#include "../app/camera.h"
#include "../app/selection.h"
#include "../app/application.h"
#include "../app/commands.h"
#include "../geometry/utils.h"

JVR_NAMESPACE_OPEN_SCOPE

//==================================================================================
// Create XformCommonAPI
//==================================================================================
void _EnsureXformCommonAPI(pxr::UsdPrim& prim, const pxr::UsdTimeCode& timeCode)
{
  pxr::GfVec3d translation;
  pxr::GfVec3f rotation;
  pxr::GfVec3f scale;
  pxr::GfVec3f pivot;
  pxr::UsdGeomXformCommonAPI::RotationOrder rotOrder;
  pxr::UsdGeomXformCommonAPI api(prim);
  api.GetXformVectors(&translation, &rotation, &scale, &pivot, &rotOrder, timeCode);
  pxr::UsdGeomXformable xformable(prim);
  xformable.ClearXformOpOrder();
  api.SetXformVectors(translation, rotation, scale, pivot, rotOrder, timeCode);
}

//==================================================================================
// BASE HANDLE IMPLEMENTATION
//==================================================================================
void 
BaseHandle::ComputeSizeMatrix(float width, float height)
{
  if(_compensate && _camera) {
    const pxr::GfVec3d delta = 
      _camera->GetZUpMatrix().Transform(_camera->GetPosition()) - _position;
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
  else {
    _sizeMatrix = {
      1.f,0.f,0.f,0.f,
      0.f,1.f,0.f,0.f,
      0.f,0.f,1.f,0.f,
      0.f,0.f,0.f,1.f
    };
  }
}

void 
BaseHandle::AddComponent(Shape::Component& component)
{
  if (component.index == AXIS_CAMERA)
    component.SetFlag(Shape::FLAT, true);
  _shape.AddComponent(component);
}

void 
BaseHandle::AddXComponent(Shape::Component& component)
{
  component.index = AXIS_X;
  component.color = HANDLE_X_COLOR;
  component.parentMatrix = HANDLE_X_MATRIX;
  _shape.AddComponent(component);
}

void 
BaseHandle::AddYComponent(Shape::Component& component)
{
  component.index = AXIS_Y;
  component.color = HANDLE_Y_COLOR;
  component.parentMatrix =  HANDLE_Y_MATRIX;
  _shape.AddComponent(component);
}

void 
BaseHandle::AddZComponent(Shape::Component& component)
{
  component.index = AXIS_Z;
  component.color = HANDLE_Z_COLOR;
  component.parentMatrix =  HANDLE_Z_MATRIX;
  _shape.AddComponent(component);
}

void 
BaseHandle::AddXYZComponents(Shape::Component& component)
{
  AddXComponent(component);
  AddYComponent(component);
  AddZComponent(component);
}


void
BaseHandle::AddYZXZXYComponents(Shape::Component& component)
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

void 
BaseHandle::AddHelperComponent(Shape::Component& component)
{
  component.SetFlag(Shape::HELPER, true);
  _help.AddComponent(component);
}

void 
BaseHandle::SetMatrixFromSRT()
{
  pxr::GfTransform transform;
  transform.SetScale(pxr::GfVec3d(_scale));
  transform.SetRotation(pxr::GfRotation(_rotation));
  transform.SetTranslation(pxr::GfVec3d(_position));
  _matrix = pxr::GfMatrix4f(transform.GetMatrix());
}

void 
BaseHandle::SetSRTFromMatrix()
{
  pxr::GfTransform transform;
  transform.SetMatrix(pxr::GfMatrix4d(_matrix));
  _scale = pxr::GfVec3f(transform.GetScale());
  pxr::GfQuaternion rot = transform.GetRotation().GetQuaternion();
  _rotation = pxr::GfQuatf(rot.GetReal(), pxr::GfVec3f(rot.GetImaginary()));
  _position = pxr::GfVec3f(transform.GetTranslation());
}

void 
BaseHandle::ComputeViewPlaneMatrix()
{
  _viewPlaneMatrix =
    _sizeMatrix * pxr::GfMatrix4f(_camera->GetTransform());
  _viewPlaneMatrix[3][0] = _position[0];
  _viewPlaneMatrix[3][1] = _position[1];
  _viewPlaneMatrix[3][2] = _position[2];
  _viewPlaneMatrix[3][3] = 1;
}

void 
BaseHandle::ComputePickFrustum()
{
}

void 
BaseHandle::SetVisibility(short axis)
{
}

void 
BaseHandle::ResetSelection()
{
  Application* app = Application::Get();
  
  Selection* selection = app->GetSelection();
  pxr::UsdStageRefPtr stage = app->GetWorkStage();
  if (!stage)return;
  pxr::UsdTimeCode activeTime = Time::Get()->GetActiveTime();
  pxr::UsdGeomXformCache xformCache(activeTime);
  _targets.clear();
  bool resetXformCache;
  for(size_t i=0; i<selection->GetNumSelectedItems(); ++i ) {
    const Selection::Item& item = selection->GetItem(i);
    pxr::UsdPrim prim = stage->GetPrimAtPath(item.path);

    if(prim.IsA<pxr::UsdGeomXformable>()) {
      pxr::GfMatrix4f parentMatrix(
        xformCache.GetParentToWorldTransform(prim));
      pxr::GfMatrix4f invParentMatrix(
        parentMatrix.GetInverse());
      ManipXformVectors vectors;
      pxr::UsdGeomXformCommonAPI xformApi(prim);
      xformApi.GetXformVectors(&vectors.translation, &vectors.rotation, &vectors.scale,
        &vectors.pivot, &vectors.rotOrder, activeTime); 
      const pxr::GfMatrix4f rotationMatrix(
        pxr::UsdGeomXformOp::GetOpTransform(
          pxr::UsdGeomXformCommonAPI::ConvertRotationOrderToOpType(vectors.rotOrder), VtValue(vectors.rotation)));
      const pxr::GfMatrix4f translationMatrix = pxr::GfMatrix4f(1.f).SetTranslate(pxr::GfVec3f(vectors.translation));
      const pxr::GfMatrix4f pivotMatrix = pxr::GfMatrix4f(1.f).SetTranslate(vectors.pivot);
      pxr::GfMatrix4f world( rotationMatrix * pivotMatrix * translationMatrix * parentMatrix);
      _targets.push_back({item.path, world, 
        pxr::GfMatrix4f(xformCache.GetLocalTransformation(prim, &resetXformCache)), invParentMatrix, vectors});
    }
  }
  _xformCache.Swap(xformCache);
  _ComputeCOGMatrix();
  
  pxr::GfMatrix4f invMatrix = _matrix.GetInverse();
  for(auto& target: _targets) {
    target.offset = target.base * invMatrix;
  }

  SetSRTFromMatrix();
  _displayMatrix = _ExtractRotationAndTranslateFromMatrix();
}

void 
BaseHandle::SetActiveAxis(short axis)
{
  _activeAxis = axis;
}

void 
BaseHandle::SetHoveredAxis(short axis)
{
  _hoveredAxis = axis;
}

void 
BaseHandle::UpdateCamera(const pxr::GfMatrix4f& view,
  const pxr::GfMatrix4f& proj)
{
  _shape.UpdateCamera(view, proj);
}

void 
BaseHandle::UpdatePickingPlane(short axis)
{
 _plane.Set(
    _camera->GetViewPlaneNormal(),
    pxr::GfVec3d(_position)
 );
}

const pxr::GfVec4f& 
BaseHandle::GetColor(const Shape::Component& comp)
{
  if (comp.index == AXIS_NONE)return comp.color;
  if(_interacting) {
    if(comp.flags &Shape::SELECTED) {
      return HANDLE_ACTIVE_COLOR;
    } else if (!(comp.flags & Shape::MASK)) {
      return HANDLE_HELP_COLOR;
    } else {
      return comp.color;
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

short 
BaseHandle::Select(float x, float y, float width, float height, 
  bool lock)
{
  pxr::GfRay ray = _camera->GetRay(x, y, width, height);
  pxr::GfMatrix4f m = _sizeMatrix * _displayMatrix;
  short selected = _shape.Intersect(ray, m, _viewPlaneMatrix);

  if(selected) {
    SetActiveAxis(selected);
    UpdatePickingPlane(selected);
  }
  _shape.UpdateComponents(selected, selected);
  return selected;
}

short 
BaseHandle::Pick(float x, float y, float width, float height)
{
  pxr::GfRay ray = _camera->GetRay(x, y, width, height);
  pxr::GfMatrix4f m = _sizeMatrix * _displayMatrix;
  short hovered = _shape.Intersect(ray, m, _viewPlaneMatrix);

  SetHoveredAxis(hovered);
  _shape.UpdateComponents(hovered, AXIS_NONE);
  return hovered;
}

pxr::GfMatrix4f 
BaseHandle::_ExtractRotationAndTranslateFromMatrix()
{
  return
    pxr::GfMatrix4f(1.f).SetRotate(_rotation) *
    pxr::GfMatrix4f(1.f).SetTranslate(_position);
}

void 
BaseHandle::_DrawShape(Shape* shape, const pxr::GfMatrix4f& m)
{
  shape->Bind();
  
  for (size_t i = 0; i < shape->GetNumComponents(); ++i) {
    const Shape::Component& component = shape->GetComponent(i);
    if (component.GetFlag(Shape::VISIBLE)) {
      if (component.index == AXIS_CAMERA) {
        shape->DrawComponent(i, _viewPlaneMatrix, GetColor(component));
      }
      else {
        shape->DrawComponent(i, component.parentMatrix * m, GetColor(component));
      }
    }
  }
 
  shape->Unbind();
}

void 
BaseHandle::Draw(float width, float height)
{
  ComputeSizeMatrix(width, height);
  ComputeViewPlaneMatrix();
  pxr::GfMatrix4f m = _sizeMatrix * _displayMatrix;
  GLint restoreVao;
  GLint restorePgm;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &restoreVao);
  glGetIntegerv(GL_CURRENT_PROGRAM, &restorePgm);

  _DrawShape(&_shape, m);

  if (_interacting && _help.GetNumComponents()) {
    if (_needUpdate) {
      _help.Setup(true);
      _needUpdate = false;
    }
    _DrawShape(&_help, _sizeMatrix);
  }
  glBindVertexArray(restoreVao);
  glUseProgram(restorePgm);
}

void 
BaseHandle::Setup()
{
  _shape.Setup();
  _help.Setup();
  SetMatrixFromSRT();
}

void
BaseHandle::SetProgram(GLSLProgram* pgm)
{
  _shape.SetProgram(pgm);
  _help.SetProgram(pgm);
}

void 
BaseHandle::Update(float x, float y, float width, float height)
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

pxr::GfMatrix4f _GetWorldMatrix(pxr::UsdGeomXformCache& xformCache, const pxr::UsdPrim& prim)
{
  return pxr::GfMatrix4f(xformCache.GetLocalToWorldTransform(prim));
}

void 
BaseHandle::_ComputeCOGMatrix()
{
  /*pxr::UsdGeomBBoxCache bboxCache(
    activeTime, purposes, false, false);*/
  _position = pxr::GfVec3f(0.f);
  _rotation = pxr::GfQuatf(1.f);
  _scale = pxr::GfVec3f(1.f);
  pxr::GfRange3d accumulatedRange;
  size_t numPrims = 0;
  pxr::GfTransform transform;
  for (auto& target: _targets) {
    /*
    pxr::UsdGeomXformCommonAPI xformApi(prim);
    xformApi.GetXformVectors(&transformVectors.translation, &transformVectors.rotation, 
      &transformVectors.scale, &transformVectors.pivot, &transformVectors.rotOrder, activeTime);
    const auto transMat = GfMatrix4d(1.0).SetTranslate(transformVectors.translation);
    const auto pivotMat = GfMatrix4d(1.0).SetTranslate(transformVectors.pivot);
    const auto parentToWorld = _xformCache.GetParentToWorldTransform(prim);

    // We are just interested in the pivot position and the orientation
    const GfMatrix4d toManipulator = pivotMat * transMat * parentToWorld;
    transform.SetMatrix(toManipulator.GetOrthonormalized());
    */
    bool resetsXformStack = false;
    transform.SetMatrix(pxr::GfMatrix4d(target.base));
    _position += pxr::GfVec3f(transform.GetTranslation());
    _scale += pxr::GfVec3f(transform.GetScale());
    _rotation *= pxr::GfQuatf(transform.GetRotation().GetQuat());
        
    ++numPrims;

  }
  if(numPrims) {
    float ratio = 1.f / (float)numPrims;
    _position *= ratio;
    _scale *= ratio;
    _rotation.Normalize();

    _matrix =
      pxr::GfMatrix4f(1.f).SetScale(_scale) *
      pxr::GfMatrix4f(1.f).SetRotate(_rotation) *
      pxr::GfMatrix4f(1.f).SetTranslate(_position);

    _displayMatrix = _ExtractRotationAndTranslateFromMatrix();
  }
}

pxr::GfVec3f 
BaseHandle::_ConstraintPointToAxis(const pxr::GfVec3f& point, 
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

pxr::GfVec3f 
BaseHandle::_ConstraintPointToPlane(const pxr::GfVec3f& point, 
  short axis)
{
  switch (axis) {
    case AXIS_X:
      return pxr::GfVec3f(pxr::GfPlane(_matrix.TransformDir(
        pxr::GfVec3f::Axis(0)), _position).Project(point));
    case AXIS_Y:
      return pxr::GfVec3f(pxr::GfPlane(_matrix.TransformDir(
        pxr::GfVec3f::Axis(1)), _position).Project(point));
    case AXIS_Z:
      return pxr::GfVec3f(pxr::GfPlane(_matrix.TransformDir(
        pxr::GfVec3f::Axis(2)), _position).Project(point));
    case AXIS_CAMERA:
      return _viewPlaneMatrix.GetInverse().Transform(point);
  }
  return pxr::GfVec3f(0.f);
}


pxr::GfVec3f
BaseHandle::_ConstraintPointToCircle(const pxr::GfVec3f& center, const pxr::GfVec3f& normal, 
  const pxr::GfRay& ray, short axis, float radius)
{
  double distance;
  ray.Intersect(pxr::GfPlane(normal, center), &distance, NULL);
  const pxr::GfVec3f hit(ray.GetPoint(distance));
  return center - (hit - center).GetNormalized() * radius;
}

void 
BaseHandle::BeginUpdate(float x, float y, float width, float height)
{
  Application* app = Application::Get();
  pxr::GfRay ray(
    _camera->GetPosition(), 
    _camera->GetRayDirection(x, y, width, height));

  double distance;
  bool frontFacing;
  _startMatrix = _matrix;
  if(ray.Intersect(_plane, &distance, &frontFacing)) {
    _offset = pxr::GfVec3f(pxr::GfVec3d(_position) - ray.GetPoint(distance));
  }
  _interacting = true;
  SetVisibility(_activeAxis);

}

void 
BaseHandle::EndUpdate()
{
  _interacting = false;
  _shape.SetVisibility(0b1111111111111111);
  _UpdateTargets(false);
}

//==================================================================================
// SELECT HANDLE IMPLEMENTATION
//==================================================================================
SelectHandle::SelectHandle()
{

}

void 
SelectHandle::BeginUpdate(float x, float y, float width, float height)
{

}

void 
SelectHandle::Update(float x, float y, float width, float height)
{

}

void SelectHandle::EndUpdate()
{

}

void SelectHandle::_DrawShape(Shape* shape, const pxr::GfMatrix4f& m)
{

}

void SelectHandle::SetVisibility(short axis)
{

}

void SelectHandle::_UpdateTargets(bool interacting)
{

}

//==================================================================================
// TRANSLATE HANDLE IMPLEMENTATION
//==================================================================================
TranslateHandle::TranslateHandle()
 : BaseHandle()
 , _radius(0.1f)
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


void
TranslateHandle::SetVisibility(short axis)
{
  /*YZXZXY*/
  int bits = 0;
  switch (axis) {
  case AXIS_X:
    bits = 0b0000010010; break;
  case AXIS_Y:
    bits = 0b0000100100; break;
  case AXIS_Z:
    bits = 0b0001001000; break;
  case AXIS_XY:
    bits = 0b100110110; break;
  case AXIS_XZ:
    bits = 0b0101011010; break;
  case AXIS_YZ:
    bits = 0b0011101100; break;
  case AXIS_XYZ:
    bits = 0b0000001111; break;
  default:
    bits = 0b1111111111; break;
  }
  _shape.SetVisibility(bits);
}

void 
TranslateHandle::BeginUpdate(float x, float y, float width, float height)
{
  pxr::GfRay ray = _camera->GetRay(x, y, width, height);
  double distance;
  bool frontFacing;
  _startMatrix = _matrix;
  if(ray.Intersect(_plane, &distance, &frontFacing)) {
    if(_activeAxis == AXIS_CAMERA) {
      _offset = pxr::GfVec3f(pxr::GfVec3d(_position) - ray.GetPoint(distance));
    } else {
      _offset =  pxr::GfVec3f(pxr::GfVec3d(_position) -
        _ConstraintPointToAxis(pxr::GfVec3f(ray.GetPoint(distance)), _activeAxis));
    }
  }
  _interacting = true;
  SetVisibility(_activeAxis);
}

void
TranslateHandle::Update(float x, float y, float width, float height)
{
  if(_interacting) {
    pxr::GfRay ray = _camera->GetRay(x, y, width, height);

    double distance;
    bool frontFacing;
    if(ray.Intersect(_plane, &distance, &frontFacing)) {
      if(_activeAxis == AXIS_CAMERA) {
        _position = pxr::GfVec3f(ray.GetPoint(distance)) + _offset;
      } else {
        _position = _ConstraintPointToAxis(pxr::GfVec3f(ray.GetPoint(distance)),
            _activeAxis) + _offset;
      }
      
      SetMatrixFromSRT();
      _displayMatrix = _ExtractRotationAndTranslateFromMatrix();
      _UpdateTargets(true);
    }
  }
}

void
TranslateHandle::_UpdateTargets(bool interacting)
{
  Application* app = Application::Get();
  pxr::UsdStageRefPtr stage = app->GetWorkStage();
  pxr::UsdTimeCode activeTime = pxr::UsdTimeCode::Default();
  Selection* selection = app->GetSelection();
  if (interacting) {
    for (auto& target : _targets) {
      pxr::UsdPrim targetPrim = stage->GetPrimAtPath(target.path);
      pxr::UsdGeomXformCommonAPI xformApi(stage->GetPrimAtPath(target.path));
      pxr::GfMatrix4d xformMatrix((target.offset * _matrix) * target.parent);
      xformApi.SetTranslate(xformMatrix.GetRow3(3) - target.previous.pivot, activeTime);
    }
    if(!Time::Get()->IsPlaying())AttributeChangedNotice().Send();
  }
  else {
    pxr::UsdGeomXformCache xformCache(activeTime);
    for (auto& target : _targets) {
      pxr::UsdPrim targetPrim = stage->GetPrimAtPath(target.path);
      pxr::GfMatrix4f invParentMatrix(
        xformCache.GetParentToWorldTransform(targetPrim).GetInverse());
      pxr::GfMatrix4d xformMatrix((target.offset * _matrix) * invParentMatrix);
      target.current.translation = pxr::GfVec3f(xformMatrix.GetRow3(3)) - target.previous.pivot;
    }
    ADD_COMMAND(TranslateCommand, Application::Get()->GetWorkStage(), _targets, activeTime);
  }
}

//==================================================================================
// ROTATE HANDLE IMPLEMENTATION
//==================================================================================
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

  const pxr::GfMatrix4f zeroScaleMatrix = {
    1.f, 0.f, 0.f, 0.f,
      0.f, 1.f, 0.f, 0.f,
      0.f, 0.f, 1.f, 0.f,
      0.f, 0.f, 0.f, 1.f
  };
  Shape::Component help1 = _help.AddSphere(
    AXIS_NONE, 0.1f, 8, 8, HANDLE_ACTIVE_COLOR, zeroScaleMatrix);
  AddHelperComponent(help1);
  Shape::Component help2 = _help.AddSphere(
    AXIS_NONE, 0.1f, 8, 8, HANDLE_ACTIVE_COLOR, zeroScaleMatrix);
  AddHelperComponent(help2);
  Shape::Component help3 = _help.AddDisc(
    AXIS_NONE, _radius, 0.f, 360.f, 32, HANDLE_HELP_COLOR, zeroScaleMatrix);
  AddHelperComponent(help3);
  _shape.SetVisibility(0b11111);
}

void
RotateHandle::SetVisibility(short axis)
{
  int bits = 0;
  switch (axis) {
  case AXIS_X:
    bits = 0b00010; break;
  case AXIS_Y:
    bits = 0b00100; break;
  case AXIS_Z:
    bits = 0b01000; break;
  case AXIS_CAMERA:
    bits = 0b10000; break;
  default:
    bits = 0b11111; break;
  }
  _shape.SetVisibility(bits);
}

pxr::GfVec3f 
RotateHandle::_ContraintPointToRotationPlane(const pxr::GfRay& ray)
{
  pxr::GfVec3f normal;

  switch (_activeAxis) {
  case AXIS_X:
    normal = _base.Transform(pxr::GfVec3f::Axis(0));
    break;
  case AXIS_Y:
    normal = _base.Transform(pxr::GfVec3f::Axis(1));
    break;
  case AXIS_Z:
    normal = _base.Transform(pxr::GfVec3f::Axis(2));
    break;
  }
  return _ConstraintPointToCircle(_position, normal, ray, _activeAxis, _radius);
}

void 
RotateHandle::BeginUpdate(float x, float y, float width, float height)
{
  pxr::GfRay ray = _camera->GetRay(x, y, width, height);
  double distance;

  bool frontFacing;
  _startMatrix = _matrix;
  _position = pxr::GfVec3f(_matrix[3][0], _matrix[3][1], _matrix[3][2]);
  if (ray.Intersect(_plane, &distance, &frontFacing)) {
    if (_activeAxis == AXIS_CAMERA) {
      _offset = pxr::GfVec3f(pxr::GfVec3d(_position) - ray.GetPoint(distance));
    }
    else {
      _base = _rotation;
      const pxr::GfVec3f constrained = _ContraintPointToRotationPlane(ray);

      _offset = pxr::GfVec3f(constrained - _position).GetNormalized() * _radius;

      Shape::Component& help = _help.GetComponent(0);
      help.parentMatrix = {
        1.f, 0.f, 0.f,0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        _offset[0] + _position[0], _offset[1] + _position[1], _offset[2] + _position[2], 1.f
      };
      _needUpdate = true;
    }
  }
  _interacting = true;
  SetVisibility(_activeAxis);
}

void 
RotateHandle::Update(float x, float y, float width, float height)
{

  if(_interacting) {
    pxr::GfRay ray = _camera->GetRay(x, y, width, height);

    double distance;
    bool frontFacing;
    if(ray.Intersect(_plane, &distance, &frontFacing)) {
      if(_activeAxis == AXIS_CAMERA) {
        _position = pxr::GfVec3f(ray.GetPoint(distance)) + _offset;
      } else {
        const pxr::GfVec3f constrained = _ContraintPointToRotationPlane(ray);
        const pxr::GfVec3f offset((constrained - _position).GetNormalized() * _radius);

        Shape::Component& help = _help.GetComponent(1);
        help.parentMatrix = {
          1.f, 0.f, 0.f,0.f,
          0.f, 1.f, 0.f, 0.f,
          0.f, 0.f, 1.f, 0.f,
          offset[0] + _position[0], offset[1] + _position[1], offset[2] + _position[2], 1.f
        };
        _needUpdate = true;

        const pxr::GfRotation rotation(offset, _offset);
        _rotation = pxr::GfQuatf(rotation.GetInverse().GetQuat()) * _base;

      }
      SetMatrixFromSRT();
      _displayMatrix = _ExtractRotationAndTranslateFromMatrix();
      _UpdateTargets(true);
    }
  }
}

using RotationDesc = std::pair<pxr::GfVec3f, pxr::UsdGeomXformCommonAPI::RotationOrder>;

static 
RotationDesc
_ResolveRotation(ManipTargetDesc& target,
  pxr::UsdGeomXformCommonAPI& xformApi, const pxr::GfMatrix4d& matrix,
  pxr::UsdTimeCode activeTime)
{
  const pxr::GfVec3d xAxis = target.parent.GetRow3(0);
  const pxr::GfVec3d yAxis = target.parent.GetRow3(1);
  const pxr::GfVec3d zAxis = target.parent.GetRow3(2);

  // Get latest rotation values to give a hint to the decompose function
  ManipXformVectors vectors;
  xformApi.GetXformVectors(&vectors.translation, &vectors.rotation, &vectors.scale,
    &vectors.pivot, &vectors.rotOrder, activeTime);

  double thetaTw = pxr::GfDegreesToRadians(vectors.rotation[0]);
  double thetaFB = pxr::GfDegreesToRadians(vectors.rotation[1]);
  double thetaLR = pxr::GfDegreesToRadians(vectors.rotation[2]);
  double thetaSw = 0.0;

  // Decompose the matrix in angle values
  pxr::GfRotation::DecomposeRotation(matrix, xAxis, yAxis, zAxis, 1.0,
    &thetaTw, &thetaFB, &thetaLR, &thetaSw, true);
  return std::make_pair(
    pxr::GfVec3f(pxr::GfRadiansToDegrees(thetaTw), pxr::GfRadiansToDegrees(thetaFB), pxr::GfRadiansToDegrees(thetaLR)),
    vectors.rotOrder);
}

void
RotateHandle::_UpdateTargets(bool interacting)
{
  Application* app = Application::Get();
  pxr::UsdStageRefPtr stage = app->GetWorkStage();
  pxr::UsdTimeCode activeTime = pxr::UsdTimeCode::Default();
  Selection* selection = app->GetSelection();
  if (interacting) {
    for (auto& target : _targets) {
      pxr::UsdPrim targetPrim = stage->GetPrimAtPath(target.path);
      pxr::UsdGeomXformCommonAPI xformApi(stage->GetPrimAtPath(target.path));
      pxr::GfMatrix4d xformMatrix((target.offset * _matrix) * target.parent);

      const RotationDesc rotation =
        _ResolveRotation(target, xformApi, xformMatrix, activeTime);
      xformApi.SetRotate(rotation.first, rotation.second, activeTime);
    }
    if(!Time::Get()->IsPlaying())AttributeChangedNotice().Send();
  }
  else {
    pxr::UsdGeomXformCache xformCache(activeTime);
    for (auto& target : _targets) {

      pxr::UsdGeomXformable xformable(stage->GetPrimAtPath(target.path));
      pxr::GfMatrix4f invParentMatrix(
        xformCache.GetParentToWorldTransform(xformable.GetPrim()).GetInverse());
      pxr::GfMatrix4d xformMatrix((target.offset * _matrix) * invParentMatrix);

      pxr::UsdGeomXformCommonAPI xformApi(xformable.GetPrim());
      const RotationDesc rotation =
        _ResolveRotation(target, xformApi, xformMatrix, activeTime);

      target.current.rotation = rotation.first;
      target.current.rotOrder = rotation.second;
    }
    
    ADD_COMMAND(RotateCommand, Application::Get()->GetWorkStage(), _targets, activeTime);
  }
}

//==================================================================================
// SCALE HANDLE IMPLEMENTATION
//==================================================================================
ScaleHandle::ScaleHandle()
  : BaseHandle()
  , _baseScale(pxr::GfVec3f(1.f))
{
  Shape::Component center = _shape.AddBox(
    AXIS_CAMERA, 0.1f, 0.1f, 0.01f, HANDLE_HELP_COLOR, {
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f });
  AddComponent(center);

  Shape::Component cylinder = _shape.AddCylinder(
    AXIS_X, 0.02f, 0.8f, 16, 2, HANDLE_X_COLOR, {
      1.f,0.f,0.f,0.f,
      0.f,1.f,0.f,0.f,
      0.f,0.f,1.f,0.f,
      0.f,0.2f,0.f,1.f 
    });
  AddXYZComponents(cylinder);

  Shape::Component box = _shape.AddBox(
    AXIS_X, 0.16f, 0.16f, 0.16f, HANDLE_X_COLOR, {
      1.f, 0.f, 0.f, 0.f,
      0.f, 1.F, 0.f, 0.f,
      0.f, 0.f, 1.f, 0.f,
      0.f, 1.f, 0.f, 1.f 
    });
  AddXYZComponents(box);

  Shape::Component plane = _shape.AddBox(
    AXIS_X, 0.25f, 0.025f, 0.25f, HANDLE_X_COLOR, {
      1.f, 0.f, 0.f, 0.f,
      0.f, 1.F, 0.f, 0.f,
      0.f, 0.f, 1.f, 0.f,
      0.5f, 0.f, 0.5f, 1.f
    });
  AddYZXZXYComponents(plane);
}

void
ScaleHandle::SetVisibility(short axis)
{
  /*YZXZXY*/
  int bits = 0;
  switch (axis) {
  case AXIS_X:
    bits = 0b0000010010; break;
  case AXIS_Y:
    bits = 0b0000100100; break;
  case AXIS_Z:
    bits = 0b0001001000; break;
  case AXIS_XY:
    bits = 0b100110110; break;
  case AXIS_XZ:
    bits = 0b0101011010; break;
  case AXIS_YZ:
    bits = 0b0011101100; break;
  case AXIS_XYZ:
    bits = 0b0000001111; break;
  default:
    bits = 0b1111111111; break;
  }
  _shape.SetVisibility(bits);
}

void 
ScaleHandle::BeginUpdate(float x, float y, float width, float height)
{
  pxr::GfRay ray = _camera->GetRay(x, y, width, height);

  double distance;
  bool frontFacing;
  _startMatrix = _matrix;
  _baseScale = pxr::GfVec3f(0.f);
  if (ray.Intersect(_plane, &distance, &frontFacing)) {
    switch (_activeAxis) {
      case AXIS_CAMERA:
        _offset = pxr::GfVec3f(pxr::GfVec3d(_position) - ray.GetPoint(distance));
        break;
      case AXIS_X:
      case AXIS_Y:
      case AXIS_Z:
        _offset = _position - 
          _ConstraintPointToAxis(pxr::GfVec3f(ray.GetPoint(distance)), _activeAxis);
        break;
      case AXIS_XY:
      case AXIS_XZ:
      case AXIS_YZ:
        _offset = _position - 
          _ConstraintPointToPlane(pxr::GfVec3f(ray.GetPoint(distance)), _activeAxis);
      default:
        _offset = pxr::GfVec3f(pxr::GfVec3d(_position) - ray.GetPoint(distance));
        break;
    }
  }
  _interacting = true;
  SetVisibility(_activeAxis);
}

void 
ScaleHandle::Update(float x, float y, float width, float height)
{
  if (_interacting) {
    pxr::GfRay ray = _camera->GetRay(x, y, width, height);

    double distance;
    bool frontFacing;
    float value;
    pxr::GfVec3f offset;

    if (ray.Intersect(_plane, &distance, &frontFacing)) {
      switch (_activeAxis) {
        case AXIS_CAMERA:
          offset = pxr::GfVec3f(pxr::GfVec3d(_position) - ray.GetPoint(distance));
          value = _offset[0] - offset[0] + _offset[1] - offset[1];
          _scale = _baseScale + pxr::GfVec3f(value);
          break;
        case AXIS_X:
          offset = pxr::GfVec3f(_position) - 
            _ConstraintPointToAxis(pxr::GfVec3f(ray.GetPoint(distance)), _activeAxis);
          _offsetScale = pxr::GfVec3f(pxr::GfPow(offset[0] / _offset[0], 3) - 1.f, 0.f, 0.f);
          _scale = _baseScale + _offsetScale;
          break;
        case AXIS_Y:
          offset = pxr::GfVec3f(_position) - 
            _ConstraintPointToAxis(pxr::GfVec3f(ray.GetPoint(distance)), _activeAxis);
          _offsetScale = pxr::GfVec3f(0.f, pxr::GfPow(offset[1] / _offset[1] - 1.f, 3), 0.f);
          _scale = _baseScale + _offsetScale;
          break;
        case AXIS_Z:
          offset = pxr::GfVec3f(_position) - 
            _ConstraintPointToAxis(pxr::GfVec3f(ray.GetPoint(distance)), _activeAxis);
          _offsetScale = pxr::GfVec3f(0.f, 0.f, pxr::GfPow(offset[2] / _offset[2], 3));
          _scale = _baseScale + _offsetScale;
          break;
        case AXIS_XY:
          offset = pxr::GfVec3f(pxr::GfVec3d(_offset) -
            _ConstraintPointToPlane(pxr::GfVec3f(ray.GetPoint(distance)), _activeAxis));
          _offsetScale = pxr::GfVec3f(offset[0] / _offset[0] - 1.f, offset[1] / _offset[1] - 1.f, 0.f);
          _scale = _baseScale + _offsetScale;
          break;
        case AXIS_XZ:
          offset = pxr::GfVec3f(pxr::GfVec3d(_offset) -
            _ConstraintPointToPlane(pxr::GfVec3f(ray.GetPoint(distance)), _activeAxis));
          _offsetScale = pxr::GfVec3f(offset[0] / _offset[0] - 1.f, 0.f, offset[2] / _offset[2] - 1.f);
          _scale = _baseScale + _offsetScale;
          break;
        case AXIS_YZ:
          offset = pxr::GfVec3f(pxr::GfVec3d(_offset) -
            _ConstraintPointToPlane(pxr::GfVec3f(ray.GetPoint(distance)), _activeAxis));
          _offsetScale = pxr::GfVec3f(0.f, offset[1] / _offset[1] - 1.f, offset[2] / _offset[2] - 1.f);
          _scale = _baseScale + _offsetScale;
          break;
      }
      SetMatrixFromSRT();
      _displayMatrix = _ExtractRotationAndTranslateFromMatrix();
      _UpdateTargets(true);
    }
  }
}

void 
ScaleHandle::EndUpdate()
{
  _UpdateTargets(false);
  _offsetScale = pxr::GfVec3f(0.f);
  _displayMatrix = _ExtractRotationAndTranslateFromMatrix();
  _interacting = false;
}

void 
ScaleHandle::_SetMaskMatrix(size_t axis)
{
  switch (axis) {
  case AXIS_X:
    _maskMatrix = {
      1.f, 0.f, 0.f, 0.f,
      0.f, 0.f, 0.f, 0.f,
      0.f, 0.f, 0.f, 0.f,
      0.f, 0.f, 0.f, 1.f
    }; break;

  case AXIS_Y:
    _maskMatrix = {
      0.f, 0.f, 0.f, 0.f,
      0.f, 1.f, 0.f, 0.f,
      0.f, 0.f, 0.f, 0.f,
      0.f, 0.f, 0.f, 1.f
    }; break;

  case AXIS_Z:
    _maskMatrix = {
      0.f, 0.f, 0.f, 0.f,
      0.f, 0.f, 0.f, 0.f,
      0.f, 0.f, 1.f, 0.f,
      0.f, 0.f, 0.f, 1.f
    }; break;

  case AXIS_XY:
    _maskMatrix = {
      1.f, 0.f, 0.f, 0.f,
      0.f, 1.f, 0.f, 0.f,
      0.f, 0.f, 0.f, 0.f,
      0.f, 0.f, 0.f, 1.f
    }; break;

  case AXIS_XZ:
    _maskMatrix = {
      1.f, 0.f, 0.f, 0.f,
      0.f, 0.f, 0.f, 0.f,
      0.f, 0.f, 1.f, 0.f,
      0.f, 0.f, 0.f, 1.f
    }; break;

  case AXIS_YZ:
    _maskMatrix = {
      0.f, 0.f, 0.f, 0.f,
      0.f, 1.f, 0.f, 0.f,
      0.f, 0.f, 1.f, 0.f,
      0.f, 0.f, 0.f, 1.f
    }; break;

  default:
    _maskMatrix = {
      1.f, 0.f, 0.f, 0.f,
      0.f, 1.f, 0.f, 0.f,
      0.f, 0.f, 1.f, 0.f,
      0.f, 0.f, 0.f, 1.f
    }; break;

  }
}

pxr::GfVec3f 
ScaleHandle::_GetTranslateOffset(size_t axis)
{
  switch (axis) {
    case AXIS_X:
      if (_activeAxis == axis)
        return { (float)_offsetScale[0], 0.f, 0.f };
      else 
        return { 0.f, 0.f, 0.f };

    case AXIS_Y:
      if (_activeAxis == axis)
        return { 0.f, (float)_offsetScale[1], 0.f };
      else
        return { 0.f, 0.f, 0.f };
      
    case AXIS_Z:
      if (_activeAxis == axis)
        return { 0.f, 0.f, (float)_offsetScale[2] };
      else
        return { 0.f, 0.f, 0.f };
      
    case AXIS_XY:
      if (_activeAxis == axis)
        return { (float)_offsetScale[0], (float)_offsetScale[1], 0.f };
      else
        return { 0.f, 0.f, 0.f };
      
    case AXIS_XZ:
      if (_activeAxis == axis)
        return { (float)_offsetScale[0], 0.f, (float)_offsetScale[2] };
      else
        return { 0.f, 0.f, 0.f };
      
    case AXIS_YZ:
      if (_activeAxis == axis)
        return { 0.f, (float)_offsetScale[1], (float)_offsetScale[2] };
      else
        return { 0.f, 0.f, 0.f };
      
    }
  
  return pxr::GfVec3f(0.f);
}

pxr::GfVec3f 
ScaleHandle::_GetScaleOffset(size_t axis)
{
  switch (axis) {
  case AXIS_X:
    if(_activeAxis == axis)
      return { 1.f + (float)_offsetScale[0], 1.f, 1.f };
    else
      return { 1.f, 1.f, 1.f };

  case AXIS_Y:
    if(_activeAxis == axis)
      return { 1.f, 1.f + (float)_offsetScale[1], 1.f };
    else
      return { 1.f, 1.f, 1.f };

  case AXIS_Z:
    if(_activeAxis == axis)
      return { 1.f, 1.f, 1.f + (float)_offsetScale[2] };
    else
      return { 1.f, 1.f, 1.f };

    case AXIS_XY:
      if (_activeAxis == axis)
        return { (float)_offsetScale[0], (float)_offsetScale[1], 0.f };
      else
        return { 1.f, 1.f, 1.f };
      
    case AXIS_XZ:
      if (_activeAxis == axis)
        return { (float)_offsetScale[0], 0.f, (float)_offsetScale[2] };
      else
        return { 1.f, 1.f, 1.f };
      
    case AXIS_YZ:
      if (_activeAxis == axis)
        return { 0.f, (float)_offsetScale[1], (float)_offsetScale[2] };
      else
        return { 1.f, 1.f, 1.f };

  }

  return pxr::GfVec3f(0.f);
}

void 
ScaleHandle::_DrawShape(Shape* shape, const pxr::GfMatrix4f& m)
{
  shape->Bind();
  Shape::Component* component;
  
  // center
  {
    component = &(shape->GetComponent(0));
    shape->DrawComponent(0, _viewPlaneMatrix, GetColor(*component));
  }
  // cylinders
  {
    for (size_t i = 0; i < 3; ++i) {
      component = &(shape->GetComponent(1 + i));
      pxr::GfMatrix4f mm = pxr::GfMatrix4f(component->parentMatrix) *
        pxr::GfMatrix4f(1.f).SetScale(_GetScaleOffset(AXIS_X + i)) * m;
      shape->DrawComponent(1 + i, mm, GetColor(*component));
    }
  }
  // boxes
  {
    for (size_t i = 0; i < 3; ++i) {
      component = &(shape->GetComponent(4 + i));
      pxr::GfMatrix4f mm = pxr::GfMatrix4f(component->parentMatrix) *
        pxr::GfMatrix4f(1.f).SetTranslate(_GetTranslateOffset(AXIS_X + i)) * m;
      shape->DrawComponent(4 + i, mm, GetColor(*component));
    }
  }
  // planes
  {
    for (size_t i = 0; i < 3; ++i) {
      component = &(shape->GetComponent(7 + i));
      pxr::GfMatrix4f mm = pxr::GfMatrix4f(component->parentMatrix) *
        pxr::GfMatrix4f(1.f).SetTranslate(_GetTranslateOffset(AXIS_XY + i)) * m;
      shape->DrawComponent(7 + i, mm, GetColor(*component));
    }
  }
        
  shape->Unbind();
}

void
ScaleHandle::_UpdateTargets(bool interacting)
{
  Application* app = Application::Get();
  pxr::UsdStageRefPtr stage = app->GetWorkStage();
  pxr::UsdTimeCode activeTime = pxr::UsdTimeCode::Default();
  Selection* selection = app->GetSelection();
  if (interacting) {
    for (auto& target : _targets) {
      pxr::UsdPrim targetPrim = stage->GetPrimAtPath(target.path);
      pxr::UsdGeomXformCommonAPI api(stage->GetPrimAtPath(target.path));
      pxr::GfMatrix4d xformMatrix((target.offset * _matrix) * target.parent);
      api.SetScale(target.previous.scale + 
        pxr::GfVec3f(xformMatrix[0][0], xformMatrix[1][1], xformMatrix[2][2]), activeTime);
    }
    if(!Time::Get()->IsPlaying())AttributeChangedNotice().Send();
  }
  else {
    pxr::UsdGeomXformCache xformCache(activeTime);
    for (auto& target : _targets) {
      pxr::UsdPrim targetPrim = stage->GetPrimAtPath(target.path);
      pxr::GfMatrix4f invParentMatrix(
        xformCache.GetParentToWorldTransform(targetPrim).GetInverse());
      pxr::GfMatrix4d xformMatrix((target.offset * _matrix) * invParentMatrix);

      target.current.scale = target.previous.scale +
        pxr::GfVec3f(xformMatrix[0][0], xformMatrix[1][1], xformMatrix[2][2]);
    }

    ADD_COMMAND(ScaleCommand, Application::Get()->GetWorkStage(), _targets, activeTime);
  }
}


//==================================================================================
// BRUSH HANDLE IMPLEMENTATION
//==================================================================================
BrushHandle::BrushHandle()
  : BaseHandle(false), _minRadius(1.f), _maxRadius(2.f), _color(pxr::GfVec4f(1.f,0.f,0.f,1.f))
{
  Shape::Component cylinder = _shape.AddCylinder(
    AXIS_NORMAL, 0.02f, 1.f, 16, 2, HANDLE_ACTIVE_COLOR, HANDLE_Z_MATRIX);
  AddComponent(cylinder);
  Shape::Component innerRadius = _shape.AddTorus(
    AXIS_NORMAL, _minRadius, 0.025f, 32, 8, HANDLE_HELP_COLOR, HANDLE_Z_MATRIX);
  AddComponent(innerRadius);
  Shape::Component outerRadius = _shape.AddTorus(
    AXIS_NORMAL, _maxRadius, 0.025f, 32, 8, HANDLE_HELP_COLOR, HANDLE_Z_MATRIX);
  AddComponent(outerRadius);

}

void 
BrushHandle::_BuildStroke(bool replace)
{
  if (_path.size() < 2) return;

  const pxr::GfVec3f normal(_camera->GetViewPlaneNormal());
  std::vector<pxr::GfVec3f> profile(2);

  std::vector<pxr::GfMatrix4f> xfos(_path.size());
  pxr::GfVec3f tangent, bitangent, up;
  for (size_t i = 0; i < _path.size(); ++i) {
    if (i == 0) {
      tangent = (_path[1] - _path[0]).GetNormalized();
    }
    else if (i == _path.size() - 1) {
      size_t last = _path.size() - 1;
      tangent = (_path[last] - _path[last - 1]).GetNormalized();
    }
    else {
      tangent =
        ((_path[i] - _path[i - 1]) + (_path[i + 1] - _path[i])).GetNormalized();
    }
    bitangent = (tangent ^ normal).GetNormalized();
    up = (bitangent ^ tangent).GetNormalized();
    pxr::GfMatrix3f rotation(
      bitangent[0], bitangent[1], bitangent[2],
      tangent[0], tangent[1], tangent[2],
      up[0], up[1], up[2]
    );
    xfos[i] =
      pxr::GfMatrix4f(1.f).SetRotate(rotation) *
      pxr::GfMatrix4f(1.f).SetTranslate(_path[i]);
  }
  profile[0] = { -1, 0, 0 };
  profile[1] = { 1, 0, 0 };
  if(replace) _help.RemoveLastComponent();
  Shape::Component comp = _help.AddExtrusion(
    AXIS_NONE, xfos, profile, _color);
  _help.AddComponent(comp);
  _needUpdate = true;
}

void 
BrushHandle::BeginUpdate(float x, float y, float width, float height)
{
  _path.clear();
  pxr::GfRay ray = _camera->GetRay(x, y, width, height);

  double distance;
  ray.Intersect(_plane, &distance);
  _path.push_back(pxr::GfVec3f(ray.GetPoint(distance)));
  
  _interacting = true;
  _color = pxr::GfVec4f(
    RANDOM_0_1,
    RANDOM_0_1,
    RANDOM_0_1,
    1.f
  );
}

void 
BrushHandle::EndUpdate()
{
  _BuildStroke(false);
  _path.clear();
  _interacting = false;
}

void 
BrushHandle::Update(float x, float y, float width, float height)
{
  if (_interacting) {
    Pick(x, y, width, height);
    pxr::GfRay ray = _camera->GetRay(x, y, width, height);

    double distance;
    ray.Intersect(_plane, &distance);
    _path.push_back(pxr::GfVec3f(ray.GetPoint(distance)));

    _BuildStroke(true);
  }
}
  
short 
BrushHandle::Pick(float x, float y, float width, float height)
{
  UpdatePickingPlane(AXIS_CAMERA);
  pxr::GfRay ray = _camera->GetRay(x, y, width, height);

  ComputeViewPlaneMatrix();

  pxr::GfVec3f translate;
  double planeDistance;
  if (ray.Intersect(_plane, &planeDistance))
    translate = pxr::GfVec3f(ray.GetPoint(planeDistance));
  else translate = pxr::GfVec3f(ray.GetPoint(_distance));
  _matrix = _viewPlaneMatrix;
  memcpy(&_matrix[3][0], &translate[0], 3 * sizeof(float));

  return false;
}

void 
BrushHandle::Draw(float width, float height)
{
  ComputeSizeMatrix(width, height);
  ComputeViewPlaneMatrix();
  pxr::GfMatrix4f m = _sizeMatrix * _matrix;
  GLint vao;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);

  _DrawShape(&_shape, m);
  
  if (_help.GetNumComponents()) {
    if (_needUpdate) {
      _help.Setup(true);
      _needUpdate = false;
    }
    _DrawShape(&_help, pxr::GfMatrix4f(1.f));
  }
  glBindVertexArray(vao);
}

short 
BrushHandle::Select(float x, float y, float width, float height, bool lock)
{
  return AXIS_NORMAL;
}

JVR_NAMESPACE_CLOSE_SCOPE