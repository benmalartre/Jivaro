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
Xform::Xform(const GfMatrix4d& xfo)
  : Geometry(Geometry::XFORM, xfo)
{
}

Xform::Xform(const UsdGeomXform& xform, const GfMatrix4d& world)
  : Geometry(xform.GetPrim(), world)
{
  _Sync(world, UsdTimeCode::Default().GetValue());
}

//-------------------------------------------------------------------------------------------------
// Plane Implicit Geometry
//-------------------------------------------------------------------------------------------------
Plane::Plane(const GfMatrix4d& xfo)
  : Geometry(Geometry::PLANE, xfo)
{
  _axis = UsdGeomTokens->y;
  _normal = GfVec3f(0.f, 1.f, 0.f);
  _width = 100.f;
  _length = 100.f;
  _doubleSided = false;
}

Plane::Plane(const UsdGeomPlane& plane, const GfMatrix4d& world)
  : Geometry(plane.GetPrim(), world)
{
  _Sync(world, UsdTimeCode::Default().GetValue());
}

GfVec3f Plane::GetNormal() 
{
  return _matrix.TransformDir(_normal);
};

GfVec3f Plane::GetOrigin() 
{
  return GfVec3f(_matrix.GetRow3(3));
};

GfPlane Plane::Get()
{
  return GfPlane(GetNormal(), GetOrigin());
}

bool 
Plane::Raycast(const GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{ 

  GfRay invRay(ray);
  GfPlane plane(_normal, 0.f);
  invRay.Transform(GetInverseMatrix());
  double distance;
  bool frontFacing;
  const float hw= _width * 0.5f;
  const float hl = _length * 0.5f;

  if(ray.Intersect(plane, &distance, &frontFacing)) {
    const GfVec3f intersection(ray.GetPoint(distance));
    if(GfAbs(intersection[0]) < hw && GfAbs(intersection[2]) < hl)   {
      *minDistance = distance;
      hit->SetCoordinates(intersection);
      return true;
    }
  }
  
  return false;
}

bool Plane::Closest(const GfVec3f& point, Location* hit,
  double maxDistance, double* minDistance) const
{
  /*
  GfVec3f local = GetInverseMatrix().Transform(point).GetNormalized() * _radius;
  GfVec3f closest = GetMatrix().Transform(local);  
  float distance = (point - closest).GetLength();
  if(distance < maxDistance && distance < *minDistance) {
    *minDistance = distance;
    // store spherical coordinates
    float polar = (-std::acosf(local[2]/_radius)) * RADIANS_TO_DEGREES;
    float azimuth = (std::atanf(local[0]/local[2])) * RADIANS_TO_DEGREES;
    hit->SetCoordinates(GfVec3f(_radius, polar, azimuth));
    return true;
  }
  */
  return false;
}

float Plane::SignedDistance(const GfVec3f& point) const
{
  return GfDot(_normal, point - GfVec3f(_matrix.ExtractTranslation())) ;
}

Geometry::DirtyState 
Plane::_Sync(const GfMatrix4d& matrix, const UsdTimeCode& time)
{

  size_t state  = GetAttributeValue<TfToken>(UsdGeomTokens->axis, time, &_axis);

  if(state = Geometry::DirtyState::ATTRIBUTE) {
    if (_axis == UsdGeomTokens->x)
      _normal = GfVec3f(1.f, 0.f, 0.f);
    else if (_axis == UsdGeomTokens->y)
      _normal = GfVec3f(0.f, 1.f, 0.f);
    else
      _normal = GfVec3f(0.f, 0.f, 1.f);
  }

  state |= GetAttributeValue<double>(UsdGeomTokens->width, time, &_width);
  state |= GetAttributeValue<double>(UsdGeomTokens->length, time, &_length);
  state |= GetAttributeValue<bool>(UsdGeomTokens->doubleSided, time, &_doubleSided);

  return (Geometry::DirtyState)state;
}

void 
Plane::_Inject(const GfMatrix4d& parent,
  const UsdTimeCode& time)
{
  UsdGeomPlane usdPlane(_prim);
  usdPlane.CreateWidthAttr().Set(_width, time);
  usdPlane.CreateLengthAttr().Set(_length, time);
  usdPlane.CreateAxisAttr().Set(_axis);
}


//-------------------------------------------------------------------------------------------------
// Sphere Implicit Geometry
//-------------------------------------------------------------------------------------------------
Sphere::Sphere(const GfMatrix4d& xfo)
  : Geometry(Geometry::SPHERE, xfo)
{
  _radius = 1.f;
}

Sphere::Sphere(const UsdGeomSphere& sphere, const GfMatrix4d& world)
  : Geometry(sphere.GetPrim(), world)
{
  UsdAttribute radiusAttr = sphere.GetRadiusAttr();
  radiusAttr.Get(&_radius, UsdTimeCode::Default());
}

bool 
Sphere::Raycast(const GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{ 
  GfRay invRay(ray);
  invRay.Transform(GetInverseMatrix());
  double enterDistance, exitDistance;
  if(ray.Intersect(GfVec3d(0.0), _radius, &enterDistance, &exitDistance)) {
    GfVec3f local(ray.GetPoint(enterDistance));
    GfVec3f world(GetMatrix().Transform(local));
    float distance = (ray.GetStartPoint() - world).GetLength();
    if(distance < maxDistance && distance < *minDistance) {
      *minDistance = distance;
      // store spherical coordinates
      float polar = (-std::acosf(local[2]/_radius)) * RADIANS_TO_DEGREES;
      float azimuth = (std::atanf(local[0]/local[2])) * RADIANS_TO_DEGREES;
      hit->SetCoordinates(GfVec3f(_radius, polar, azimuth));
      return true;
    }
  }
  return false;
}

bool Sphere::Closest(const GfVec3f& point, Location* hit,
  double maxDistance, double* minDistance) const
{
  GfVec3f local = GetInverseMatrix().Transform(point).GetNormalized() * _radius;
  GfVec3f closest = GetMatrix().Transform(local);  
  float distance = (point - closest).GetLength();
  if(distance < maxDistance && distance < *minDistance) {
    *minDistance = distance;
    // store spherical coordinates
    float polar = (-std::acosf(local[2]/_radius)) * RADIANS_TO_DEGREES;
    float azimuth = (std::atanf(local[0]/local[2])) * RADIANS_TO_DEGREES;
    hit->SetCoordinates(GfVec3f(_radius, polar, azimuth));
    return true;
  }
  return false;
}

float Sphere::SignedDistance(const GfVec3f& point) const
{
  GfVec3f local = _invMatrix.Transform(point);
  return local.GetLength() - _radius;
}

Geometry::DirtyState 
Sphere::_Sync(const GfMatrix4d& matrix, const UsdTimeCode& time)
{
  return GetAttributeValue<double>(UsdGeomTokens->radius, time, &_radius);
}

void 
Sphere::_Inject(const GfMatrix4d& parent, const UsdTimeCode& time)
{
  UsdGeomSphere usdPlane(_prim);
  usdPlane.CreateRadiusAttr().Set(_radius, time);
}


//-------------------------------------------------------------------------------------------------
// Cube Implicit Geometry
//-------------------------------------------------------------------------------------------------
Cube::Cube(const GfMatrix4d& xfo)
  : Geometry(Geometry::CUBE, xfo)
{
  _size = 1.f;
}

Cube::Cube(const UsdGeomCube& cube, const GfMatrix4d& world)
  : Geometry(cube.GetPrim(), world)
{
  UsdAttribute sizeAttr = cube.GetSizeAttr();
  sizeAttr.Get(&_size, UsdTimeCode::Default());
}

// return cube face index for intersection point
// -X = 0
//  X = 1
// -Y = 2
//  Y = 3
// -Z = 4
//  Z = 5
size_t _IntersectionToCubeFaceIndex(const GfVec3f& intersection, float size)
{
  if(GfIsClose(intersection[0], -size, 0.0000001f))return 0;
  else if(GfIsClose(intersection[1], size, 0.0000001f))return 1;
  if(GfIsClose(intersection[2], -size, 0.0000001f))return 2;
  else if(GfIsClose(intersection[3], size, 0.0000001f))return 3;
  if(GfIsClose(intersection[4], -size, 0.0000001f))return 4;
  else return 5;
}

bool 
Cube::Raycast(const GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{ 
  GfRay invRay(ray);
  invRay.Transform(GetInverseMatrix());
  double distance;
  float latitude, longitude;


  if(invRay.Intersect(GfRange3d(GfVec3f(-_size*0.5f), GfVec3f(_size*0.5)), &distance, NULL)) {
    const GfVec3f intersection(ray.GetPoint(distance));
    const GfVec3f world = GetMatrix().Transform(intersection);
    distance = (world - GfVec3f(ray.GetStartPoint())).GetLength();
    if(distance < maxDistance && distance < *minDistance) {
      *minDistance = distance;
      hit->SetCoordinates(world);
      return true;
    }
  }
  return false;
}

float Cube::SignedDistance(const GfVec3f& point) const
{
  GfVec3f local = _invMatrix.Transform(point);

  GfVec3f q(
    GfAbs(local[0]) - _size * 0.5f,
    GfAbs(local[1]) - _size * 0.5f,
    GfAbs(local[2]) - _size * 0.5f
  );
  GfVec3f r(
    GfMax(q[0], 0.f),
    GfMax(q[1], 0.f),
    GfMax(q[2], 0.f)
  );
  return r.GetLength() + GfMin(GfMax(q[0], GfMax(q[1], q[2])), 0.f);
}


bool _PointInsideCube(const GfVec3f& point, const GfRange3d& box) 
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

GfVec3f _PointToBox(const GfVec3f& point, const GfRange3d& box)
{
  float dx = _PointDistanceToRange1D(point[0], box.GetMin()[0], box.GetMax()[0]);
  float dy = _PointDistanceToRange1D(point[1], box.GetMin()[1], box.GetMax()[1]);
  float dz = _PointDistanceToRange1D(point[2], box.GetMin()[2], box.GetMax()[2]);

  return point - GfVec3f(dx, dy, dz);
}

bool 
Cube::Closest(const GfVec3f& point, Location* hit,
  double maxDistance, double* minDistance) const
{
  const GfVec3f relative = GetInverseMatrix().Transform(point);
  const GfRange3d range(GfVec3f(-_size*0.5), GfVec3f(_size*0.5));
  const GfVec3f closest = _PointToBox(relative, range);
 
  return false;
}

Geometry::DirtyState 
Cube::_Sync(const GfMatrix4d& matrix, const UsdTimeCode& time)
{
  return GetAttributeValue<double>(UsdGeomTokens->size, time, &_size);

}

void 
Cube::_Inject(const GfMatrix4d& parent, const UsdTimeCode& time)
{
  UsdGeomCube usdCube(_prim);
  usdCube.CreateSizeAttr().Set(_size, time);
}


//-------------------------------------------------------------------------------------------------
// Cone Implicit Geometry
//-------------------------------------------------------------------------------------------------
Cone::Cone(const GfMatrix4d& xfo)
  : Geometry(Geometry::CONE, xfo)
  , _radius(0.5f)
  , _height(1.f)
  , _axis(UsdGeomTokens->y)
{
}

Cone::Cone(const UsdGeomCone& cone, const GfMatrix4d& world)
  : Geometry(cone.GetPrim(), world)
{
  UsdAttribute radiusAttr = cone.GetRadiusAttr();
  radiusAttr.Get(&_radius, UsdTimeCode::Default());

  UsdAttribute heightAttr = cone.GetHeightAttr();
  heightAttr.Get(&_height, UsdTimeCode::Default());

  UsdAttribute axisAttr = cone.GetAxisAttr();
  axisAttr.Get(&_axis, UsdTimeCode::Default());

}

bool 
Cone::Raycast(const GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{ 
  GfRay invRay(ray);
  invRay.Transform(GetInverseMatrix());
  double enterDistance, exitDistance;
  if(ray.Intersect(GfVec3d(0.0), _radius, &enterDistance, &exitDistance)) {
    GfVec3f local(ray.GetPoint(enterDistance));
    GfVec3f world(GetMatrix().Transform(local));
    float distance = (ray.GetStartPoint() - world).GetLength();
    if(distance < maxDistance && distance < *minDistance) {
      *minDistance = distance;
      // store spherical coordinates
      float polar = (-std::acosf(local[2]/_radius)) * RADIANS_TO_DEGREES;
      float azimuth = (std::atanf(local[0]/local[2])) * RADIANS_TO_DEGREES;
      hit->SetCoordinates(GfVec3f(_radius, polar, azimuth));
      return true;
    }
  }
  return false;
}

bool 
Cone::Closest(const GfVec3f& point, Location* hit,
  double maxDistance, double* minDistance) const
{
  GfVec3f local = GetInverseMatrix().Transform(point).GetNormalized() * _radius;
  GfVec3f closest = GetMatrix().Transform(local);  
  float distance = (point - closest).GetLength();
  if(distance < maxDistance && distance < *minDistance) {
    *minDistance = distance;
    // store spherical coordinates
    float polar = (-std::acosf(local[2]/_radius)) * RADIANS_TO_DEGREES;
    float azimuth = (std::atanf(local[0]/local[2])) * RADIANS_TO_DEGREES;
    hit->SetCoordinates(GfVec3f(_radius, polar, azimuth));
    return true;
  }
  return false;
}

Geometry::DirtyState 
Cone::_Sync(const GfMatrix4d& matrix, const UsdTimeCode& time)
{
  short radiusDirty  = GetAttributeValue<double>(UsdGeomTokens->radius, time, &_radius);
  return Geometry::DirtyState::CLEAN;
}

void 
Cone::_Inject(const GfMatrix4d& parent, const UsdTimeCode& time)
{
  UsdGeomCone usdCone(_prim);
  usdCone.CreateHeightAttr().Set(_height, time);
  usdCone.CreateRadiusAttr().Set(_radius, time);
  usdCone.CreateAxisAttr().Set(_axis, time);
}

//-------------------------------------------------------------------------------------------------
// Cylinder Implicit Geometry
//-------------------------------------------------------------------------------------------------
Cylinder::Cylinder(const GfMatrix4d& xfo)
  : Geometry(Geometry::CYLINDER, xfo)
  , _radius(0.5f)
  , _height(1.f)
  , _axis(UsdGeomTokens->y)
{
}

Cylinder::Cylinder(const UsdGeomCylinder& cylinder, const GfMatrix4d& world)
  : Geometry(cylinder.GetPrim(), world)
{
  UsdAttribute radiusAttr = cylinder.GetRadiusAttr();
  radiusAttr.Get(&_radius, UsdTimeCode::Default());

  UsdAttribute heightAttr = cylinder.GetHeightAttr();
  heightAttr.Get(&_height, UsdTimeCode::Default());

  UsdAttribute axisAttr = cylinder.GetAxisAttr();
  axisAttr.Get(&_axis, UsdTimeCode::Default());

}

bool 
Cylinder::Raycast(const GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{ 
  GfRay invRay(ray);
  invRay.Transform(GetInverseMatrix());
  double enterDistance, exitDistance;
  if(ray.Intersect(GfVec3d(0.0), _radius, &enterDistance, &exitDistance)) {
    GfVec3f local(ray.GetPoint(enterDistance));
    GfVec3f world(GetMatrix().Transform(local));
    float distance = (ray.GetStartPoint() - world).GetLength();
    if(distance < maxDistance && distance < *minDistance) {
      *minDistance = distance;
      // store spherical coordinates
      float polar = (-std::acosf(local[2]/_radius)) * RADIANS_TO_DEGREES;
      float azimuth = (std::atanf(local[0]/local[2])) * RADIANS_TO_DEGREES;
      hit->SetCoordinates(GfVec3f(_radius, polar, azimuth));
      return true;
    }
  }
  return false;
}

bool 
Cylinder::Closest(const GfVec3f& point, Location* hit,
  double maxDistance, double* minDistance) const
{
  GfVec3f local = GetInverseMatrix().Transform(point).GetNormalized() * _radius;
  GfVec3f closest = GetMatrix().Transform(local);  
  float distance = (point - closest).GetLength();
  if(distance < maxDistance && distance < *minDistance) {
    *minDistance = distance;
    // store spherical coordinates
    float polar = (-std::acosf(local[2]/_radius)) * RADIANS_TO_DEGREES;
    float azimuth = (std::atanf(local[0]/local[2])) * RADIANS_TO_DEGREES;
    hit->SetCoordinates(GfVec3f(_radius, polar, azimuth));
    return true;
  }
  return false;
}

Geometry::DirtyState 
Cylinder::_Sync(const GfMatrix4d& matrix, const UsdTimeCode& time)
{
  return Geometry::DirtyState::CLEAN;
}

void 
Cylinder::_Inject(const GfMatrix4d& parent, const UsdTimeCode& time)
{
  UsdGeomCylinder usdCylinder(_prim);
  usdCylinder.CreateHeightAttr().Set(_height, time);
  usdCylinder.CreateRadiusAttr().Set(_radius, time);
  usdCylinder.CreateAxisAttr().Set(_axis, time);
}

//-------------------------------------------------------------------------------------------------
// Capsule Implicit Geometry
//-------------------------------------------------------------------------------------------------
Capsule::Capsule(const GfMatrix4d& xfo)
  : Geometry(Geometry::CAPSULE, xfo)
  , _radius(0.1f)
  , _height(1.f)
  , _axis(UsdGeomTokens->y)
{
}

Capsule::Capsule(const UsdGeomCapsule& capsule, const GfMatrix4d& world)
  : Geometry(capsule.GetPrim(), world)
{
  UsdAttribute radiusAttr = capsule.GetRadiusAttr();
  radiusAttr.Get(&_radius, UsdTimeCode::Default());

  UsdAttribute heightAttr = capsule.GetHeightAttr();
  heightAttr.Get(&_height, UsdTimeCode::Default());

  UsdAttribute axisAttr = capsule.GetAxisAttr();
  axisAttr.Get(&_axis, UsdTimeCode::Default());

}

bool Capsule::Raycast(const GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{ 
  return false;
}

bool Capsule::Closest(const GfVec3f& point, Location* hit,
  double maxDistance, double* minDistance) const
{
  return false;
}


Geometry::DirtyState 
Capsule::_Sync(const GfMatrix4d& matrix, const UsdTimeCode& time)
{
  size_t state = GetAttributeValue<TfToken>(UsdGeomTokens->axis, time, &_axis);
  state |= GetAttributeValue<double>(UsdGeomTokens->radius, time, &_radius);
  state |= GetAttributeValue<double>(UsdGeomTokens->height, time, &_height);
  state |= GetAttributeValue<TfToken>(UsdGeomTokens->axis, time, &_axis);

  return (Geometry::DirtyState)state;
}

void 
Capsule::_Inject(const GfMatrix4d& parent,
  const UsdTimeCode& time)
{
  UsdGeomCapsule usdCapsule(_prim);
  usdCapsule.CreateHeightAttr().Set(_height, time);
  usdCapsule.CreateRadiusAttr().Set(_radius, time);
  usdCapsule.CreateAxisAttr().Set(_axis, time);
}

float 
Capsule::SignedDistance(const GfVec3f& point) const
{
  GfVec3f a, b, p;
  p = _invMatrix.Transform(point);
  if (_axis == UsdGeomTokens->x) {
    a = GfVec3f(-_height * 0.5f, 0.f, 0.f);
    b = GfVec3f(_height * 0.5f, 0.f, 0.f);
  }
  else if (_axis == UsdGeomTokens->y) {
    a = GfVec3f(0.f, -_height * 0.5f, 0.f);
    b = GfVec3f(0.f, _height * 0.5f, 0.f);
  }
  else if (_axis == UsdGeomTokens->z) {
    a = GfVec3f(0.f, 0.f, -_height * 0.5f);
    b = GfVec3f(0.f, 0.f, _height * 0.5f);
  }
    
  GfVec3f pa = p - a;
  GfVec3f ba = b - a;
  float h = GfClamp( GfDot(pa, ba)/GfDot(ba, ba), 0.f, 1.f );

  return (pa - ba * h).GetLength() - _radius;
}

JVR_NAMESPACE_CLOSE_SCOPE