#ifndef JVR_PBD_PARTICLE_H
#define JVR_PBD_PARTICLE_H

#include <pxr/base/vt/array.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4f.h>

#include "../common.h"
#include "../pbd/element.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

class Body : public Element
{

public:
  Body(Geometry* geom, size_t offset, size_t n, const pxr::GfVec3f& color,
    float mass=1.f, float radius=0.01f, float damping=0.1f)
  : Element(Element::BODY)
  , _geometry(geom)
  , _offset(offset)
  , _numPoints(n)
  , _mass(mass)
  , _radius(radius)
  , _damping(damping)
  , _color(color){}

  void SetOffset(size_t offset){_offset = offset;};
  void SetNumPoints(size_t numPoints){_numPoints = numPoints;};

  void SetVelocity(const pxr::GfVec3f& velocity){_velocity=velocity;};
  void SetTorque(const pxr::GfVec3f& torque){_torque=torque;};

  Geometry* GetGeometry() const {return _geometry;};
  size_t GetOffset() const {return _offset;};
  size_t GetNumPoints() const {return _numPoints;};
  float GetMass() const {return _mass;};
  float GetRadius() const {return _radius;};
  pxr::GfVec3f GetColor() const {return _color;};
  pxr::GfVec3f GetVelocity() const {return _velocity;};
  pxr::GfVec3f GetTorque() const {return _torque;};

protected:
  Geometry*     _geometry;
  size_t        _offset;
  size_t        _numPoints;
  float         _mass;
  float         _radius;
  float         _damping;
  pxr::GfVec3f  _color;
  pxr::GfVec3f  _velocity;
  pxr::GfVec3f  _torque;

  //bool     _simulated;
};

enum BodyType
{
  RIGID,
  SOFT,
  CLOTH,
  HAIR
};

struct Particles
{
  enum State {MUTE, IDLE, ACTIVE};

  size_t GetNumParticles() { return position.size(); };
  void AddBody(Body* body, const pxr::GfMatrix4d& matrix);
  void RemoveBody(Body* body);
  void RemoveAllBodies();

  void SetAllState(short state);
  void SetBodyState(Body* body, short state);

  pxr::VtArray<short>        state;
  pxr::VtArray<int>          body;
  pxr::VtArray<float>        mass;
  pxr::VtArray<float>        invMass;
  pxr::VtArray<float>        radius;
  pxr::VtArray<pxr::GfVec3f> rest;
  pxr::VtArray<pxr::GfVec3f> previous;
  pxr::VtArray<pxr::GfVec3f> position;
  pxr::VtArray<pxr::GfVec3f> predicted;
  pxr::VtArray<pxr::GfVec3f> velocity;
  pxr::VtArray<pxr::GfVec3f> color;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
