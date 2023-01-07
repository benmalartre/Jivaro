#ifndef JVR_PBD_SOLVER_H
#define JVR_PBD_SOLVER_H

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

    void AddGeometry(Geometry* geom, const pxr::GfMatrix4f& m);
    void RemoveGeometry(Geometry* geom);
    void AddColliders(std::vector<Geometry*>& colliders);
    void AddConstraints(Geometry* geom, size_t offset);
    void SatisfyConstraints();
    void UpdateColliders();
    void Reset();
    void Step();
    PBDParticle& GetSystem() { return _system; };

private:
    PBDParticle                 _system;
    pxr::GfVec3f                _gravity;
    float                       _timeStep;
    size_t                      _substeps;
    bool                        _paused;		
    std::vector<PBDConstraint*> _constraints;
    std::vector<Geometry*>      _colliders;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_SOLVER_H
