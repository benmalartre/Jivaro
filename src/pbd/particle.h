#ifndef JVR_PBD_PARTICLE_H
#define JVR_PBD_PARTICLE_H

#include <pxr/base/vt/array.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4f.h>

#include "../common.h"
#include "../geometry/smooth.h"
#include "../pbd/element.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
class Constraint;
class Particles;

struct ConstraintsGroup;

class Body : public Element
{

public:

  Body(Geometry* geom, size_t offset, size_t n, const GfVec3f& color,
    float mass=1.f, float radius=0.01f, float damp=0.1f)
  : Element(Element::BODY)
  , _geometry(geom)
  , _offset(offset)
  , _numPoints(n)
  , _mass(mass)
  , _radius(radius)
  , _damp(damp)
  , _velocity(0.f)
  , _color(color)
  {
    _InitSmoothKernel();
  }

  ~Body();

  size_t GetTypeId() const override {return 0;};
  void UpdateParameters(UsdPrim& prim, float time) override;
  void UpdateParticles(Particles* particles);

  void SetOffset(size_t offset){_offset = offset;};
  void SetNumPoints(size_t numPoints){_numPoints = numPoints;};

  void SetVelocity(const GfVec3f& velocity){_velocity=velocity;};
  void SetTorque(const GfVec3f& torque){_torque=torque;};

  Geometry* GetGeometry() const {return _geometry;};
  size_t GetOffset() const {return _offset;};
  size_t GetNumPoints() const {return _numPoints;};
  float GetMass() const {return _mass;};
  float GetRadius() const {return _radius;};
  float GetDamp() const {return _damp;};
  GfVec3f GetColor() const {return _color;};
  GfVec3f GetVelocity() const {return _velocity;};
  GfVec3f GetTorque() const {return _torque;};

  bool GetSelfCollisionEnabled() const {return _selfCollisionEnabled;};
  float GetSelfCollisionRadius() const {return _selfCollisionRadius;};
  float GetSelfCollisionFriction() const {return _selfCollisionFriction;};
  float GetSelfCollisionDamp() const {return _selfCollisionDamp;};
  //float GetSelfCollisionStiffness() const {return _selfCollisionStiffness;};
  float GetSelfCollisionRestitution() const {return _selfCollisionRestitution;};
  float GetSelfCollisionMaxSeparationVelocity() const {return _selfCollisionMaxSeparationVelocity;};

  size_t GetNumConstraintsGroup();
  ConstraintsGroup* AddConstraintsGroup(const TfToken& group, short type);
  ConstraintsGroup* GetConstraintsGroup(const TfToken& group);

  void SmoothVelocities(Particles* particles, size_t iterations);


protected:
  void                                      _InitSmoothKernel();

  Geometry*                                 _geometry;
  size_t                                    _offset;
  size_t                                    _numPoints;
  bool                                      _simulationEnabled;
  float                                     _mass;
  float                                     _radius;
  float                                     _damp;

  bool                                      _selfCollisionEnabled;
  float                                     _selfCollisionRadius;
  float                                     _selfCollisionFriction;
  float                                     _selfCollisionRestitution;
  float                                     _selfCollisionDamp;
  float                                     _selfCollisionMaxSeparationVelocity;

  GfVec3f                              _color;
  GfVec3f                              _velocity;
  GfVec3f                              _torque;
  std::map<TfToken, ConstraintsGroup*> _constraints;

  std::vector<int>                          _connexions;
  std::vector<int>                          _connexionsCounts;
  std::vector<int>                          _connexionsOffsets;

  Smooth<GfVec3f>*                     _smoothKernel;
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
  enum State {
    MUTE, 
    IDLE, 
    ACTIVE
  };

  Particles() : num(0){};
  ~Particles();

  size_t GetNumParticles() {return num; };
  void AddBody(Body* body, const GfMatrix4d& matrix);
  void RemoveBody(Body* body);
  void RemoveAllBodies();

  void SetAllState(short state);
  void SetBodyState(Body* body, short state);

  void ResetCounter(const std::vector<Constraint*>& constraints, size_t c);

  void _EnsureDataSize(size_t size);

  short*         state;
  Body**         body;
  float*         mass;
  float*         invMass;
  float*         radius;
  GfVec3f*       rest;
  GfQuatf*       rotation;
  GfVec3f*       input;
  GfVec3f*       previous;
  GfVec3f*       position;
  GfVec3f*       predicted;
  GfVec3f*       velocity;
  GfVec3f*       color;
  GfVec2f*       counter;
  size_t         num;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
