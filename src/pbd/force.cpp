#include "../pbd/force.h"
#include "../pbd/particle.h"
#include "../pbd/solver.h"

JVR_NAMESPACE_OPEN_SCOPE

GravitationalForce::GravitationalForce() 
  : Force()
  , _gravity(0.f, -9.8f, 0.f)
{
}

GravitationalForce::GravitationalForce(const pxr::GfVec3f& gravity) 
  : Force()
  , _gravity(gravity)
{
}

void GravitationalForce::Apply(size_t begin, size_t end, Particles* particles, float dt) const
{
  const float* mass = &particles->_mass[0];
  const float* invMass = &particles->_invMass[0];
  pxr::GfVec3f* velocity = &particles->_velocity[0];

  Mask::Iterator iterator((const Mask*)this, begin, end);

  if(HasWeights())
    for(size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next()) {
      velocity[index] += _gravity * mass[index] * _weights[index] * invMass[index] * dt;
    }
  else
    for(size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next()) {
      velocity[index] += _gravity * mass[index] * invMass[index] * dt;
    }
}

DampingForce::DampingForce()
  : Force()
  , _damp(0.5f)
{
}

DampingForce::DampingForce(float damp)
  : Force(),
   _damp(damp)
{
}

void DampingForce::Apply(size_t begin, size_t end, Particles* particles, float dt) const
{
  const float* mass = &particles->_mass[0];
  const float* invMass = &particles->_invMass[0];
  pxr::GfVec3f* velocity = &particles->_velocity[0];

  if(HasWeights())
    for(size_t index = begin; index < end; ++index) {
      if (particles->_state[index] != Particles::ACTIVE)continue;
      velocity[index] -= _damp * velocity[index] * mass[index] * _weights[index] *invMass[index] * dt;
    }
  else 
    for(size_t index = begin; index < end; ++index) {
      if (particles->_state[index] != Particles::ACTIVE)continue;
      velocity[index] -= _damp * velocity[index] * mass[index] * invMass[index] * dt;
    }
}

JVR_NAMESPACE_CLOSE_SCOPE