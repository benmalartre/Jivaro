#include "../geometry/geometry.h"
#include "../geometry/implicit.h"
#include "../geometry/deformable.h"
#include "../pbd/contact.h"
#include "../pbd/particle.h"
#include "../pbd/collision.h"


JVR_NAMESPACE_OPEN_SCOPE


void Contact::Init(Collision* collision, Particles* particles, size_t index, size_t geomId)
{
  _geomId = geomId;

  _normal = collision->GetGradient(particles, index);
  _d = collision->GetValue(particles, index);
  _velocity = collision->GetVelocity(particles, index);
  _speed = pxr::GfDot(particles->_velocity[index] - _velocity, _normal);
}

void Contact::Update(Collision* collision, Particles* particles, size_t index)
{
  _normal = collision->GetGradient(particles, index);
  _d = collision->GetValue(particles, index);
  _velocity = collision->GetVelocity(particles, index);
  
}

JVR_NAMESPACE_CLOSE_SCOPE