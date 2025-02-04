#ifndef JVR_PBD_FORCE_H
#define JVR_PBD_FORCE_H

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/usd/usd/attribute.h>

#include "../common.h"
#include "../pbd/mask.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

struct Particles;
struct Body;

class Force : public Mask
{
public:
  enum Type { GRAVITY, DAMP, NOISE, WAVE};

  Force() : Mask(Element::FORCE) {};
  virtual ~Force() {};
  
  void AddBody(Particles* particles, Body* body);
  void RemoveBody(Particles* particles, Body* body);

  virtual void Update(float time) = 0;
  virtual void Apply(size_t begin, size_t end, Particles* particles, float dt) const = 0;

  virtual size_t GetTypeId() const override = 0;
};

class GravityForce : public Force
{
public:
  GravityForce(const GfVec3f& gravity=GfVec3f(0.f, -9.81f, 0.f));
  GravityForce(const UsdAttribute& attr);

  void Set(const GfVec3f& gravity){_gravity = gravity;};

  void Update(float time);
  void Apply(size_t begin, size_t end, Particles* particles, float dt) const override;
  virtual size_t GetTypeId() const {return TYPE_ID;};

private:
  static size_t         TYPE_ID;
  UsdAttribute     _attr;
  GfVec3f          _gravity;
};

class DampForce : public Force
{
public:

  DampForce(float damping=0.1f);
  DampForce(const UsdAttribute& attr);

  void Set(float damp){_damp = damp;};

  void Update(float time);
  void Apply(size_t begin, size_t end, Particles* particles, float dt) const override;
  virtual size_t GetTypeId() const {return TYPE_ID;};

private:
  static size_t         TYPE_ID;
  UsdAttribute     _attr;
  float _damp;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_FORCE_H
