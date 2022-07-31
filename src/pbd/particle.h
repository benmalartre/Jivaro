#ifndef JVR_PBD_PARTICLE_H
#define JVR_PBD_PARTICLE_H



#include <map>
#include <float.h>
#include <pxr/base/vt/array.h>


#include "../common.h"
#include "../geometry/geometry.h"


JVR_NAMESPACE_OPEN_SCOPE

class PBDParticle
{
public:
    PBDParticle();
    ~PBDParticle();

    void AddGeometry(Geometry* geom);
    void RemoveGeometry(Geometry* geom);

    void Integrate(float step);
    void SatisfyConstraints();
    void AccumulateForces(const pxr::GfVec3f& gravity);
    void UpdateGeometries();
    void Reset();
    size_t GetNumParticles() { return _N; };
    pxr::VtArray<pxr::GfVec3f>& GetPositions() { return _position; };
    const pxr::VtArray<pxr::GfVec3f>& GetPositions() const { return _position; };
    pxr::VtArray<pxr::GfVec3f>& GetPrevious() { return _previous; };
    const pxr::VtArray<pxr::GfVec3f>& GetPrevious() const { return _previous; };
    pxr::VtArray<pxr::GfVec3f>& GetForces() { return _force; };
    const pxr::VtArray<pxr::GfVec3f>& GetForces() const { return _force; };
    pxr::VtArray<float>& GetMasses() { return _mass; };
    const pxr::VtArray<float>& GetMasses() const { return _mass; };

private:
    size_t                       _N;
    pxr::VtArray<pxr::GfVec3f>   _initial;
    pxr::VtArray<pxr::GfVec3f>   _preload;
    pxr::VtArray<pxr::GfVec3f>   _position;
    pxr::VtArray<pxr::GfVec3f>   _previous;
    pxr::VtArray<pxr::GfVec3f>   _force;
    pxr::VtArray<float>          _mass;

    std::map<Geometry*, size_t>  _geometries;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
