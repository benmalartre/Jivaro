#include "../pbd/solver.h"

JVR_NAMESPACE_OPEN_SCOPE
PBDSolver::PBDSolver() 
  : _gravity(0,-1,0)
  , _timeStep(0.05)
{
}


PBDSolver::~PBDSolver()
{

}

void PBDSolver::AddGeometry(Geometry* geom)
{
  _system.AddGeometry(geom);
}

void PBDSolver::RemoveGeometry(Geometry* geom)
{
  _system.RemoveGeometry(geom);
}

void PBDSolver::Step()
{
  _system.AccumulateForces(_gravity);
  _system.Integrate(_timeStep);
  _system.SatisfyConstraints();

  _system.UpdateGeometries();
}

JVR_NAMESPACE_CLOSE_SCOPE