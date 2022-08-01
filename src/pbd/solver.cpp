#include "../pbd/solver.h"
#include "../pbd/constraint.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"


JVR_NAMESPACE_OPEN_SCOPE
PBDSolver::PBDSolver() 
  : _gravity(0,-1,0)
  , _timeStep(0.05)
{
}


PBDSolver::~PBDSolver()
{

}

void PBDSolver::Reset()
{
  _system.Reset();
}

void PBDSolver::AddGeometry(Geometry* geom, const pxr::GfMatrix4f& m)
{
  _system.AddGeometry(geom, m);
  AddConstraints(geom);
}

void PBDSolver::RemoveGeometry(Geometry* geom)
{
  _system.RemoveGeometry(geom);
}

void PBDSolver::AddConstraints(Geometry* geom)
{
  if (geom->GetType() == Geometry::MESH) {
    Mesh* mesh = (Mesh*)geom;
    std::vector<HalfEdge*> edges = mesh->GetUniqueEdges();
    _constraints.reserve(_constraints.size() + edges.size());
    for (HalfEdge* edge : edges) {
      PBDDistanceConstraint* constraint = new PBDDistanceConstraint();
      constraint->Init(this, edge->vertex, edge->next->vertex, 0.5f);
      _constraints.push_back(constraint);
    }
  }
}

void PBDSolver::SatisfyConstraints()
{
  std::cout << "NUM CONSTRAINTS : " << _constraints.size() << std::endl;
  for (int j = 0; j < 5; j++) {
    for (int i = 0; i < _constraints.size(); i++) {
      PBDConstraint* c = _constraints[i];
      c->Solve(this, 1);
    }
    // Constrain one particle of the cloth to origo
    _system.GetPosition(0) = _system.GetInitPositions()[0];

    for (int i = 0; i < _system.GetNumParticles(); ++i) {
      pxr::GfVec3f& p = _system.GetPosition(i);


      //_position[i][0] = pxr::GfMin(pxr::GfMax(_position[i][0], -100.f), 100.f); 
      p[1] = pxr::GfMax(p[1], 0.f);
      //_position[i][2] = pxr::GfMin(pxr::GfMax(_position[i][2], 100.f), 100.f);
    }
  }
}

void PBDSolver::Step()
{
  _system.AccumulateForces(_gravity);
  _system.Integrate(_timeStep);
  SatisfyConstraints();

  _system.UpdateGeometries();
}

JVR_NAMESPACE_CLOSE_SCOPE