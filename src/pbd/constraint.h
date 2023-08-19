#ifndef JVR_PBD_CONSTRAINT_H
#define JVR_PBD_CONSTRAINT_H

#include <map>
#include <float.h>

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

struct Particles;
struct Body;

class Constraint
{
public:
  static const int INVALID_INDEX = std::numeric_limits<int>::max();

  Constraint(Body* body)
  {
    _body.resize(1);
    _body[0] = body;
  }

  Constraint(Body* body1, Body* body2)
  {
    _body.resize(2);
    _body[0] = body1;
    _body[1] = body2;
  }

  virtual size_t GetNumParticles();
  virtual ~Constraint() {};
  virtual size_t& GetTypeId() const = 0;

  virtual bool Update(Particles* particles) { return true; };
  virtual bool Solve(Particles* particles, const float di) = 0;

  // this one has to be called serially 
  // as two constraints can move the same point
  virtual void Apply(Particles* particles) = 0;

  pxr::VtArray<Body*>& GetBodies() {return _body;};
  Body* GetBody(size_t index) {return _body[index];};
  const Body* GetBody(size_t index) const {return _body[index];};

protected:
  enum ConstraintType {
    STRETCH = 1,
    BEND,
    RIGID
  };

  pxr::VtArray<Body*>        _body;
  pxr::VtArray<pxr::GfVec3f> _correction;
};

class StretchConstraint : public Constraint
{
public:
  StretchConstraint(Body* body, const float stretchStiffness=0.5f, const float compressionStiffness=0.5f);
  virtual size_t& GetTypeId() const { return TYPE_ID; }

  bool Solve(Particles* particles, const float di) override;
  void Apply(Particles* particles) override;

protected:
  static size_t         TYPE_ID;
  pxr::VtArray<float>   _rest;
  pxr::VtArray<int>     _edges;
  float                 _stretch;
  float                 _compression;
  
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
