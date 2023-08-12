#include "../geometry/geometry.h"
#include "../pbd/constraint.h"
#include "../pbd/body.h"
#include "../pbd/solver.h"


JVR_NAMESPACE_OPEN_SCOPE

namespace PBD {
  int DistanceConstraint::TYPE_ID = 2;

  bool DistanceConstraint::Init(Solver* solver, 
    const size_t p1, const size_t p2, const float stretchStiffness, const float compressionStiffness)
  {
    _stretchStiffness = stretchStiffness;
    _compressionStiffness = compressionStiffness;
    _bodies[0] = p1;
    _bodies[1] = p2;
    Body* body = solver->GetBody();
    const pxr::GfVec3f* positions = body->geometry->GetPositionsCPtr();s

    const pxr::GfVec3f& x1_0 = positions[p1];
    const pxr::GfVec3f& x2_0 = positions[p2];

    _restLength = (x2_0 - x1_0).GetLength();

    return true;
  }

  bool DistanceConstraint::Solve(Solver* solver, const size_t iter)
  { 
    Body* body = solver->GetBody();
    const unsigned p1 = _bodies[0];
    const unsigned p2 = _bodies[1];

    pxr::GfVec3f* positions = body->geometry->GetPositionsPtr();

    pxr::GfVec3f& x1 = positions[p1];
    pxr::GfVec3f& x2 = positions[p2];

    const float mass = body->mass;
    const float invMass1 = 1.f / mass;
    const float invMass2 = 1.f / mass;

    float weightSum = invMass1 + invMass2;
    if (weightSum == 0.0)
      return false;

    pxr::GfVec3f n = x2 - x1;
    float d = n.GetLength();
    n.Normalize();

    pxr::GfVec3f corr;
    corr = _stiffness * n * ((double)d - _restLength) / weightSum;

    pxr::GfVec3f corr1 = invMass1 * corr;
    pxr::GfVec3f corr2 = -invMass2 * corr;

    if (invMass1 != 0.0)
      x1 += corr1;
    if (invMass2 != 0.0)
      x2 += corr2;

    return true;
  }
}

JVR_NAMESPACE_CLOSE_SCOPE