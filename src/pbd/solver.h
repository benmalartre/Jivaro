#ifndef JVR_PBD_SOLVER_H
#define JVR_PBD_SOLVER_H

#include <pxr/base/gf/matrix4f.h>
#include "../common.h"
#include "../pbd/body.h"
#include "../pbd/constraint.h"

JVR_NAMESPACE_OPEN_SCOPE

namespace PBD {
  class Solver
  {
  public:
    Solver();
    ~Solver();

    float GetTimeStep() { return _timeStep; };
    void SetTimeStep(float step) { _timeStep = step; };
    const pxr::GfVec3f& GetGravity() { return _gravity; };
    void SetGravity(const pxr::GfVec3f& gravity) { _gravity = gravity; };
    void AddGeometry(Geometry* geom, const pxr::GfMatrix4f& m);
    void RemoveGeometry(Geometry* geom);
    void AddCollider(Geometry* colliders);
    void AddConstraints(Geometry* geom, size_t offset);
    void SatisfyConstraints();
    void UpdateColliders();
    void UpdateGeometries();
    void Reset();
    void Step();
    Body* GetBody() { return &_body; };
    size_t GetNumConstraints() { return _constraints.size(); };
    Constraint* GetConstraint(size_t idx) { return _constraints[idx]; };

    void SetGrain(size_t grain) { _grain = grain; };

  protected:
    void _Reset(size_t start, size_t end);
    void _Integrate(size_t start, size_t end);
    void _AccumulateForces(size_t start, size_t end);
    void _SolveCollisions(size_t start, size_t end);
    void _SatisfyConstraints(size_t start, size_t end);

  private:
    size_t                              _grain;
    Body                                _body;
    pxr::GfVec3f                        _gravity;
    float                               _timeStep;
    size_t                              _substeps;
    bool                                _paused;		
    std::vector<Constraint*>            _constraints;
    //std::vector<Collider*>              _colliders;
    std::vector<Body*>                  _bodies;

  };
}

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_SOLVER_H
