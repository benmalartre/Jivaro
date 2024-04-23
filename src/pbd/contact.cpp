#include "../geometry/geometry.h"
#include "../geometry/implicit.h"
#include "../geometry/deformable.h"
#include "../pbd/contact.h"
#include "../pbd/particle.h"


JVR_NAMESPACE_OPEN_SCOPE

int _GetElementDimensionFromGeometry(Geometry* geometry)
{

  const short type = geometry->GetType();

  if(type < Geometry::POINT) return 0;

  switch(geometry->GetType()) {
    case Geometry::POINT:
      return 1;

    case Geometry::CURVE:
      return 2;

    case Geometry::MESH:
      return 3;
  }
  return 0;
}

void Contact::Init(Geometry* geometry, Particles* particles, size_t index)
{
  _id = index;
  switch(geometry->GetType()) {
    case Geometry::PLANE:
    {
      Plane* plane = (Plane*)geometry;
      _normal = plane->GetNormal(0.f);
      _d = pxr::GfDot(_normal, particles->_position[index] - plane->GetOrigin(0.f)) - particles->_radius[index];
      _velocity = geometry->GetVelocity();
      _relV = particles->_velocity[index] - _velocity;
      _nrmV = pxr::GfDot(_relV, _normal);
      break;
    }
    case Geometry::SPHERE:
      break;

    case Geometry::POINT:
    case Geometry::CURVE:
    case Geometry::MESH:
    {
      size_t n = _GetElementDimensionFromGeometry(geometry);
      const pxr::GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
      _normal = ComputeNormal(positions, &_elemId, n, geometry->GetMatrix());

      _velocity = geometry->GetVelocity();
      _relV = particles->_velocity[index] - geometry->GetVelocity();
      _nrmV = pxr::GfDot(_relV, _normal);
      break;
    }
  }
}

void Contact::Update(Geometry* geometry, Particles* particles, size_t index, float t)
{
  switch(geometry->GetType()) {
    case Geometry::PLANE:
    {
      Plane* plane = (Plane*)geometry;
      _normal = plane->GetNormal(t);
      _velocity = pxr::GfSlerp(t, geometry->GetPreviousVelocity(), geometry->GetVelocity());
      _torque = pxr::GfSlerp(t, geometry->GetPreviousTorque(), geometry->GetTorque());
      _d = pxr::GfDot(_normal, particles->_position[index] - plane->GetOrigin(t)) - particles->_radius[index];
      break;
    }
    case Geometry::SPHERE:
        break;

    case Geometry::POINT:
    case Geometry::CURVE:
    case Geometry::MESH:
    {
      size_t n = _GetElementDimensionFromGeometry(geometry);
      const pxr::GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
      const pxr::GfVec3f position = ComputePosition(positions, &_elemId, n, geometry->GetMatrix());

      _normal = ComputeNormal(positions, &_elemId, n, geometry->GetMatrix());

      const pxr::GfVec3f delta = position - particles->_predicted[index];

      _d = pxr::GfDot(delta, _normal);
      break;
    }
    }
}

JVR_NAMESPACE_CLOSE_SCOPE