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

namespace PBD {
  class Solver
  {
  public:
    const static size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

    Solver();
    ~Solver();

    // setup
    void AddBody(Geometry* geom, const pxr::GfMatrix4f& m);
    void RemoveBody(Geometry* geom);
    //void AddCollision(Geometry* geom);
    //void RemoveCollision(Geometry* geom);
    void AddConstraints(Geometry* geom, size_t offset);

    // attributes
    float GetTimeStep() { return _timeStep; };
    void SetTimeStep(float step) { _timeStep = step; };
    const pxr::GfVec3f& GetGravity() { return _gravity; };
    void SetGravity(const pxr::GfVec3f& gravity) { _gravity = gravity; };
    size_t GetNumParticles() { return _particles.GetNumParticles(); };
    size_t GetNumConstraints() { return _constraints.size(); };
    Body* GetBody(size_t index);
    Body* GetBody(Geometry* geom);
    size_t GetBodyIndex(Geometry* geom);
    Constraint* GetConstraint(size_t idx) { return _constraints[idx]; };
    Particles* GetParticles(){ return &_particles; };
   
    // solver
    void SatisfyConstraints();
    void UpdateCollisions();
    void UpdateGeometries();
    void Reset();
    void Step();

  private:
    size_t                              _grain;
    size_t                              _substeps;
    float                               _timeStep;
    bool                                _paused;		

    // system
    Particles                           _particles;
    std::vector<Constraint*>            _constraints;
    std::vector</*Static*/Constraint>   _staticConstraints;
    //std::vector<Collision*>              _collisions;
    std::vector<Body*>                  _bodies;
    std::vector<Force*>                 _force;
    pxr::GfVec3f                        _gravity;

  };
}

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_SOLVER_H
