#ifndef JVR_PBD_SOLVER_H
#define JVR_PBD_SOLVER_H

#include <limits>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/vt/array.h>

#include "../common.h"
#include "../pbd/particle.h"
#include "../pbd/constraint.h"
#include "../pbd/force.h"

JVR_NAMESPACE_OPEN_SCOPE

class Solver
{
public:
  const static size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

  Solver();
  ~Solver();
  
  //void AddCollision(Geometry* geom);
  //void RemoveCollision(Geometry* geom);
  void AddConstraints(Geometry* geom, size_t offset);

  // attributes
  size_t GetSolverIterations() { return _solverIterations; };
  void SetSolverIterations(size_t iterations) { _solverIterations = iterations; };
  size_t GetCollisionIterations() { return _collisionIterations; };
  void SetCollisionIterations(size_t iterations) { _collisionIterations = iterations; };
  float GetSleepThreshold() { return _sleepThreshold; };
  void SetSleepThreshold(float threshold) { _sleepThreshold = threshold; };
  const pxr::GfVec3f& GetGravity() { return _gravity; };
  void SetGravity(const pxr::GfVec3f& gravity) { _gravity = gravity; };

  // system
  size_t GetNumParticles() { return _particles.GetNumParticles(); };
  size_t GetNumConstraints() { return _constraints.size(); };
  size_t GetNumForces() { return _force.size(); };
  //size_t GetNumCollisions() { return _collisions.size(); };

  // bodies
  void AddBody(Geometry* geom, const pxr::GfMatrix4f& m);
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

  // collisions
  /*
  void AddCollision(Collision* collision) { _collisions.push_back(collision); };
  Collision* GetCollision(size_t idx = 0) { return _collisions[idx]; };
  */
  // particles
  Particles* GetParticles() { return &_particles; };
  
  // solver
  void UpdateCollisions();
  void UpdateGeometries();
  void Reset();
  void Step(float dt);

private:
  void _ApplyForce(size_t begin, size_t end, const Force* force, const float dt);
  void _ApplyForceMasked(size_t begin, size_t end, const Force* force, const float dt);
  size_t                              _solverIterations;
  size_t                              _collisionIterations;
  float                               _sleepThreshold;
  bool                                _paused;		

  // system
  Particles                           _particles;
  std::vector<Constraint*>            _constraints;
  //std::vector<Collision*>           _collisions;
  std::vector<Body*>                  _bodies;
  std::vector<Force*>                 _force;
  pxr::GfVec3f                        _gravity;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_SOLVER_H
