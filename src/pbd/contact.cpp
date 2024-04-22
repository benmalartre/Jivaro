#include "../pbd/contact.h"
#include "../pbd/particles.h"

JVR_NAMESPACE_OPEN_SCOPE

int _GetElementDimensionFromGeometry(Geometry* geometry)
{
  const short type = geometry->GetType();

  // implicit geometries
  if(type < Geometry::POINT) return 0;

  size_t n = 0;
  switch(geometry->GetType()) {
    case Geometry::POINT:
      return 1;

    case Geometry::CURVE:
      return 2

    case Geometry::MESH:
      return 3
  }
  return -1;
}

void Contact::Init(Geometry* geometry, Particles* particles, size_t index)
{
  size_t n = _GetElementDimensionFromGeometry(geometry);

  if(n == 0) {
    if(geometry->GetType() == Geometry::PLANE) {
      Plane* plane = (Plane*)geometry;
      _normal = plane->GetNormal;
    }
  } else {
    _normal = GetNormal(geometry->GetPositionsCPtr(), 
    &_elemId[0], n, geometry->GetMatrix());

    _relV = particles->_velocity[index] - geometry->GetVelocity(1.f);
    _nrmV = pxr::GfDot(_relV, _normal);
  }
}

void Contact::Update(Geometry* geometry, Particles* particles, size_t index)
{
  size_t n = _GetElementDimensionFromGeometry(geometry);

  if(n == 0) {
     switch(geometry->GetType()) {
      case Geometry::PLANE:
        Plane* plane = (Plane*)geometry;
        _normal = plane->GetNormal();
        _d = -pxr::GfDot(_normal, particles->_predicted[index] - _position) - particles->_radius[index];
        break;

     }
  } else {
    const pxr::GfVec3f position = GetPosition(geometry->GetPositionsCPtr(), 
      &_elemId[0], n, geometry->GetMatrix());

    _normal = GetNormal(geometry->GetPositionsCPtr(), &_elemId[0], n, geometry->GetMatrix());

    const pxr::GfVec3f delta = position - particles->_predicted[index];

    _d = -pxr::GfDot(delta, _normal);
  }
}

JVR_NAMESPACE_CLOSE_SCOPE