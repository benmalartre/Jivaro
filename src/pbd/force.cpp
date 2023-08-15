#include "../pbd/force.h"
#include "../pbd/particle.h"
#include "../pbd/solver.h"

JVR_NAMESPACE_OPEN_SCOPE

void Force::AddBody(Particles* particles, Body* body)
{

}

void Force::RemoveBody(Particles* particles, Body* body)
{
  if (!HasMask()) return;

  const size_t offset = body->offset;
  const size_t remove = body->numPoints;

  if (_mask.back() < body->offset) return;

  size_t removeStart = Solver::INVALID_INDEX;
  size_t removeEnd = Solver::INVALID_INDEX;

  for (size_t p = 0; p < _mask.size(); ++p) {
    if (_mask[p] < offset) continue;
    if (_mask[p] < offset + remove) {
      if (removeStart == Solver::INVALID_INDEX) {
        removeStart = p;
        removeEnd = p;
      } else {
        removeEnd = p;
      }
    } else {
      _mask[p] -= remove;
    }

    if (removeStart != Solver::INVALID_INDEX) {
      _mask.erase(_mask.begin() + removeStart, _mask.begin() + removeEnd);
    }
  }
}

bool Force::Affects(size_t index) const {
  if (!HasMask())return true;
  const size_t bitsIdx = index / sizeof(int);
  if (bitsIdx >= _mask.size())return false;
  return BITMASK_CHECK(_mask[bitsIdx], index % sizeof(int));
}

GravitationalForce::GravitationalForce() 
  : _gravity(0.f, -9.18f, 0.f)
{
}

GravitationalForce::GravitationalForce(const pxr::GfVec3f& gravity) 
  : _gravity(gravity)
{
}

void GravitationalForce::Apply(pxr::GfVec3f* velocity, const float dt, size_t index) const
{
  if (HasWeights())
    velocity[index] += dt * _gravity * _weights[index];
  else
    velocity[index] += dt * _gravity;
}

DampingForce::DampingForce()
  : _damp(0.f)
{
}

DampingForce::DampingForce(float damp)
  : _damp(damp)
{
}

void DampingForce::Apply(pxr::GfVec3f* velocity, const float dt, size_t index) const
{
  if (HasWeights())
    velocity[index] -= velocity[index] * _damp * _weights[index] * dt;
  else
    velocity[index] -= velocity[index] * _damp * dt;
}

JVR_NAMESPACE_CLOSE_SCOPE