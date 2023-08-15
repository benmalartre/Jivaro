#ifndef JVR_PBD_FORCE_H
#define JVR_PBD_FORCE_H

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;


class Particles;
struct Body;
class Force
{
public:
  bool HasMask() const { return _mask.size() > 0; };
  void SetMask(const pxr::VtArray<int>& mask) { _mask = mask; };
  void RemoveMask() { _mask.clear(); }
  size_t MaskSize() const { return _mask.size(); };
  const int* GetMaskCPtr() const { return &_mask[0]; };
  bool HasWeights() const { return _weights.size() > 0; };
  void SetWeights(const pxr::VtArray<float>& weights) { _weights = weights; };
  void RemoveWeights() { _weights.clear(); };

  bool Affects(size_t index) const;

  void AddBody(Particles* particles, Body* body);
  void RemoveBody(Particles* particles, Body* body);

  virtual void Apply(pxr::GfVec3f* velocities, const float dt, const size_t index) const = 0;

protected:
  pxr::VtArray<int>   _mask;      // mask is in the list of active point indices
  pxr::VtArray<float> _weights;   // if mask weights size must equals mask size 
                                  // else weights size must equals num particles
};

class GravitationalForce : public Force
{
public:
  GravitationalForce();
  GravitationalForce(const pxr::GfVec3f& gravity);

  void Apply(pxr::GfVec3f* velocities, const float dt, const size_t index) const override;

private:
  pxr::GfVec3f _gravity;
};

class DampingForce : public Force
{
public:
  DampingForce();
  DampingForce(float damping);

  void Apply(pxr::GfVec3f* velocities, const float dt, const size_t index) const override;

private:
  float _damp;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_FORCE_H
