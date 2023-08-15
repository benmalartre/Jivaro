#include "../geometry/geometry.h"
#include "../pbd/constraint.h"
#include "../pbd/particle.h"
#include "../pbd/solver.h"


JVR_NAMESPACE_OPEN_SCOPE

size_t DistanceConstraint::TYPE_ID = Constraint::DISTANCE;

bool DistanceConstraint::Init(Particles* particles, 
  const size_t p1, const size_t p2, const float stretchStiffness, const float compressionStiffness)
{
  _stretchStiffness = stretchStiffness;
  _compressionStiffness = compressionStiffness;
  _particles[0] = p1;
  _particles[1] = p2;

  const pxr::GfVec3f* positions = &particles->position[0];
  _restLength = (positions[p2] - positions[p1]).GetLength();

  return true;
}

bool DistanceConstraint::Solve(Particles* particles, const size_t iter)
{ 
  const size_t p1 = _particles[0];
  const size_t p2 = _particles[1];

  pxr::GfVec3f* positions = &particles->position[0];
  const float* masses = &particles->mass[0];

  pxr::GfVec3f& x1 = positions[p1];
  pxr::GfVec3f& x2 = positions[p2];

  const float invMass1 = pxr::GfIsClose(masses[p1], 0, 0.0000001) ? 0.f : 1.f / masses[p1];
  const float invMass2 = pxr::GfIsClose(masses[p2], 0, 0.0000001) ? 0.f : 1.f / masses[p2];

  float weightSum = invMass1 + invMass2;
  if (pxr::GfIsClose(weightSum, 0.0, 0.0000001))
    return false;

  pxr::GfVec3f n = x2 - x1;
  float d = n.GetLength();
  n.Normalize();

  pxr::GfVec3f corr;
  corr = _stretchStiffness * n * ((double)d - _restLength) / weightSum;

  _corrections[0] = invMass1 * corr;
  _corrections[1] = -invMass2 * corr;

  return true;
}

void DistanceConstraint::Apply(Particles* particles, const size_t index)
{
  pxr::GfVec3f* positions = &particles->position[0];
  size_t p = _GetParticleIndex(index);
  if(p != Constraint::INVALID_INDEX)positions[index] += _corrections[p];
}

JVR_NAMESPACE_CLOSE_SCOPE