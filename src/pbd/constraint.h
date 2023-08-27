#ifndef JVR_PBD_CONSTRAINT_H
#define JVR_PBD_CONSTRAINT_H

#include <map>
#include <float.h>

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec3i.h>
#include <pxr/base/gf/vec4i.h>
#include <pxr/base/gf/matrix4f.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

struct Particles;
struct Body;

class Constraint
{
public:
  constexpr static size_t BlockSize = 256;
  enum ConstraintType {
    STRETCH = 1,
    BEND,
    DIHEDRAL,
    RIGID
  };

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
  virtual bool Solve(Particles* particles) = 0;
  virtual void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results) = 0;

  // this one has to be called serially 
  // as two constraints can move the same point
  virtual void Apply(Particles* particles, const float di) = 0;

  pxr::VtArray<Body*>& GetBodies() {return _body;};
  Body* GetBody(size_t index) {return _body[index];};
  const Body* GetBody(size_t index) const {return _body[index];};
  void ResetCorrection();

protected:
  pxr::VtArray<Body*>        _body;
  pxr::VtArray<pxr::GfVec3f> _correction;
};

class StretchConstraint : public Constraint
{
public:
  StretchConstraint(Body* body, const float stretchStiffness=0.5f, 
    const float compressionStiffness=0.5f);
  StretchConstraint(Body* body, const pxr::VtArray<pxr::GfVec2i>& edges,
    const float stretchStiffness=0.5f, const float compressionStiffness=0.5f);

  virtual size_t& GetTypeId() const override { return TYPE_ID; }

  bool Solve(Particles* particles) override;
  void Apply(Particles* particles, const float di) override;
  void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results) override;
  

protected:
  static size_t                 TYPE_ID;
  pxr::VtArray<float>           _rest;
  pxr::VtArray<pxr::GfVec2i>    _edges;
  float                         _stretch;
  float                         _compression;
};

void CreateStretchConstraints(Body* body, pxr::VtArray<Constraint*>& constraints,
  const float stretchStiffness=0.5f, const float compressionStiffness=0.5f);


class BendConstraint : public Constraint
{
public:
  BendConstraint(Body* body, const float stiffness = 0.1f);
  BendConstraint(Body* body, const pxr::VtArray<pxr::GfVec3i>& edges, 
    const float stiffness = 0.1f);
  virtual size_t& GetTypeId() const override { return TYPE_ID; }

  bool Solve(Particles* particles) override;
  void Apply(Particles* particles, const float di) override;
  void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results) override;

protected:
  static size_t                 TYPE_ID;
  pxr::VtArray<float>           _rest;
  pxr::VtArray<pxr::GfVec3i>    _edges;
  float                         _stiffness;
};

void CreateBendConstraints(Body* body, pxr::VtArray<Constraint*>& constraints);


class DihedralConstraint : public Constraint
{
public:
  DihedralConstraint(Body* body, const float stiffness = 0.1f);
  DihedralConstraint(Body* body, const pxr::VtArray<pxr::GfVec4i>& edges,
    const float stiffness = 0.1f);
  virtual size_t& GetTypeId() const override { return TYPE_ID; }

  bool Solve(Particles* particles) override;
  void Apply(Particles* particles, const float di) override;
  void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results) override;

protected:
  static size_t                 TYPE_ID;
  pxr::VtArray<float>           _rest;
  pxr::VtArray<pxr::GfVec4i>    _vertices;
  float                         _stiffness;

};

void CreateDihedralConstraints(Body* body, pxr::VtArray<Constraint*>& constraints);

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
