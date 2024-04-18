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

  Geometry* GetGeometry(){return _geometry;};
  size_t GetOffset(){return _offset;};
  size_t GetNumPoints(){return _numPoints;};

  float GetMass(){return _mass;};
  float GetRadius(){return _radius;};
  pxr::GfVec3f GetColor(){return _color;};

protected:
  Geometry*     _geometry;
  size_t        _offset;
  size_t        _numPoints;
  float         _mass;
  float         _radius;
  float         _damping;
  pxr::GfVec3f  _color;
  //bool     _simulated;
};

enum BodyType
{
  RIGID,
  SOFT,
  CLOTH,
  HAIR
};

class Particles : public Element
{
public:
  enum State {MUTE, IDLE, ACTIVE};

  Particles() : Element(Element::PARTICLES){};

  size_t GetNumParticles() { return _position.size(); };
  void AddBody(Body* body, const pxr::GfMatrix4f& matrix);
  void RemoveBody(Body* body);
  void RemoveAllBodies();

  void SetAllState(short state);
  void SetBodyState(Body* body, short state);

  pxr::VtArray<short>        _state;
  pxr::VtArray<int>          _body;
  pxr::VtArray<float>        _mass;
  pxr::VtArray<float>        _invMass;
  pxr::VtArray<float>        _radius;
  pxr::VtArray<pxr::GfVec3f> _rest;
  pxr::VtArray<pxr::GfVec3f> _previous;
  pxr::VtArray<pxr::GfVec3f> _position;
  pxr::VtArray<pxr::GfVec3f> _predicted;
  pxr::VtArray<pxr::GfVec3f> _velocity;
  pxr::VtArray<pxr::GfVec3f> _color;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
