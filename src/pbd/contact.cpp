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
}

void Contact::Update(Collision* collision, Particles* particles, size_t index)
{
  _normal = collision->GetGradient(particles, index);
  _d = collision->GetValue(particles, index);
}

JVR_NAMESPACE_CLOSE_SCOPE