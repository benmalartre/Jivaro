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
  constexpr static size_t BlockSize = 1024;
  enum ConstraintType {
    STRETCH = 1,
    BEND,
    DIHEDRAL,
    RIGID
  };

  static const int INVALID_INDEX = std::numeric_limits<int>::max();

  Constraint(Body* body, const float stiffness, const pxr::VtArray<int>& elems=pxr::VtArray<int>()) 
    : _elements(elems)
    , _stiffness(stiffness)
  {
    _body.resize(1);
    _body[0] = body;
  }

  Constraint(Body* body1, Body* body2, const float stiffness)
    : _stiffness(stiffness)
  {
    _body.resize(2);
    _body[0] = body1;
    _body[1] = body2;
  }

  virtual ~Constraint() {};
  virtual size_t& GetTypeId() const = 0;
  virtual size_t& GetElementSize() const = 0;

  virtual bool Update(Particles* particles) { return true; };
  virtual bool Solve(Particles* particles) = 0;
  virtual void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results) = 0;

  // this one has to be called serially 
  // as two constraints can move the same point
  virtual void Apply(Particles* particles, const float di);

  pxr::VtArray<Body*>& GetBodies() {return _body;};
  Body* GetBody(size_t index) {return _body[index];};
  const Body* GetBody(size_t index) const {return _body[index];};
  void ResetCorrection();

protected:
  pxr::VtArray<Body*>        _body;
  pxr::VtArray<int>          _elements;
  pxr::VtArray<pxr::GfVec3f> _correction;
  float                      _stiffness;
};

class StretchConstraint : public Constraint
{
public:
  StretchConstraint(Body* body, const pxr::VtArray<int>& elems, 
    const float stiffness=0.5f);

  virtual size_t& GetTypeId() const override { return TYPE_ID; };
  virtual size_t& GetElementSize() const override { return ELEM_SIZE; };

  bool Solve(Particles* particles) override;
  void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results) override;

  static size_t                 ELEM_SIZE;

protected:
  static size_t                 TYPE_ID;
  pxr::VtArray<float>           _rest;
};

void CreateStretchConstraints(Body* body, pxr::VtArray<Constraint*>& constraints, 
  const float stiffness=0.5f);


class BendConstraint : public Constraint
{
public:
  BendConstraint(Body* body, const pxr::VtArray<int>& elems, 
    const float stiffness = 0.1f);

  virtual size_t& GetTypeId() const override { return TYPE_ID; };
  virtual size_t& GetElementSize() const override { return ELEM_SIZE; };

  bool Solve(Particles* particles) override;
  void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results) override;

  static size_t                 ELEM_SIZE;

protected:
  static size_t                 TYPE_ID;
  pxr::VtArray<float>           _rest;
};

void CreateBendConstraints(Body* body, pxr::VtArray<Constraint*>& constraints,
  const float stiffness=0.5f);


class DihedralConstraint : public Constraint
{
public:
  DihedralConstraint(Body* body, const pxr::VtArray<int>& elems,
    const float stiffness = 0.1f);

  virtual size_t& GetTypeId() const override { return TYPE_ID; };
  virtual size_t& GetElementSize() const override { return ELEM_SIZE; };

  bool Solve(Particles* particles) override;
  void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results) override;

  static size_t                 ELEM_SIZE;

protected:
  static size_t                 TYPE_ID;
  pxr::VtArray<float>           _rest;

};

void CreateDihedralConstraints(Body* body, pxr::VtArray<Constraint*>& constraints,
  const float stiffness=0.5f);

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
