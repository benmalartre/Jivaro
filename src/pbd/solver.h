#ifndef JVR_PBD_SOLVER_H
#define JVR_PBD_SOLVER_H

#include <pxr/base/gf/matrix4f.h>
#include "../common.h"
#include "../acceleration/pool.h"
#include "../pbd/particle.h"
#include "../pbd/constraint.h"

JVR_NAMESPACE_OPEN_SCOPE

class PBDSolver
{
public:
  PBDSolver();
  ~PBDSolver();

  float GetTimeStep() { return _timeStep; };
  void SetTimeStep(float step) { _timeStep = step; };
  const pxr::GfVec3f& GetGravity() { return _gravity; };
  void SetGravity(const pxr::GfVec3f& gravity) { _gravity = gravity; };
  void AddGeometry(Geometry* geom, const pxr::GfMatrix4f& m);
  void RemoveGeometry(Geometry* geom);
  void AddColliders(std::vector<Geometry*>& colliders);
  void AddConstraints(Geometry* geom, size_t offset);
  void SatisfyConstraints();
  void UpdateColliders();
  void UpdateGeometries();
  void Reset();
  void Step();
  PBDParticle* GetSystem() { return &_system; };
  size_t GetNumConstraints() { return _constraints.size(); };
  PBDConstraint* GetConstraint(size_t idx) { return _constraints[idx]; };

  void SetNumTasks(size_t numTasks) { _numTasks = numTasks; };
  void _ParallelEvaluation(ThreadPool::TaskFn fn, size_t numElements, size_t numTasks);

private:
  size_t                              _numTasks;
  ThreadPool                          _pool;
  PBDParticle                         _system;
  pxr::GfVec3f                        _gravity;
  float                               _timeStep;
  size_t                              _substeps;
  bool                                _paused;		
  std::vector<PBDConstraint*>         _constraints;
  std::vector<Geometry*>              _colliders;
  std::map<Geometry*, PBDGeometry>    _geometries;

};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_SOLVER_H
