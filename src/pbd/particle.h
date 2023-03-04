#ifndef JVR_PBD_PARTICLE_H
#define JVR_PBD_PARTICLE_H

#include <map>
#include <float.h>
#include <pxr/base/vt/array.h>
#include <pxr/base/gf/matrix4f.h>

#include "../common.h"
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

struct PBDGeometry 
{
  Geometry*       ptr;
  size_t          offset;
  pxr::GfMatrix4f matrix;
  pxr::GfMatrix4f invMatrix;
};

class PBDParticle
{
public:
  size_t AddGeometry(PBDGeometry* geom);
  void RemoveGeometry(PBDGeometry* geom);

  void Reset(size_t startIdx, size_t endIdx);
  void Integrate(size_t startIdx, size_t endIdx, float step);
  void AccumulateForces(size_t startIdx, size_t endIdx, const pxr::GfVec3f& gravity);
  
  size_t GetNumParticles() { return _position.size(); };
  pxr::GfVec3f& GetPosition(size_t index) { return _position[index]; };
  const pxr::GfVec3f& GetPosition(size_t index) const { return _position[index]; };
  const pxr::VtArray<pxr::GfVec3f>& Get() const { return _position; };
  pxr::GfVec3f* GetPositions() { return &_position[0]; };
  const pxr::GfVec3f* GetPositions() const { return &_position[0]; };
  pxr::GfVec3f* GetPreviousPositions() { return &_previous[0]; };
  const pxr::GfVec3f* GetPreviousPositions() const { return &_previous[0]; };
  pxr::GfVec3f* GetRestPositions() { return &_rest[0]; };
  const pxr::GfVec3f* GetRestPositions() const { return &_rest[0]; };
  pxr::GfVec3f& GetRestPosition(size_t index) { return _rest[index]; };
  pxr::GfVec3f* GetInputPositions() { return &_input[0]; };
  const pxr::GfVec3f* GetInputPositions() const { return &_input[0]; };
  pxr::GfVec3f* GetForcesPtr() { return &_force[0]; };
  const pxr::GfVec3f* GetForcesCPtr() const { return &_force[0]; };
  float* GetMassesPtr() { return &_mass[0]; };
  const float* GetMassesCPtr() const { return &_mass[0]; };

private:
  pxr::VtArray<pxr::GfVec3f>  _rest;
  pxr::VtArray<pxr::GfVec3f>  _preload;
  pxr::VtArray<pxr::GfVec3f>  _position;
  pxr::VtArray<pxr::GfVec3f>  _previous;
  pxr::VtArray<pxr::GfVec3f>  _velocity;
  pxr::VtArray<pxr::GfVec3f>  _input;
  pxr::VtArray<pxr::GfVec3f>  _force;
  pxr::VtArray<float>         _mass;
  pxr::VtArray<float>         _invMass;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
