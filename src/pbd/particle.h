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
  PBDParticle();
  ~PBDParticle();

  size_t AddGeometry(PBDGeometry* geom);
  void RemoveGeometry(PBDGeometry* geom);

  void Reset(size_t startIdx, size_t endIdx);
  void Integrate(size_t startIdx, size_t endIdx, float step);
  void AccumulateForces(size_t startIdx, size_t endIdx, const pxr::GfVec3f& gravity);
  
  size_t GetNumParticles() { return _position[0].size(); };
  pxr::GfVec3f& GetPosition(size_t index) { return _position[_flip][index]; };
  const pxr::GfVec3f& GetPosition(size_t index) const { return _position[_flip][index]; };
  pxr::GfVec3f* GetPositions() { return &_position[_flip][0]; };
  const pxr::GfVec3f* GetPositions() const { return &_position[_flip][0]; };
  pxr::GfVec3f* GetPreviousPositions() { return &_previous[_flip][0]; };
  const pxr::GfVec3f* GetPreviousPositions() const { return &_previous[_flip][0]; };
  pxr::GfVec3f* GetRestPositions() { return &_rest[0]; };
  const pxr::GfVec3f* GetRestPositions() const { return &_rest[0]; };
  pxr::GfVec3f* GetInputPositions() { return &_input[0]; };
  const pxr::GfVec3f* GetInputPositions() const { return &_input[0]; };
  pxr::GfVec3f* GetForces() { return &_force[0]; };
  const pxr::GfVec3f* GetForces() const { return &_force[0]; };
  float* GetMasses() { return &_mass[0]; };
  const float* GetMasses() const { return &_mass[0]; };

private:
  bool                                _flip;
  pxr::VtArray<pxr::GfVec3f>          _rest;
  pxr::VtArray<pxr::GfVec3f>          _preload;
  pxr::VtArray<pxr::GfVec3f>          _position[2];
  pxr::VtArray<pxr::GfVec3f>          _previous[2];
  pxr::VtArray<pxr::GfVec3f>          _velocity[2];
  pxr::VtArray<pxr::GfVec3f>          _input;
  pxr::VtArray<pxr::GfVec3f>          _force;
  pxr::VtArray<float>                 _mass;
  pxr::VtArray<float>                 _invMass;
 

};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
