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
    void AddConstraints(Geometry* geom, size_t offset);
    void SatisfyConstraints();
    void Reset();
    void Step();
    PBDParticle& GetSystem() { return _system; };

private:
    PBDParticle                 _system;
    pxr::GfVec3f                _gravity;
    float                       _timeStep;
    std::vector<PBDConstraint*> _constraints;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_SOLVER_H
