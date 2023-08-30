#ifndef JVR_PBD_SOLVER_H
#define JVR_PBD_SOLVER_H

#include <limits>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/vt/array.h>

#include "../common.h"
#include "../pbd/particle.h"

JVR_NAMESPACE_OPEN_SCOPE

struct Particles;
class Constraint;
class Force;
class Collision;
class Solver
{
public:
  const static size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

  Solver();
  ~Solver();
  
  //void AddCollision(Geometry* geom);
  //void RemoveCollision(Geometry* geom);
  void AddConstraints(Body* body);

  // attributes
  size_t GetSolverIterations() { return _solverIterations; };
  void SetSolverIterations(size_t iterations) { _solverIterations = iterations; };
  float GetSleepThreshold() { return _sleepThreshold; };
  void SetSleepThreshold(float threshold) { _sleepThreshold = threshold; };
  const pxr::GfVec3f& GetGravity() { return _gravity; };
  void SetGravity(const pxr::GfVec3f& gravity) { _gravity = gravity; };

  // system
  size_t GetNumParticles() { return _particles.GetNumParticles(); };
  size_t GetNumConstraints() { return _constraints.size(); };
  size_t GetNumForces() { return _force.size(); };
  size_t GetNumCollisions() { return _collisions.size(); };

  // bodies
  Body* AddBody(Geometry* geom, const pxr::GfMatrix4f& m);
  void RemoveBody(Geometry* geom);
  Body* GetBody(size_t index);
  Body* GetBody(Geometry* geom);
  size_t GetBodyIndex(Geometry* geom);

  // forces
  void AddForce(Force* force) { _force.push_back(force); };
  Force* GetForce(size_t idx) { return _force[idx]; };

  // constraints
  void AddConstraint(Constraint* constraint) { _constraints.push_back(constraint); };
  Constraint* GetConstraint(size_t idx) { return _constraints[idx]; };
  void GetConstraintsByType(short type, pxr::VtArray<Constraint*>& results);

  // collisions
  void AddCollision(Collision* collision) { _collisions.push_back(collision); };
  Collision* GetCollision(size_t idx = 0) { return _collisions[idx]; };

  // particles
  Particles* GetParticles() { return &_particles; };
  void LockPoints();
  
  // solver
  void UpdateCollisions();
  void UpdateGeometries();
  void Reset();
  void Step(bool serial=false);

private:
  void _ResolveCollisions(bool serial=false);
  void _UpdateCollisions(bool serial=false);
  void _IntegrateParticles(size_t begin, size_t end);
  void _UpdateParticles(size_t begin, size_t end);
  void _StepOneSerial();
  void _StepOne();

  size_t                              _subSteps;
  size_t                              _solverIterations;
  float                               _sleepThreshold;
  float                               _stepTime;
  float                               _physicTime;
  float                               _startFrame;
  bool                                _paused;		

  // system
  Particles                           _particles;
  pxr::VtArray<Constraint*>           _constraints;
  pxr::VtArray<Collision*>            _collisions;
  pxr::VtArray<Body*>                 _bodies;
  pxr::VtArray<Force*>                _force;
  pxr::GfVec3f                        _gravity;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_SOLVER_H
