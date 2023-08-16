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
  _p[0] = p1;
  _p[1] = p2;

  memset(&_c[0], 0.f, 2 * sizeof(pxr::GfVec3f));

  const pxr::GfVec3f* positions = &particles->position[0];
  _restLength = (positions[p2] - positions[p1]).GetLength();

  return true;
}

bool DistanceConstraint::Solve(Particles* particles, const size_t iter)
{ 
  memset(&_c[0], 0.f, 2 * sizeof(pxr::GfVec3f));

  const pxr::GfVec3f& x1 = particles->predicted[_p[0]];
  const pxr::GfVec3f& x2 = particles->predicted[_p[1]];

  const float im1 = 
    pxr::GfIsClose(particles->mass[_p[0]], 0, 0.0000001) ? 
    0.f : 
    1.f / particles->mass[_p[0]];

  const float im2 =
    pxr::GfIsClose(particles->mass[_p[1]], 0, 0.0000001) ? 
    0.f : 
    1.f / particles->mass[_p[1]];

  float sum = im1 + im2;
  if (pxr::GfIsClose(sum, 0.f, 0.0000001f))
    return false;

  pxr::GfVec3f n = x2 - x1;
  float d = n.GetLength();
  if (pxr::GfIsClose(d, _restLength, 0.0000001f))return false;
  if (!n.Normalize()) {
    std::cout << "fail normlaize :(" << x1 << ", " << x2 << std::endl;
  }

  pxr::GfVec3f c;
  if(d < _restLength)
    c = _compressionStiffness * n * (d - _restLength) * sum;
  else 
    c = _stretchStiffness * n * (d - _restLength) * sum;

  _c[0] = im1 * c;
  _c[1] = -im2 * c;


  return true;
}

// this one has to happen serialy
void DistanceConstraint::Apply(Particles* particles, const float dt)
{
  particles->predicted[_p[0]] += _c[0] * dt;
  particles->predicted[_p[1]] += _c[1] * dt;
}

JVR_NAMESPACE_CLOSE_SCOPE