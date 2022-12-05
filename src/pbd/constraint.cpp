#include "../pbd/constraint.h"
#include "../pbd/particle.h"
#include "../pbd/solver.h"


JVR_NAMESPACE_OPEN_SCOPE

int PBDDistanceConstraint::TYPE_ID = 2;

bool PBDDistanceConstraint::Init(PBDSolver* solver, 
  const unsigned int p1, const unsigned int p2, const float stiffness)
{
  _stiffness = stiffness;
  _bodies[0] = p1;
  _bodies[1] = p2;
  PBDParticle& system = solver->GetSystem();
  const pxr::VtArray<pxr::GfVec3f>& initPositions = system.GetInitPositions();

  const pxr::GfVec3f& x1_0 = initPositions[p1];
  const pxr::GfVec3f& x2_0 = initPositions[p2];

  _restLength = (x2_0 - x1_0).GetLength();

  return true;
}

bool PBDDistanceConstraint::Solve(PBDSolver* solver, const unsigned int iter)
{
  PBDParticle& system = solver->GetSystem();
  const unsigned p1 = _bodies[0];
  const unsigned p2 = _bodies[1];

  pxr::VtArray<pxr::GfVec3f>& positions = system.GetPositions();

  pxr::GfVec3f& x1 = system.GetPosition(p1);
  pxr::GfVec3f& x2 = system.GetPosition(p2);

  const pxr::VtArray<float>& masses = system.GetMasses();
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

int PBDRestoreConstraint::TYPE_ID = 3;

bool PBDRestoreConstraint::Init(PBDSolver* solver,
  const unsigned int p1, const float stiffness)
{
  _stiffness = stiffness;
  _bodies[0] = p1;
  return true;
}

bool PBDRestoreConstraint::Solve(PBDSolver* solver, const unsigned int iter)
{
  PBDParticle& system = solver->GetSystem();
  const unsigned p = _bodies[0];

  pxr::VtArray<pxr::GfVec3f>& positions = system.GetPositions();
  pxr::VtArray<pxr::GfVec3f>& inputs = system.GetInputPositions();

  pxr::GfVec3f& x1 = positions[p];
  pxr::GfVec3f& x2 = inputs[p];

  const pxr::VtArray<float>& masses = system.GetMasses();
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
  if (invMass != 0.0)
    x2 += corr2;

  return true;
}

JVR_NAMESPACE_CLOSE_SCOPE