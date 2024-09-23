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

#include "../pbd/element.h"
#include "../pbd/mask.h"
#include "../geometry/location.h"

JVR_NAMESPACE_OPEN_SCOPE

struct Particles;
struct Body;
class Collision;
class SelfCollision;
class Constraint;


struct ConstraintTargets {
  pxr::VtArray<Body*> bodies;
  pxr::VtArray<int>   elements;
  pxr::TfToken        key;
};

struct ConstraintsGroup {
  pxr::UsdPrim               prim;
  short                      type;
  pxr::VtArray<Constraint*>  constraints;
};

class Constraint: public Element
{
public:
  constexpr static size_t BlockSize = 64;
  constexpr static float EPSILON = 1e-6f;
  enum TypeId {
    ATTACH = 1,
    PIN,
    STRETCH,
    SHEAR,
    BEND,
    DIHEDRAL,
    COLLISION,
    RIGID,
    LAST
  };

  static const int INVALID_INDEX = std::numeric_limits<int>::max();

  Constraint(size_t elementSize, float stiffness, float damping,
    const pxr::VtArray<int>& elems=pxr::VtArray<int>());

  virtual ~Constraint() {};
  virtual size_t GetTypeId() const override = 0;
  virtual size_t GetElementSize() const = 0;
  inline size_t GetNumElements() const{
    return _elements.size() / GetElementSize();
  };
  inline bool HaveElements(){return _elements.size() > 0;};
  const pxr::VtArray<int>& GetElements() {return _elements;};
  virtual void SetElements(const pxr::VtArray<int>& elements);
  virtual void SetStiffness(float stiffness);
  virtual void SetDamp(float damp);
  
  virtual void Solve(Particles* particles, float dt) = 0;

  virtual void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions, 
    pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors) = 0;

  // this one has to be called serially 
  // as two constraints can move the same point
  virtual void Apply(Particles* particles);


protected:
  void _ResetCorrection();

  pxr::VtArray<int>             _elements;
  pxr::VtArray<pxr::GfVec3f>    _correction;
  float                         _compliance;
  float                         _damp;
  pxr::TfToken                  _key;
  pxr::GfVec3f                  _color;
};


static ConstraintsGroup* CreateConstraintsGroup(Body* body, const pxr::TfToken& name, short type, 
  const pxr::VtArray<int>& allElements, size_t elementSize, size_t blockSize, Geometry* target=NULL);


class AttachConstraint : public Constraint
{
public:
  AttachConstraint(Body* body, const pxr::VtArray<int>& elems, 
    float stiffness=0.5f, float damping=0.25f);

  size_t GetTypeId() const override { return TYPE_ID; };
  size_t GetElementSize() const override { return ELEM_SIZE; };

  void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results,
    pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors) override;

  void Solve(Particles* particles, float dt) override;

  static size_t                 ELEM_SIZE;

protected:
  static size_t                 TYPE_ID;

  Body*                         _body;
};

class PinConstraint : public Constraint
{
public:
  PinConstraint(Body* body, const pxr::VtArray<int>& elems, Geometry* target,
    float stiffness, float damping);

  size_t GetTypeId() const override { return TYPE_ID; };
  size_t GetElementSize() const override { return ELEM_SIZE; };

  void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results,
    pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors) override;

  void Solve(Particles* particles, float dt) override;

  static size_t                 ELEM_SIZE;

protected:
  static size_t                 TYPE_ID;
  pxr::VtArray<Location>        _location;
  pxr::VtArray<pxr::GfVec3f>    _offset;
  Geometry*                     _target;
};


class StretchConstraint : public Constraint
{
public:
  StretchConstraint(Body* body, const pxr::VtArray<int>& elems, 
    float stiffness=0.5f, float damping=0.25f);

  size_t GetTypeId() const override { return TYPE_ID; };
  size_t GetElementSize() const override { return ELEM_SIZE; };

  void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results,
    pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors) override;

  void Solve(Particles* particles, float dt) override;

  static size_t                 ELEM_SIZE;

protected:
  static size_t                 TYPE_ID;
  pxr::VtArray<float>           _rest;
};

ConstraintsGroup* CreateStretchConstraints(Body* body, float stiffness=0.5f, float damping=0.1f);

class ShearConstraint : public StretchConstraint
{
public:
 ShearConstraint(Body* body, const pxr::VtArray<int>& elems, 
    float stiffness=0.5f, float damping=0.25f);

  size_t GetTypeId() const override { return TYPE_ID; };

protected:
  static size_t                 TYPE_ID;

};

ConstraintsGroup* CreateShearConstraints(Body* body, float stiffness=0.5f, float damping=0.1f);

class BendConstraint : public Constraint
{
public:
  BendConstraint(Body* body, const pxr::VtArray<int>& elems, 
    float stiffness = 0.1f, float damping=0.25f);

  size_t GetTypeId() const override { return TYPE_ID; };
  size_t GetElementSize() const override { return ELEM_SIZE; };

  void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results, 
    pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors) override;

  void Solve(Particles* particles, float dt) override;

  static size_t                 ELEM_SIZE;

protected:
  static size_t                 TYPE_ID;
  pxr::VtArray<float>           _rest;
};

ConstraintsGroup* CreateBendConstraints(Body* body, float stiffness=1000.f, float damping=0.05f);


class DihedralConstraint : public Constraint
{
public:
  DihedralConstraint(Body* body, const pxr::VtArray<int>& elems,
    float stiffness=0.1f, float damping=0.05f);

  size_t GetTypeId() const override { return TYPE_ID; };
  size_t GetElementSize() const override { return ELEM_SIZE; };

  void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results,
    pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors) override;

  void Solve(Particles* particles, float dt) override;

  static size_t                 ELEM_SIZE;

protected:
  static size_t                 TYPE_ID;
  pxr::VtArray<float>           _rest;

};

ConstraintsGroup* CreateDihedralConstraints(Body* body, float stiffness=0.5f, float damping=0.05f);

class CollisionConstraint : public Constraint
{
  typedef void (CollisionConstraint::*SolveFunc)(Particles* particles, float dt);

public:
  enum Mode {
    GEOM,
    SELF
  };

  CollisionConstraint(Body* body, Collision* collision, const pxr::VtArray<int>& elems,
    float stiffness=0.f, float damping = 0.25f, float restitution = 0.2f, float friction = 0.2f);

  CollisionConstraint(Particles* particles, SelfCollision* collision, const pxr::VtArray<int>& elems,
    float stiffness=0.f, float damping = 0.5f, float restitution = 0.2f, float friction = 0.5f);

  size_t GetTypeId() const override { return TYPE_ID; };
  size_t GetElementSize() const override { return 1; };

  Collision* GetCollision() {return _collision;};
  const Collision* GetCollision() const { return _collision; };
  void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results,
    pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors) override;

  void Solve(Particles* particles, float dt) override;

  // this one has to be called serially 
  // as two constraints can move the same point
  void Apply(Particles* particles) override;


protected:
  void _SolveGeom(Particles* particles, float dt);
  void _SolveSelf(Particles* particles, float dt);

  pxr::GfVec3f _ComputeFriction(const pxr::GfVec3f& correction, const pxr::GfVec3f& relativeVelocity);

  static size_t                 TYPE_ID;
  Mode                          _mode;
  Collision*                    _collision;
  SolveFunc                     _Solve;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H