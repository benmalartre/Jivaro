//--------------------------------------------------
// Implicit Geometries
// 
// Plane
// Sphere
// Cube
// Cone
// Capsule
//--------------------------------------------------

#include "../geometry/implicit.h"
#include "../geometry/utils.h"
#include "../geometry/intersection.h"
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/range3d.h>

JVR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------------------------------------------------
// Plane Implicit Geometry
//-------------------------------------------------------------------------------------------------
Plane::Plane(const pxr::GfMatrix4d& xfo)
  : Geometry(Geometry::PLANE, xfo)
{
  _normal = pxr::GfVec3f(0.f, 1.f, 0.f);
  _width = 100.f;
  _length = 100.f;
  _doubleSided = false;
}

Plane::Plane(const Plane* other, bool normalize)
  : Geometry(other, Geometry::PLANE, normalize)
{
  _normal = other->_normal;
  _width = other->_width;
  _length = other->_length;
  _doubleSided = other->_doubleSided;
}

Plane::Plane(const pxr::UsdGeomPlane& plane, const pxr::GfMatrix4d& world)
  : Geometry(Geometry::PLANE, world)
{
  pxr::TfToken axis;
  pxr::UsdAttribute axisAttr = plane.GetAxisAttr();
  axisAttr.Get(&axis, pxr::UsdTimeCode::Default());

  if (axis == pxr::UsdGeomTokens->x)
    _normal = pxr::GfVec3f(1.f, 0.f, 0.f);
  else if (axis == pxr::UsdGeomTokens->y)
    _normal = pxr::GfVec3f(0.f, 1.f, 0.f);
  else
    _normal = pxr::GfVec3f(0.f, 0.f, 1.f);

  pxr::UsdAttribute widthAttr = plane.GetWidthAttr();
  widthAttr.Get(&_width, pxr::UsdTimeCode::Default());
  pxr::UsdAttribute lengthAttr = plane.GetLengthAttr();
  lengthAttr.Get(&_length, pxr::UsdTimeCode::Default());
  pxr::UsdAttribute doubleSidedAttr = plane.GetDoubleSidedAttr();
  doubleSidedAttr.Get(&_doubleSided, pxr::UsdTimeCode::Default());
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

//-------------------------------------------------------------------------------------------------
// Sphere Implicit Geometry
//-------------------------------------------------------------------------------------------------
Sphere::Sphere(const pxr::GfMatrix4d& xfo)
  : Geometry(Geometry::SPHERE, xfo)
{
}

Sphere::Sphere(const Sphere* other, bool normalize)
  : Geometry(other, Geometry::SPHERE, normalize)
{
  _radius = other->_radius;
}

Sphere::Sphere(const pxr::UsdGeomSphere& sphere, const pxr::GfMatrix4d& world)
  : Geometry(Geometry::SPHERE, world)
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

//-------------------------------------------------------------------------------------------------
// Cube Implicit Geometry
//-------------------------------------------------------------------------------------------------
Cube::Cube(const pxr::GfMatrix4d& xfo)
  : Geometry(Geometry::CUBE, xfo)
{
  _size = 1.f;
}

Cube::Cube(const Cube* other, bool normalize)
  : Geometry(other, Geometry::CUBE, normalize)
{
  _size = other->_size;
}

Cube::Cube(const pxr::UsdGeomCube& cube, const pxr::GfMatrix4d& world)
  : Geometry(Geometry::CUBE, world)
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

Cone::Cone(const Cone* other, bool normalize)
  : Geometry(other, Geometry::CONE, normalize)
{
  _radius = other->_radius;
  _height = other->_height;
  _axis = other->_axis;
}

Cone::Cone(const pxr::UsdGeomCone& cone, const pxr::GfMatrix4d& world)
  : Geometry(Geometry::CONE, world)
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

Capsule::Capsule(const Capsule* other, bool normalize)
  : Geometry(other, Geometry::CAPSULE, normalize)
{
  _radius = other->_radius;
  _height = other->_height;
  _axis = other->_axis;
}

Capsule::Capsule(const pxr::UsdGeomCapsule& capsule, const pxr::GfMatrix4d& world)
  : Geometry(Geometry::CAPSULE, world)
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
JVR_NAMESPACE_CLOSE_SCOPE