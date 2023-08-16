#ifndef JVR_PBD_FORCE_H
#define JVR_PBD_FORCE_H

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>

#include "../common.h"
#include "../pbd/mask.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

struct Particles;
struct Body;

class Force : public Mask
{
public:
  Force() : Mask() {};
  bool HasWeights() const { return _weights.size() > 0; };
  void SetWeights(const pxr::VtArray<float>& weights) { _weights = weights; };
  void RemoveWeights() { _weights.clear(); };

  void AddBody(Particles* particles, Body* body);
  void RemoveBody(Particles* particles, Body* body);

  virtual void Apply(size_t begin, size_t end, pxr::GfVec3f* velocities, float dt) const = 0;

protected:
  pxr::VtArray<float> _weights;   // if mask weights size must equals mask size 
                                  // else weights size must equals num particles
};

class GravitationalForce : public Force
{
public:
  GravitationalForce();
  GravitationalForce(const pxr::GfVec3f& gravity);

  void Apply(size_t begin, size_t end, pxr::GfVec3f* velocities, float dt) const override;

private:
  pxr::GfVec3f _gravity;
};

class DampingForce : public Force
{
public:
  DampingForce();
  DampingForce(float damping);

  void Apply(size_t begin, size_t end, pxr::GfVec3f* velocities, float dt) const override;

private:
  float _damp;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_FORCE_H
