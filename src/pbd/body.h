#ifndef JVR_PBD_PARTICLE_H
#define JVR_PBD_PARTICLE_H

#include <map>
#include <float.h>
#include <pxr/base/vt/array.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/bbox3d.h>

#include "../common.h"
#include "../geometry/geometry.h"
#include "../pbd/constraint.h"

JVR_NAMESPACE_OPEN_SCOPE

namespace PBD {
  class Body
  {
  public:
    float GetMass(){return _mass;};
    void SetMass(float mass){_mass = mass;};
    float GetDamping(){return _damping;};
    void SetDamping(float damping){_damping = damping;};
    float GetRadius(){return _radius;};
    void SetRadius(float radius){_radius = radius;};
    float GetDiameter(){return _radius * 2.f;};

    void SetGeometry(Geometry* geom, const pxr::GfMatrix4f& matrix);
    
    size_t GetNumParticles() { return _position.size(); };
    size_t GetNumConstraints() {return _constraints.size(); };

    pxr::GfVec3f& GetPosition(size_t index) { return _position[index]; };
    const pxr::GfVec3f& GetPosition(size_t index) const { return _position[index]; };
    const pxr::VtArray<pxr::GfVec3f>& GetPositions() const { return _position; };
    pxr::GfVec3f* GetPositionsPtr() { return &_position[0]; };
    const pxr::GfVec3f* GetPositionsPtr() const { return &_position[0]; };
    pxr::GfVec3f* GetPredictedPtr() { return &_predicted[0]; };
    const pxr::GfVec3f* GetPredictedPtr() const { return &_predicted[0]; };

    void ConstrainPositions(float di);
    void ConstrainVelocities();


  private:

    float                            _damping;
    float                            _radius;
    float                            _mass;

    pxr::VtArray<pxr::GfVec3f>       _position;
    pxr::VtArray<pxr::GfVec3f>       _predicted;
    pxr::VtArray<pxr::GfVec3f>       _velocity;

    std::vector<Constraint>          _constraints;
    std::vector</*Static*/Constraint> _staticConstraints;

    pxr::GfBBox3d                    _bbox;
    Geometry*                        _geom;
  };
} // naespace PBD

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
