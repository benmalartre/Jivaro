#ifndef JVR_PBD_SOLVER_H
#define JVR_PBD_SOLVER_H

#include <atomic>
#include <thread>
#include <pxr/base/gf/matrix4f.h>
#include "../common.h"
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
  size_t GetNumThreads() { return _numThreads; };
  void SetNumThreads(size_t n) { _numThreads = n; };
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
  PBDConstraint* GetConstraint(size_t idx) { return _constraints[idx]; };

private:
  size_t                              _numThreads;
  std::vector<std::thread>            _threads;
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
