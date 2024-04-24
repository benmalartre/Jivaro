#include "../pbd/force.h"
#include "../pbd/particle.h"
#include "../pbd/solver.h"

JVR_NAMESPACE_OPEN_SCOPE

GravityForce::GravityForce(const pxr::GfVec3f& gravity) 
  : Force()
  , _gravity(gravity)
  , _attr()
{
}

GravityForce::GravityForce(const pxr::UsdAttribute& attribute) 
  : Force()
  , _attr(attribute)
{
  _attr.Get(&_gravity, pxr::UsdTimeCode::Default());
}

void GravityForce::Update(float time)
{
  if(_attr.IsValid()) _attr.Get(&_gravity, time);
}

void GravityForce::Apply(size_t begin, size_t end, Particles* particles, float dt) const
{
  const float* mass = &particles->_mass[0];
  const float* invMass = &particles->_invMass[0];
  pxr::GfVec3f* velocity = &particles->_velocity[0];


  Mask::Iterator iterator((const Mask*)this, begin, end);

  if(HasWeights())
    for(size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next()) {
      velocity[index] += _gravity * _weights[index] * invMass[index] * dt;
    }
  else
    for(size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next()) {
      velocity[index] += _gravity * invMass[index] * dt;
    }
}

DampForce::DampForce(const pxr::UsdAttribute& attr)
  : Force()
  , _attr(attr)
{
  _attr.Get(&_damp, pxr::UsdTimeCode::Default());
}

DampForce::DampForce(float damp)
  : Force()
  , _damp(damp)
  , _attr()
{
}

void DampForce::Update(float time)
{
  if(_attr.IsValid()) _attr.Get(&_damp, time);
}

void DampForce::Apply(size_t begin, size_t end, Particles* particles, float dt) const
{
  const float* mass = &particles->_mass[0];
  const float* invMass = &particles->_invMass[0];
  pxr::GfVec3f* velocity = &particles->_velocity[0];

  if(HasWeights())
    for(size_t index = begin; index < end; ++index) {
      if (particles->_state[index] != Particles::ACTIVE)continue;
      velocity[index] -= _damp * velocity[index]  * _weights[index] *invMass[index] * dt;
    }
  else 
    for(size_t index = begin; index < end; ++index) {
      if (particles->_state[index] != Particles::ACTIVE)continue;
      velocity[index] -= _damp * velocity[index] * invMass[index] * dt;
    }
}

JVR_NAMESPACE_CLOSE_SCOPE