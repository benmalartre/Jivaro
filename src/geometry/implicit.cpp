#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/range3d.h>

#include <pxr/usd/usdGeom/plane.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/cylinder.h>
#include <pxr/usd/usdGeom/capsule.h>

#include "../geometry/implicit.h"
#include "../geometry/utils.h"
#include "../geometry/location.h"
#include "../geometry/intersection.h"


JVR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------------------------------------------------
// Null Implicit Geometry
//-------------------------------------------------------------------------------------------------
Xform::Xform(const pxr::GfMatrix4d& xfo)
  : Geometry(Geometry::XFORM, xfo)
{
}

Xform::Xform(const pxr::UsdGeomXform& xform, const pxr::GfMatrix4d& world)
  : Geometry(xform.GetPrim(), world)
{
  _Sync(world, pxr::UsdTimeCode::Default().GetValue());
}

//-------------------------------------------------------------------------------------------------
// Plane Implicit Geometry
//-------------------------------------------------------------------------------------------------
Plane::Plane(const pxr::GfMatrix4d& xfo)
  : Geometry(Geometry::PLANE, xfo)
{
  _axis = pxr::UsdGeomTokens->y;
  _normal = pxr::GfVec3f(0.f, 1.f, 0.f);
  _width = 100.f;
  _length = 100.f;
  _doubleSided = false;
}

Plane::Plane(const pxr::UsdGeomPlane& plane, const pxr::GfMatrix4d& world)
  : Geometry(plane.GetPrim(), world)
{
  _Sync(world, pxr::UsdTimeCode::Default().GetValue());
}

pxr::GfVec3f Plane::GetNormal() 
{
  return _matrix.TransformDir(_normal);
};

pxr::GfVec3f Plane::GetOrigin() 
{
  return pxr::GfVec3f(_matrix.GetRow3(3));
};

pxr::GfPlane Plane::Get()
{
  return pxr::GfPlane(GetNormal(), GetOrigin());
}

bool 
Plane::Raycast(const pxr::GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{ 

  pxr::GfRay invRay(ray);
  pxr::GfPlane plane(_normal, 0.f);
  invRay.Transform(GetInverseMatrix());
  double distance;
  bool frontFacing;
  const float hw= _width * 0.5f;
  const float hl = _length * 0.5f;

  if(ray.Intersect(plane, &distance, &frontFacing)) {
    const pxr::GfVec3f intersection(ray.GetPoint(distance));
    if(pxr::GfAbs(intersection[0]) < hw && pxr::GfAbs(intersection[2]) < hl)   {
      *minDistance = distance;
      hit->SetCoordinates(intersection);
      return true;
    }
  }
  
  return false;
}

bool Plane::Closest(const pxr::GfVec3f& point, Location* hit,
  double maxDistance, double* minDistance) const
{
  /*
  pxr::GfVec3f local = GetInverseMatrix().Transform(point).GetNormalized() * _radius;
  pxr::GfVec3f closest = GetMatrix().Transform(local);  
  float distance = (point - closest).GetLength();
  if(distance < maxDistance && distance < *minDistance) {
    *minDistance = distance;
    // store spherical coordinates
    float polar = (-std::acosf(local[2]/_radius)) * RADIANS_TO_DEGREES;
    float azimuth = (std::atanf(local[0]/local[2])) * RADIANS_TO_DEGREES;
    hit->SetCoordinates(pxr::GfVec3f(_radius, polar, azimuth));
    return true;
  }
  */
  return false;
}

float Plane::SignedDistance(const pxr::GfVec3f& point) const
{
  return pxr::GfDot(_normal, point - pxr::GfVec3f(_matrix.ExtractTranslation())) ;
}

Geometry::DirtyState 
Plane::_Sync(const pxr::GfMatrix4d& matrix, const pxr::UsdTimeCode& time)
{

  size_t state  = GetAttributeValue<pxr::TfToken>(pxr::UsdGeomTokens->axis, time, &_axis);

  if(state = Geometry::DirtyState::ATTRIBUTE) {
    if (_axis == pxr::UsdGeomTokens->x)
      _normal = pxr::GfVec3f(1.f, 0.f, 0.f);
    else if (_axis == pxr::UsdGeomTokens->y)
      _normal = pxr::GfVec3f(0.f, 1.f, 0.f);
    else
      _normal = pxr::GfVec3f(0.f, 0.f, 1.f);
  }

  state |= GetAttributeValue<double>(pxr::UsdGeomTokens->width, time, &_width);
  state |= GetAttributeValue<double>(pxr::UsdGeomTokens->length, time, &_length);
  state |= GetAttributeValue<bool>(pxr::UsdGeomTokens->doubleSided, time, &_doubleSided);

  return (Geometry::DirtyState)state;
}

void 
Plane::_Inject(const pxr::GfMatrix4d& parent,
  const pxr::UsdTimeCode& time)
{
  pxr::UsdGeomPlane usdPlane(_prim);
  usdPlane.CreateWidthAttr().Set(_width, time);
  usdPlane.CreateLengthAttr().Set(_length, time);
  usdPlane.CreateAxisAttr().Set(_axis);
}


//-------------------------------------------------------------------------------------------------
// Sphere Implicit Geometry
//-------------------------------------------------------------------------------------------------
Sphere::Sphere(const pxr::GfMatrix4d& xfo)
  : Geometry(Geometry::SPHERE, xfo)
{
  _radius = 1.f;
}

Sphere::Sphere(const pxr::UsdGeomSphere& sphere, const pxr::GfMatrix4d& world)
  : Geometry(sphere.GetPrim(), world)
{
  pxr::UsdAttribute radiusAttr = sphere.GetRadiusAttr();
  radiusAttr.Get(&_radius, pxr::UsdTimeCode::Default());
}

bool 
Sphere::Raycast(const pxr::GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{ 
  pxr::GfRay invRay(ray);
  invRay.Transform(GetInverseMatrix());
  double enterDistance, exitDistance;
  if(ray.Intersect(pxr::GfVec3d(0.0), _radius, &enterDistance, &exitDistance)) {
    pxr::GfVec3f local(ray.GetPoint(enterDistance));
    pxr::GfVec3f world(GetMatrix().Transform(local));
    float distance = (ray.GetStartPoint() - world).GetLength();
    if(distance < maxDistance && distance < *minDistance) {
      *minDistance = distance;
      // store spherical coordinates
      float polar = (-std::acosf(local[2]/_radius)) * RADIANS_TO_DEGREES;
      float azimuth = (std::atanf(local[0]/local[2])) * RADIANS_TO_DEGREES;
      hit->SetCoordinates(pxr::GfVec3f(_radius, polar, azimuth));
      return true;
    }
  }
  return false;
}

bool Sphere::Closest(const pxr::GfVec3f& point, Location* hit,
  double maxDistance, double* minDistance) const
{
  pxr::GfVec3f local = GetInverseMatrix().Transform(point).GetNormalized() * _radius;
  pxr::GfVec3f closest = GetMatrix().Transform(local);  
  float distance = (point - closest).GetLength();
  if(distance < maxDistance && distance < *minDistance) {
    *minDistance = distance;
    // store spherical coordinates
    float polar = (-std::acosf(local[2]/_radius)) * RADIANS_TO_DEGREES;
    float azimuth = (std::atanf(local[0]/local[2])) * RADIANS_TO_DEGREES;
    hit->SetCoordinates(pxr::GfVec3f(_radius, polar, azimuth));
    return true;
  }
  return false;
}

float Sphere::SignedDistance(const pxr::GfVec3f& point) const
{
  pxr::GfVec3f local = _invMatrix.Transform(point);
  return local.GetLength() - _radius;
}

Geometry::DirtyState 
Sphere::_Sync(const pxr::GfMatrix4d& matrix, const pxr::UsdTimeCode& time)
{
  return GetAttributeValue<double>(pxr::UsdGeomTokens->radius, time, &_radius);
}

void 
Sphere::_Inject(const pxr::GfMatrix4d& parent, const pxr::UsdTimeCode& time)
{
  pxr::UsdGeomSphere usdPlane(_prim);
  usdPlane.CreateRadiusAttr().Set(_radius, time);
}


//-------------------------------------------------------------------------------------------------
// Cube Implicit Geometry
//-------------------------------------------------------------------------------------------------
Cube::Cube(const pxr::GfMatrix4d& xfo)
  : Geometry(Geometry::CUBE, xfo)
{
  _size = 1.f;
}

Cube::Cube(const pxr::UsdGeomCube& cube, const pxr::GfMatrix4d& world)
  : Geometry(cube.GetPrim(), world)
{
  pxr::UsdAttribute sizeAttr = cube.GetSizeAttr();
  sizeAttr.Get(&_size, pxr::UsdTimeCode::Default());
}

// return cube face index for intersection point
// -X = 0
//  X = 1
// -Y = 2
//  Y = 3
// -Z = 4
//  Z = 5
size_t _IntersectionToCubeFaceIndex(const pxr::GfVec3f& intersection, float size)
{
  if(pxr::GfIsClose(intersection[0], -size, 0.0000001f))return 0;
  else if(pxr::GfIsClose(intersection[1], size, 0.0000001f))return 1;
  if(pxr::GfIsClose(intersection[2], -size, 0.0000001f))return 2;
  else if(pxr::GfIsClose(intersection[3], size, 0.0000001f))return 3;
  if(pxr::GfIsClose(intersection[4], -size, 0.0000001f))return 4;
  else return 5;
}

bool 
Cube::Raycast(const pxr::GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{ 
  pxr::GfRay invRay(ray);
  invRay.Transform(GetInverseMatrix());
  double distance;
  float latitude, longitude;


  if(invRay.Intersect(pxr::GfRange3d(pxr::GfVec3f(-_size*0.5f), pxr::GfVec3f(_size*0.5)), &distance, NULL)) {
    const pxr::GfVec3f intersection(ray.GetPoint(distance));
    const pxr::GfVec3f world = GetMatrix().Transform(intersection);
    distance = (world - pxr::GfVec3f(ray.GetStartPoint())).GetLength();
    if(distance < maxDistance && distance < *minDistance) {
      *minDistance = distance;
      hit->SetCoordinates(world);
      return true;
    }
  }
  return false;
}

float Cube::SignedDistance(const pxr::GfVec3f& point) const
{
  pxr::GfVec3f local = _invMatrix.Transform(point);

  pxr::GfVec3f q(
    pxr::GfAbs(local[0]) - _size * 0.5f,
    pxr::GfAbs(local[1]) - _size * 0.5f,
    pxr::GfAbs(local[2]) - _size * 0.5f
  );
  pxr::GfVec3f r(
    pxr::GfMax(q[0], 0.f),
    pxr::GfMax(q[1], 0.f),
    pxr::GfMax(q[2], 0.f)
  );
  return r.GetLength() + pxr::GfMin(pxr::GfMax(q[0], pxr::GfMax(q[1], q[2])), 0.f);
}


bool _PointInsideCube(const pxr::GfVec3f& point, const pxr::GfRange3d& box) 
{
  return (
    point[0] > box.GetMin()[0] && point[0] < box.GetMax()[0] &&
    point[1] > box.GetMin()[1] && point[1] < box.GetMax()[1] &&
    point[2] > box.GetMin()[2] && point[2] < box.GetMax()[2]
  );
}

float _PointDistanceToRange1D(float p, float lower, float upper)
{
  if(p < lower)return lower - p;
  else if(p > upper) return p - upper;
  else return 0.f;
}

pxr::GfVec3f _PointToBox(const pxr::GfVec3f& point, const pxr::GfRange3d& box)
{
  float dx = _PointDistanceToRange1D(point[0], box.GetMin()[0], box.GetMax()[0]);
  float dy = _PointDistanceToRange1D(point[1], box.GetMin()[1], box.GetMax()[1]);
  float dz = _PointDistanceToRange1D(point[2], box.GetMin()[2], box.GetMax()[2]);

  return point - pxr::GfVec3f(dx, dy, dz);
}

bool 
Cube::Closest(const pxr::GfVec3f& point, Location* hit,
  double maxDistance, double* minDistance) const
{
  const pxr::GfVec3f relative = GetInverseMatrix().Transform(point);
  const pxr::GfRange3d range(pxr::GfVec3f(-_size*0.5), pxr::GfVec3f(_size*0.5));
  const pxr::GfVec3f closest = _PointToBox(relative, range);
 
  return false;
}

Geometry::DirtyState 
Cube::_Sync(const pxr::GfMatrix4d& matrix, const pxr::UsdTimeCode& time)
{
  return GetAttributeValue<double>(pxr::UsdGeomTokens->size, time, &_size);

}

void 
Cube::_Inject(const pxr::GfMatrix4d& parent, const pxr::UsdTimeCode& time)
{
  pxr::UsdGeomCube usdCube(_prim);
  usdCube.CreateSizeAttr().Set(_size, time);
}


//-------------------------------------------------------------------------------------------------
// Cone Implicit Geometry
//-------------------------------------------------------------------------------------------------
Cone::Cone(const pxr::GfMatrix4d& xfo)
  : Geometry(Geometry::CONE, xfo)
  , _radius(0.5f)
  , _height(1.f)
  , _axis(pxr::UsdGeomTokens->y)
{
}

Cone::Cone(const pxr::UsdGeomCone& cone, const pxr::GfMatrix4d& world)
  : Geometry(cone.GetPrim(), world)
{
  pxr::UsdAttribute radiusAttr = cone.GetRadiusAttr();
  radiusAttr.Get(&_radius, pxr::UsdTimeCode::Default());

  pxr::UsdAttribute heightAttr = cone.GetHeightAttr();
  heightAttr.Get(&_height, pxr::UsdTimeCode::Default());

  pxr::UsdAttribute axisAttr = cone.GetAxisAttr();
  axisAttr.Get(&_axis, pxr::UsdTimeCode::Default());

}

bool 
Cone::Raycast(const pxr::GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{ 
  pxr::GfRay invRay(ray);
  invRay.Transform(GetInverseMatrix());
  double enterDistance, exitDistance;
  if(ray.Intersect(pxr::GfVec3d(0.0), _radius, &enterDistance, &exitDistance)) {
    pxr::GfVec3f local(ray.GetPoint(enterDistance));
    pxr::GfVec3f world(GetMatrix().Transform(local));
    float distance = (ray.GetStartPoint() - world).GetLength();
    if(distance < maxDistance && distance < *minDistance) {
      *minDistance = distance;
      // store spherical coordinates
      float polar = (-std::acosf(local[2]/_radius)) * RADIANS_TO_DEGREES;
      float azimuth = (std::atanf(local[0]/local[2])) * RADIANS_TO_DEGREES;
      hit->SetCoordinates(pxr::GfVec3f(_radius, polar, azimuth));
      return true;
    }
  }
  return false;
}

bool 
Cone::Closest(const pxr::GfVec3f& point, Location* hit,
  double maxDistance, double* minDistance) const
{
  pxr::GfVec3f local = GetInverseMatrix().Transform(point).GetNormalized() * _radius;
  pxr::GfVec3f closest = GetMatrix().Transform(local);  
  float distance = (point - closest).GetLength();
  if(distance < maxDistance && distance < *minDistance) {
    *minDistance = distance;
    // store spherical coordinates
    float polar = (-std::acosf(local[2]/_radius)) * RADIANS_TO_DEGREES;
    float azimuth = (std::atanf(local[0]/local[2])) * RADIANS_TO_DEGREES;
    hit->SetCoordinates(pxr::GfVec3f(_radius, polar, azimuth));
    return true;
  }
  return false;
}

Geometry::DirtyState 
Cone::_Sync(const pxr::GfMatrix4d& matrix, const pxr::UsdTimeCode& time)
{
  short radiusDirty  = GetAttributeValue<double>(pxr::UsdGeomTokens->radius, time, &_radius);
  return Geometry::DirtyState::CLEAN;
}

void 
Cone::_Inject(const pxr::GfMatrix4d& parent, const pxr::UsdTimeCode& time)
{
  pxr::UsdGeomCone usdCone(_prim);
  usdCone.CreateHeightAttr().Set(_height, time);
  usdCone.CreateRadiusAttr().Set(_radius, time);
  usdCone.CreateAxisAttr().Set(_axis, time);
}

//-------------------------------------------------------------------------------------------------
// Cylinder Implicit Geometry
//-------------------------------------------------------------------------------------------------
Cylinder::Cylinder(const pxr::GfMatrix4d& xfo)
  : Geometry(Geometry::CYLINDER, xfo)
  , _radius(0.5f)
  , _height(1.f)
  , _axis(pxr::UsdGeomTokens->y)
{
}

Cylinder::Cylinder(const pxr::UsdGeomCylinder& cylinder, const pxr::GfMatrix4d& world)
  : Geometry(cylinder.GetPrim(), world)
{
  pxr::UsdAttribute radiusAttr = cylinder.GetRadiusAttr();
  radiusAttr.Get(&_radius, pxr::UsdTimeCode::Default());

  pxr::UsdAttribute heightAttr = cylinder.GetHeightAttr();
  heightAttr.Get(&_height, pxr::UsdTimeCode::Default());

  pxr::UsdAttribute axisAttr = cylinder.GetAxisAttr();
  axisAttr.Get(&_axis, pxr::UsdTimeCode::Default());

}

bool 
Cylinder::Raycast(const pxr::GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{ 
  pxr::GfRay invRay(ray);
  invRay.Transform(GetInverseMatrix());
  double enterDistance, exitDistance;
  if(ray.Intersect(pxr::GfVec3d(0.0), _radius, &enterDistance, &exitDistance)) {
    pxr::GfVec3f local(ray.GetPoint(enterDistance));
    pxr::GfVec3f world(GetMatrix().Transform(local));
    float distance = (ray.GetStartPoint() - world).GetLength();
    if(distance < maxDistance && distance < *minDistance) {
      *minDistance = distance;
      // store spherical coordinates
      float polar = (-std::acosf(local[2]/_radius)) * RADIANS_TO_DEGREES;
      float azimuth = (std::atanf(local[0]/local[2])) * RADIANS_TO_DEGREES;
      hit->SetCoordinates(pxr::GfVec3f(_radius, polar, azimuth));
      return true;
    }
  }
  return false;
}

bool 
Cylinder::Closest(const pxr::GfVec3f& point, Location* hit,
  double maxDistance, double* minDistance) const
{
  pxr::GfVec3f local = GetInverseMatrix().Transform(point).GetNormalized() * _radius;
  pxr::GfVec3f closest = GetMatrix().Transform(local);  
  float distance = (point - closest).GetLength();
  if(distance < maxDistance && distance < *minDistance) {
    *minDistance = distance;
    // store spherical coordinates
    float polar = (-std::acosf(local[2]/_radius)) * RADIANS_TO_DEGREES;
    float azimuth = (std::atanf(local[0]/local[2])) * RADIANS_TO_DEGREES;
    hit->SetCoordinates(pxr::GfVec3f(_radius, polar, azimuth));
    return true;
  }
  return false;
}

Geometry::DirtyState 
Cylinder::_Sync(const pxr::GfMatrix4d& matrix, const pxr::UsdTimeCode& time)
{
  return Geometry::DirtyState::CLEAN;
}

void 
Cylinder::_Inject(const pxr::GfMatrix4d& parent, const pxr::UsdTimeCode& time)
{
  pxr::UsdGeomCylinder usdCylinder(_prim);
  usdCylinder.CreateHeightAttr().Set(_height, time);
  usdCylinder.CreateRadiusAttr().Set(_radius, time);
  usdCylinder.CreateAxisAttr().Set(_axis, time);
}

//-------------------------------------------------------------------------------------------------
// Capsule Implicit Geometry
//-------------------------------------------------------------------------------------------------
Capsule::Capsule(const pxr::GfMatrix4d& xfo)
  : Geometry(Geometry::CAPSULE, xfo)
  , _radius(0.1f)
  , _height(1.f)
  , _axis(pxr::UsdGeomTokens->y)
{
}

Capsule::Capsule(const pxr::UsdGeomCapsule& capsule, const pxr::GfMatrix4d& world)
  : Geometry(capsule.GetPrim(), world)
{
  pxr::UsdAttribute radiusAttr = capsule.GetRadiusAttr();
  radiusAttr.Get(&_radius, pxr::UsdTimeCode::Default());

  pxr::UsdAttribute heightAttr = capsule.GetHeightAttr();
  heightAttr.Get(&_height, pxr::UsdTimeCode::Default());

  pxr::UsdAttribute axisAttr = capsule.GetAxisAttr();
  axisAttr.Get(&_axis, pxr::UsdTimeCode::Default());

}

bool Capsule::Raycast(const pxr::GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{ 
  return false;
}

bool Capsule::Closest(const pxr::GfVec3f& point, Location* hit,
  double maxDistance, double* minDistance) const
{
  return false;
}


Geometry::DirtyState 
Capsule::_Sync(const pxr::GfMatrix4d& matrix, const pxr::UsdTimeCode& time)
{
  size_t state = GetAttributeValue<pxr::TfToken>(pxr::UsdGeomTokens->axis, time, &_axis);
  state |= GetAttributeValue<double>(pxr::UsdGeomTokens->width, time, &_radius);
  state |= GetAttributeValue<double>(pxr::UsdGeomTokens->length, time, &_height);

  return (Geometry::DirtyState)state;
}

void 
Capsule::_Inject(const pxr::GfMatrix4d& parent,
  const pxr::UsdTimeCode& time)
{
  pxr::UsdGeomCapsule usdCapsule(_prim);
  usdCapsule.CreateHeightAttr().Set(_height, time);
  usdCapsule.CreateRadiusAttr().Set(_radius, time);
  usdCapsule.CreateAxisAttr().Set(_axis, time);
}

float 
Capsule::SignedDistance(const pxr::GfVec3f& point) const
{
  pxr::GfVec3f a, b, p;
  p = _invMatrix.Transform(point);
  if (_axis == pxr::UsdGeomTokens->x) {
    a = pxr::GfVec3f(-_height * 0.5f, 0.f, 0.f);
    b = pxr::GfVec3f(_height * 0.5f, 0.f, 0.f);
  }
  else if (_axis == pxr::UsdGeomTokens->y) {
    a = pxr::GfVec3f(0.f, -_height * 0.5f, 0.f);
    b = pxr::GfVec3f(0.f, _height * 0.5f, 0.f);
  }
  else if (_axis == pxr::UsdGeomTokens->z) {
    a = pxr::GfVec3f(0.f, 0.f, -_height * 0.5f);
    b = pxr::GfVec3f(0.f, 0.f, _height * 0.5f);
  }
    
  pxr::GfVec3f pa = p - a;
  pxr::GfVec3f ba = b - a;
  float h = pxr::GfClamp( pxr::GfDot(pa, ba)/pxr::GfDot(ba, ba), 0.f, 1.f );

  return (pa - ba * h).GetLength() - _radius;
}

JVR_NAMESPACE_CLOSE_SCOPE