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
  size_t offset;
  pxr::GfMatrix4f matrix;
  pxr::GfMatrix4f invMatrix;
};

class PBDParticle
{
public:
  PBDParticle();
  ~PBDParticle();

  size_t AddGeometry(Geometry* geom, const pxr::GfMatrix4f& m);
  void RemoveGeometry(Geometry* geom);

  void UpdateInput(Geometry* geom, const pxr::VtArray<pxr::GfVec3f>& p, 
      const pxr::GfMatrix4f& m);

  void Integrate(float step);
  void AccumulateForces(const pxr::GfVec3f& gravity);
  void UpdateGeometries();
  void Reset();
  size_t GetNumParticles() { return _N; };
  pxr::GfVec3f& GetPosition(size_t index) { return _position[_flip][index]; };
  const pxr::GfVec3f& GetPosition(size_t index) const { return _position[_flip][index]; };
  pxr::VtArray<pxr::GfVec3f>& GetPositions() { return _position[_flip]; };
  const pxr::VtArray<pxr::GfVec3f>& GetPositions() const { return _position[_flip]; };
  pxr::VtArray<pxr::GfVec3f>& GetPreviousPositions() { return _previous[_flip]; };
  const pxr::VtArray<pxr::GfVec3f>& GetPreviousPositions() const { return _previous[_flip]; };
  pxr::VtArray<pxr::GfVec3f>& GetInitPositions() { return _initial; };
  const pxr::VtArray<pxr::GfVec3f>& GetInitPositions() const { return _initial; };
  pxr::VtArray<pxr::GfVec3f>& GetInputPositions() { return _input; };
  const pxr::VtArray<pxr::GfVec3f>& GetInputPositions() const { return _input; };
  pxr::VtArray<pxr::GfVec3f>& GetForces() { return _force; };
  const pxr::VtArray<pxr::GfVec3f>& GetForces() const { return _force; };
  pxr::VtArray<float>& GetMasses() { return _mass; };
  const pxr::VtArray<float>& GetMasses() const { return _mass; };

private:
  size_t                              _N;
  bool                                _flip;
  pxr::VtArray<pxr::GfVec3f>          _initial;
  pxr::VtArray<pxr::GfVec3f>          _preload;
  pxr::VtArray<pxr::GfVec3f>          _position[2];
  pxr::VtArray<pxr::GfVec3f>          _previous[2];
  pxr::VtArray<pxr::GfVec3f>          _input;
  pxr::VtArray<pxr::GfVec3f>          _force;
  pxr::VtArray<float>                 _mass;

  std::map<Geometry*, PBDGeometry>    _geometries;

};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
