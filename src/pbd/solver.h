#ifndef JVR_PBD_SOLVER_H
#define JVR_PBD_SOLVER_H


#include "../common.h"
#include "../pbd/particle.h"


JVR_NAMESPACE_OPEN_SCOPE

class PBDSolver
{
public:
    PBDSolver();
    ~PBDSolver();


    void AddGeometry(Geometry* geom);
    void RemoveGeometry(Geometry* geom);
    void Step();
    PBDParticle& GetSystem() { return _system; };

private:
    PBDParticle     _system;
    pxr::GfVec3f    _gravity;
    float           _timeStep;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_SOLVER_H
