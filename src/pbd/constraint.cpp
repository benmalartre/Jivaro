#include "../pbd/constraint.h"
#include "../pbd/body.h"
#include "../pbd/solver.h"


JVR_NAMESPACE_OPEN_SCOPE

namespace PBD {
  int DistanceConstraint::TYPE_ID = 2;

  bool DistanceConstraint::Init(Solver* solver, 
    const unsigned int p1, const unsigned int p2, const float stiffness)
  {
    _stiffness = stiffness;
    _bodies[0] = p1;
    _bodies[1] = p2;
    Body* body = solver->GetBody();
    const pxr::GfVec3f* restPositions = body->GetRestPositions();

    const pxr::GfVec3f& x1_0 = restPositions[p1];
    const pxr::GfVec3f& x2_0 = restPositions[p2];

    _restLength = (x2_0 - x1_0).GetLength();

    return true;
  }

  bool DistanceConstraint::Solve(Solver* solver, const unsigned int iter)
  { 
    Body* body = solver->GetBody();
    const unsigned p1 = _bodies[0];
    const unsigned p2 = _bodies[1];

    pxr::GfVec3f* positions = body->GetPositions();

    pxr::GfVec3f& x1 = positions[p1];
    pxr::GfVec3f& x2 = positions[p2];

    const float* masses = body->GetMassesCPtr();
    const float invMass1 = 1.f / masses[p1];
    const float invMass2 = 1.f / masses[p2];

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

  int RestoreConstraint::TYPE_ID = 3;

  bool RestoreConstraint::Init(Solver* solver,
    const unsigned int p1, const float stiffness)
  {
    _stiffness = stiffness;
    _bodies[0] = p1;
    return true;
  }

  bool RestoreConstraint::Solve(Solver* solver, const unsigned int iter)
  {
    Body* body = solver->GetBody();
    const unsigned p = _bodies[0];

    pxr::GfVec3f* positions = body->GetPositions();
    const pxr::GfVec3f* inputs = body->GetInputPositions();

    pxr::GfVec3f& x1 = positions[p];
    const pxr::GfVec3f& x2 = inputs[p];

    const float* masses = body->GetMassesCPtr();
    const float invMass = 1.f / masses[p];

    if (invMass == 0.0)
      return false;

    pxr::GfVec3f n = x2 - x1;
    float d = n.GetLength();
    n.Normalize();

    pxr::GfVec3f corr;
    corr = _stiffness * n * ((double)d) / invMass;

    pxr::GfVec3f corr1 = invMass * corr;
    pxr::GfVec3f corr2 = -invMass * corr;

    if (invMass != 0.0)
      x1 += corr1;

    return true;
  }
}

JVR_NAMESPACE_CLOSE_SCOPE