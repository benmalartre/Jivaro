#include "../pbd/solver.h"
#include "../pbd/constraint.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../acceleration/bvh.h"
#include "../app/application.h"
#include "../app/time.h"


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
  size_t offset = _system.AddGeometry(geom, m);
  AddConstraints(geom, offset);

  if (geom->GetType() == Geometry::MESH) {
    std::cout << "BVH INIT" << std::endl;
    BVH bvh;
    bvh.Init({ (Mesh*)geom });
    std::cout << "BVH NUM CELLS " << 666 << std::endl;
  }
}

void PBDSolver::RemoveGeometry(Geometry* geom)
{
  _system.RemoveGeometry(geom);
}

void PBDSolver::AddConstraints(Geometry* geom, size_t offset)
{
  std::cout << "ADD CONSTRAINT : " << geom->GetType() << std::endl;
  if (geom->GetType() == Geometry::MESH) {
    Mesh* mesh = (Mesh*)geom;
    std::vector<HalfEdge*> edges = mesh->GetUniqueEdges();
    for (HalfEdge* edge : edges) {
      PBDDistanceConstraint* constraint = new PBDDistanceConstraint();
      constraint->Init(this, edge->vertex + offset, edge->next->vertex + offset, 0.5f);
      _constraints.push_back(constraint);
    }
  }
}

void PBDSolver::SatisfyConstraints()
{
  Time& time = GetApplication()->GetTime();
  for (int j = 0; j < 5; j++) {
    for (int i = 0; i < _constraints.size(); i++) {
      PBDConstraint* c = _constraints[i];
      c->Solve(this, 1);
    }
    // Constrain one particle of the cloth to origo
    _system.GetPosition(0) =
      _system.GetInitPositions()[0] + 
        pxr::GfVec3f(0, sin(time.GetActiveTime()) * 5.f + 1.f, 0.f);

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