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
class Constraint;
struct ConstraintsGroup;

class Body : public Element
{

public:

  Body(Geometry* geom, size_t offset, size_t n, const pxr::GfVec3f& color,
    float mass=1.f, float radius=0.01f, float damp=0.1f)
  : Element(Element::BODY)
  , _geometry(geom)
  , _offset(offset)
  , _numPoints(n)
  , _mass(mass)
  , _radius(radius)
  , _damp(damp)
  , _velocity(0.f)
  , _color(color){}

  size_t GetTypeId() const override {return 0;};
  void UpdateParameters(pxr::UsdPrim& prim, float time) override;

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

  size_t GetNumConstraintsGroup();
  ConstraintsGroup* AddConstraintsGroup(const pxr::TfToken& group, short type);
  ConstraintsGroup* GetConstraintsGroup(const pxr::TfToken& group);


protected:
  Geometry*                                 _geometry;
  size_t                                    _offset;
  size_t                                    _numPoints;
  float                                     _mass;
  float                                     _radius;
  float                                     _damp;
  float                                     _friction;
  float                                     _restitution;
  pxr::GfVec3f                              _color;
  pxr::GfVec3f                              _velocity;
  pxr::GfVec3f                              _torque;
  std::map<pxr::TfToken, ConstraintsGroup*> _constraints;

  std::vector<int>                          _connexions;
  std::vector<int>                          _connexionsCounts;
  std::vector<int>                          _connexionsOffsets;
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
  const static size_t BLOCK_SIZE = 1024;
  enum State {MUTE, IDLE, ACTIVE};

  Particles() :num(0){};
  ~Particles();

  size_t GetNumParticles() { return num; };
  void AddBody(Body* body, const pxr::GfMatrix4d& matrix);
  void RemoveBody(Body* body);
  void RemoveAllBodies();

  void SetAllState(short state);
  void SetBodyState(Body* body, short state);

  void ResetCounter(const std::vector<Constraint*>& constraints, size_t c);

  void _EnsureDataSize(size_t size);

  short*        state;
  int*          body;
  float*        mass;
  float*        invMass;
  float*        radius;
  pxr::GfVec3f* rest;
  pxr::GfQuatf* rotation;
  pxr::GfVec3f* input;
  pxr::GfVec3f* position;
  pxr::GfVec3f* predicted;
  pxr::GfVec3f* velocity;
  pxr::GfVec3f* color;
  pxr::GfVec2f* counter;
 size_t         num;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
