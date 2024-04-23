#include "../geometry/geometry.h"
#include "../geometry/implicit.h"
#include "../geometry/deformable.h"
#include "../pbd/contact.h"
#include "../pbd/particle.h"
#include "../pbd/collision.h"


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

void Contact::Init(Collision* collision, Particles* particles, size_t index, size_t geomId)
{
  _geomId = geomId;

  _normal = collision->GetGradient(particles, index);
  _d = collision->GetValue(particles, index);
  _relV = particles->_velocity[index] - collision->GetVelocity(particles, index);
  _nrmV = pxr::GfDot(_relV, _normal);

  /*
  switch(geometry->GetType()) {
    case Geometry::PLANE:
    {
      Plane* plane = (Plane*)geometry;
      _normal = plane->GetNormal();
      _d = pxr::GfDot(_normal, particles->_position[index] - plane->GetOrigin()) - particles->_radius[index];
      _relV = particles->_velocity[index] - geometry->GetVelocity();
      _nrmV = pxr::GfDot(_relV, _normal);
      break;
    }
    case Geometry::SPHERE:
     {
      Sphere* sphere = (Sphere*)geometry;
      _normal = (particles->_position[index] - sphere->GetCenter());
      const float nL = _normal.GetLength();
      if(nL > 0.000001f)_normal.Normalize();
      else _normal = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1).GetNormalized();

      _d = pxr::GfDot(_normal, particles->_position[index] - plane->GetOrigin()) - particles->_radius[index];
      _relV = particles->_velocity[index] - geometry->GetVelocity();
      _nrmV = pxr::GfDot(_relV, _normal);
      break;
    }

    case Geometry::POINT:
    case Geometry::CURVE:
    case Geometry::MESH:
    {
      size_t n = _GetElementDimensionFromGeometry(geometry);
      const pxr::GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
      _normal = ComputeNormal(positions, &_elemId, n, geometry->GetMatrix());

      _relV = particles->_velocity[index] - geometry->GetVelocity();
      _nrmV = pxr::GfDot(_relV, _normal);
      break;
    }
  }
  */
}

void Contact::Update(Collision* collision, Particles* particles, size_t index)
{
  _normal = collision->GetGradient(particles, index);
  _d = collision->GetValue(particles, index);

  /*
  switch(geometry->GetType()) {
    case Geometry::PLANE:
    {
      Plane* plane = (Plane*)geometry;
      _normal = plane->GetNormal();
      _d = pxr::GfDot(_normal, particles->_position[index] - plane->GetOrigin()) - particles->_radius[index];
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
  */
}

JVR_NAMESPACE_CLOSE_SCOPE