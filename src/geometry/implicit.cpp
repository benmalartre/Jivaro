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
  _initialized = false;
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
    hit->SetCoordinates(pxr::GfVec3f(_radius, 0.f, 0.f));
    return true;
  }
  return false;
}

bool Sphere::Closest(const pxr::GfVec3f& point, Hit* hit,
  double maxDistance, double* minDistance) const
{
  return false;
}

JVR_NAMESPACE_CLOSE_SCOPE