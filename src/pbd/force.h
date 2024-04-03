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
  virtual ~Force() {};
  

  void AddBody(Particles* particles, Body* body);
  void RemoveBody(Particles* particles, Body* body);

  virtual void Apply(size_t begin, size_t end, Particles* particles, float dt) const = 0;
};

class GravitationalForce : public Force
{
public:
  GravitationalForce();
  GravitationalForce(const pxr::GfVec3f& gravity);

  void Apply(size_t begin, size_t end, Particles* particles, float dt) const override;

private:
  pxr::GfVec3f _gravity;
};

class DampingForce : public Force
{
public:
  DampingForce();
  DampingForce(float damping);

  void Apply(size_t begin, size_t end, Particles* particles, float dt) const override;

private:
  float _damp;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_FORCE_H
