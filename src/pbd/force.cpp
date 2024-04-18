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
  pxr::GfVec3f* velocity = &particles->_velocity[0];
  const float* mass = &particles->_mass[0];
  const float* invMass = &particles->_invMass[0];
  pxr::GfVec3f force;
  for (size_t index = begin; index < end; ++index) {
    if (!Affects(index) || particles->_state[index] != Particles::ACTIVE)continue;
    if(pxr::GfIsClose(mass[index], 0.f, 0.0000001f))continue;

    force = _gravity * mass[index];
    if (HasWeights())
      velocity[index] += force * _weights[index] *invMass[index] * dt;
    else
      velocity[index] += force *invMass[index] * dt;
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
  pxr::GfVec3f* velocity = &particles->_velocity[0];
  const float* mass = &particles->_mass[0];
  const float* invMass = &particles->_invMass[0];
  pxr::GfVec3f force;
  for (size_t index = begin; index < end; ++index) {
    if (!Affects(index))continue;
    force = _damp * velocity[index] * mass[index];
    if (HasWeights())
      velocity[index] -= force * _weights[index] *invMass[index] * dt;
    else
      velocity[index] -= force * invMass[index] * dt;
  }

}

JVR_NAMESPACE_CLOSE_SCOPE