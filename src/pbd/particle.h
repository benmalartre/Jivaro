#ifndef JVR_PBD_PARTICLE_H
#define JVR_PBD_PARTICLE_H

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4f.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

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
  int* GetBodyPtr(size_t idx=0) { return &_body[idx]; };
  const int* GetBodyCPtr(size_t idx=0) const { return &_body[idx]; };
  pxr::VtArray<float>& GetMass() { return _mass; };
  float* GetMassPtr(size_t idx=0) { return &_mass[idx]; };
  const float* GetMassCPtr(size_t idx=0) { return &_mass[idx]; };
  pxr::VtArray<float>& GetRadius() { return _radius; };
  float* GetRadiusPtr(size_t idx=0) { return &_radius[idx]; };
  const float* GetRadiusCPtr(size_t idx=0) { return &_radius[idx]; };
  pxr::VtArray<pxr::GfVec3f>& GetPosition() { return _position; };
  pxr::GfVec3f* GetPositionPtr(size_t idx=0) { return &_position[idx]; };
  const pxr::GfVec3f* GetPositionCPtr(size_t idx=0) const { return &_position[idx]; };
  pxr::VtArray<pxr::GfVec3f>& GetPredicted() { return _predicted; };
  pxr::GfVec3f* GetPredictedPtr(size_t idx=0) { return &_predicted[idx]; };
  const pxr::GfVec3f* GetPredictedCPtr(size_t idx=0) const { return &_predicted[idx]; };
  pxr::GfVec3f* GetVelocityPtr(size_t idx=0) { return &_velocity[idx]; };
  const pxr::GfVec3f* GetVelocityCPtr(size_t idx=0) const { return &_velocity[idx]; };
  pxr::VtArray<pxr::GfVec3f>& GetVelocity() { return _velocity; };

private:
  pxr::VtArray<int>          _body;
  pxr::VtArray<float>        _mass;
  pxr::VtArray<float>        _radius;
  pxr::VtArray<pxr::GfVec3f> _position;
  pxr::VtArray<pxr::GfVec3f> _predicted;
  pxr::VtArray<pxr::GfVec3f> _velocity;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
