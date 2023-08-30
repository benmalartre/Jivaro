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
  pxr::GfVec3f* velocity = &particles->velocity[0];
  const float* mass = &particles->mass[0];
  const float* weight = &particles->weight[0];
  for (size_t index = begin; index < end; ++index) {
    if (!Affects(index))continue;
    if (HasWeights())
      velocity[index] += _gravity * mass[index] * _weights[index] * weight[index] * dt;
    else
      velocity[index] += _gravity * mass[index] * weight[index] * dt;
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
  pxr::GfVec3f* velocity = &particles->velocity[0];
  const float* mass = &particles->mass[0];
  const float* weight = &particles->weight[0];
  for (size_t index = begin; index < end; ++index) {
    if (!Affects(index))continue;
    if (HasWeights())
      velocity[index] -= velocity[index] * _damp * _weights[index] * weight[index] * dt;
    else
      velocity[index] -= velocity[index] * _damp * weight[index] * dt;
  }

}

JVR_NAMESPACE_CLOSE_SCOPE