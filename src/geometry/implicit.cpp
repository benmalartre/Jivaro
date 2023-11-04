// IMPLICIT GEOMETRIES (SPHERE, CUBE, CONE, CAPSULE)
//--------------------------------------------------

#include "../geometry/implicit.h"
#include "../geometry/utils.h"
#include "../geometry/intersection.h"
#include <pxr/base/gf/ray.h>

JVR_NAMESPACE_OPEN_SCOPE

Sphere::Sphere()
  : Geometry(Geometry::SPHERE, pxr::GfMatrix4d(1.0))
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
Sphere::Raycast(const pxr::GfRay& ray, Hit* hit,
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

bool Sphere::Closest(const pxr::GfVec3f& point, Hit* hit,
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

Cube::Cube()
  : Geometry(Geometry::CUBE, pxr::GfMatrix4d(1.0))
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

bool 
Cube::Raycast(const pxr::GfRay& ray, Hit* hit,
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
Cube::Closest(const pxr::GfVec3f& point, Hit* hit,
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

Cone::Cone()
  : Geometry(Geometry::CONE, pxr::GfMatrix4d(1.0))
{
}

Cone::Cone(const Cone* other, bool normalize)
  : Geometry(other, Geometry::CONE, normalize)
{
  _radius = other->_radius;
  _height = other->_height;
}

Cone::Cone(const pxr::UsdGeomCone& cone, const pxr::GfMatrix4d& world)
  : Geometry(Geometry::CONE, world)
{
  pxr::UsdAttribute radiusAttr = cone.GetRadiusAttr();
  radiusAttr.Get(&_radius, pxr::UsdTimeCode::Default());

  pxr::UsdAttribute heightAttr = cone.GetHeightAttr();
  heightAttr.Get(&_height, pxr::UsdTimeCode::Default());

}

bool 
Cone::Raycast(const pxr::GfRay& ray, Hit* hit,
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
Cone::Closest(const pxr::GfVec3f& point, Hit* hit,
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

JVR_NAMESPACE_CLOSE_SCOPE