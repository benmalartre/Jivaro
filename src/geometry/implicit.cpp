// IMPLICIT GEOMETRIES (SPHERE, CUBE, CONE, CAPSULE)
//--------------------------------------------------

#include "../geometry/implicit.h"
#include "../geometry/utils.h"
#include <pxr/base/gf/ray.h>

JVR_NAMESPACE_OPEN_SCOPE

Sphere::Sphere()
  : Geometry(Geometry::SPHERE, pxr::GfMatrix4d(1.0))
{
  _initialized = false;
}

Sphere::Sphere(const Points* other, bool normalize)
  : Geometry(other, Geometry::SPHERE, normalize)
{
  size_t numPoints = _positions.size();
  _initialized = true;

  _normals = other->_normals;

  _radius.resize(numPoints);
  memcpy(&_radius[0], &other->_radius[0], numPoints * sizeof(float));
}

Sphere::Sphere(const pxr::UsdGeomSphere& sphere, const pxr::GfMatrix4d& world)
  : Geometry(Geometry::SPHERE, world)
{

  pxr::UsdAttribute pointsAttr = points.GetPointsAttr();
  pointsAttr.Get(&_positions, pxr::UsdTimeCode::Default());

  pxr::UsdAttribute normalsAttr = points.GetNormalsAttr();
  if (normalsAttr.IsDefined() && normalsAttr.HasAuthoredValue())
    normalsAttr.Get(&_normals, pxr::UsdTimeCode::Default());

  pxr::UsdAttribute widthsAttr = points.GetWidthsAttr();
  if (widthsAttr.IsDefined() && widthsAttr.HasAuthoredValue())
    widthsAttr.Get(&_radius, pxr::UsdTimeCode::Default());
}


void Points::Init(
  const pxr::VtArray<pxr::GfVec3f>& positions, 
  const pxr::VtArray<float>& radius)
{
  _radius = radius;
  _positions = positions;
  _normals = positions;
}

void Points::Update(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _positions = positions;
}

void Points::Update(const pxr::VtArray<float>& radius)
{
  _radius = radius;
}

void Points::Update(const pxr::VtArray<pxr::GfVec3f>& positions,
  const pxr::VtArray<float>& radius)
{
  _positions = positions;
  _radius = radius;
}

Point Points::Get(uint32_t index)
{
  if(index < _positions.size())
    return Point(index, index < _radius.size() ? _radius[index] : 1.f);   
  else
    return Point(); 
}

JVR_NAMESPACE_CLOSE_SCOPE