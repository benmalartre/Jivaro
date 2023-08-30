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
  enum Type {
    STRETCH = 1,
    BEND,
    DIHEDRAL,
    RIGID
  };

  static const int INVALID_INDEX = std::numeric_limits<int>::max();

  Constraint(size_t elementSize, Body* body, float stiffness, float compliance, 
    const pxr::VtArray<int>& elems=pxr::VtArray<int>()) 
    : _elements(elems)
    , _stiffness(stiffness)
    , _compliance(compliance)
  {
    const size_t numElements = elems.size() / elementSize;
    _body.resize(1);
    _body[0] = body;
    _lagrange.resize(numElements);
    _gradient.resize(elementSize);
    _correction.resize(_elements.size());

  }

  Constraint(Body* body1, Body* body2, const float stiffness)
    : _stiffness(stiffness)
  {
    _body.resize(2);
    _body[0] = body1;
    _body[1] = body2;
  }

  virtual ~Constraint() {};
  virtual size_t GetTypeId() const = 0;
  virtual size_t GetElementSize() const = 0;
  inline size_t GetNumElements() const{
    return _elements.size() / GetElementSize();
  };

  virtual bool Solve(Particles* particles, const float dt);
  virtual void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results) = 0;

  // this one has to be called serially 
  // as two constraints can move the same point
  virtual void Apply(Particles* particles);

  pxr::VtArray<Body*>& GetBodies() {return _body;};
  Body* GetBody(size_t index) {return _body[index];};
  const Body* GetBody(size_t index) const {return _body[index];};
  void ResetCorrection();
  virtual void ResetLagrangeMultiplier() {
    memset(&_lagrange[0], 0.f, _lagrange.size()*sizeof(float));
  };

protected:
  virtual float _CalculateValue(Particles* particles, size_t index) = 0;
  virtual void _CalculateGradient(Particles* particles, size_t index) = 0;
  float _ComputeLagrangeMultiplier(Particles* particles, size_t index=0);

  pxr::VtArray<Body*>           _body;
  pxr::VtArray<int>             _elements;
  pxr::VtArray<pxr::GfVec3f>    _correction;
  pxr::VtArray<pxr::GfVec3f>    _gradient;
  pxr::VtArray<float>           _lagrange;
  float                         _stiffness;
  float                         _compliance;
};

class StretchConstraint : public Constraint
{
public:
  StretchConstraint(Body* body, const pxr::VtArray<int>& elems, 
    const float stiffness=0.5f, const float compliance=0.f);

  virtual size_t GetTypeId() const override { return TYPE_ID; };
  virtual size_t GetElementSize() const override { return ELEM_SIZE; };

  void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results) override;

  static size_t                 ELEM_SIZE;

protected:
  float _CalculateValue(Particles* particles, size_t index) override;
  void _CalculateGradient(Particles* particles, size_t index) override;

  static size_t                 TYPE_ID;
  pxr::VtArray<float>           _rest;
};

void CreateStretchConstraints(Body* body, pxr::VtArray<Constraint*>& constraints, 
  const float stiffness=0.5f, const float compliance=0.5f);


class BendConstraint : public Constraint
{
public:
  BendConstraint(Body* body, const pxr::VtArray<int>& elems, 
    const float stiffness = 0.1f, const float compliance=0.5f);

  virtual size_t GetTypeId() const override { return TYPE_ID; };
  virtual size_t GetElementSize() const override { return ELEM_SIZE; };

  void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results) override;

  static size_t                 ELEM_SIZE;

protected:
  float _CalculateValue(Particles* particles, size_t index) override {return 0.f;};
  void _CalculateGradient(Particles* particles, size_t index) override {};

  static size_t                 TYPE_ID;
  pxr::VtArray<float>           _rest;
};

void CreateBendConstraints(Body* body, pxr::VtArray<Constraint*>& constraints,
  const float stiffness=0.5f);


class DihedralConstraint : public Constraint
{
public:
  DihedralConstraint(Body* body, const pxr::VtArray<int>& elems,
    const float stiffness = 0.1f, const float compliance=0.5f);

  virtual size_t GetTypeId() const override { return TYPE_ID; };
  virtual size_t GetElementSize() const override { return ELEM_SIZE; };

  void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results) override;

  static size_t                 ELEM_SIZE;

protected:
  float _CalculateValue(Particles* particles, size_t index) override {return 0.f;};
  void _CalculateGradient(Particles* particles, size_t index) override {};
  static size_t                 TYPE_ID;
  pxr::VtArray<float>           _rest;

};

void CreateDihedralConstraints(Body* body, pxr::VtArray<Constraint*>& constraints,
  const float stiffness=0.5f);

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
