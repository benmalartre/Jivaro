#ifndef JVR_PBD_BODY_H
#define JVR_PBD_BODY_H

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4f.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

namespace PBD {
  
  struct Body
  {
    float          damping;
    float          radius;
    float          mass;

    size_t         offset;
    size_t         numPoints;
    
    Geometry*      geometry;
  };

  class Particles
  {
  public:
    size_t GetNumParticles() { return _position.size(); };
    void AddBody(Body* body, const pxr::GfMatrix4f& matrix);
    void RemoveBody(Body* body);

    pxr::VtArray<int>& GetBody() { return _body; };
    pxr::VtArray<float>& GetMass() { return _mass; };
    pxr::VtArray<float>& GetRadius() { return _radius; };
    pxr::VtArray<pxr::GfVec3f>& GetPosition() { return _position; };
    pxr::VtArray<pxr::GfVec3f>& GetPredicted() { return _predicted; };
    pxr::VtArray<pxr::GfVec3f>& GetVelocity() { return _velocity; };

    int* GetBodyPtr() { return &_body[0]; };
    const int* GetBodyCPtr() const { return &_body[0]; };
    float* GetMassPtr() { return &_mass[0]; };
    const float* GetMassCPtr() { return &_mass[0]; };
    float* GetRadiusPtr() { return &_radius[0]; };
    const float* GetRadiusCPtr() { return &_radius[0]; };
    pxr::GfVec3f* GetPositionPtr() { return &_position[0]; };
    const pxr::GfVec3f* GetPositionCPtr() const { return &_position[0]; };
    pxr::GfVec3f* GetPredictedPtr() { return &_predicted[0]; };
    const pxr::GfVec3f* GetPredictedCPtr() const { return &_predicted[0]; };
    pxr::GfVec3f* GetVelocityPtr() { return &_velocity[0]; };
    const pxr::GfVec3f* GetVelocityPtr() const { return &_velocity[0]; };

  private:
    pxr::VtArray<int>                   _body;
    pxr::VtArray<float>                 _mass;
    pxr::VtArray<float>                 _radius;
    pxr::VtArray<pxr::GfVec3f>          _position;
    pxr::VtArray<pxr::GfVec3f>          _predicted;
    pxr::VtArray<pxr::GfVec3f>          _velocity;
  };

} // namespace PBD

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_BODY_H
