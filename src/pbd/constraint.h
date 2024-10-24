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
#include "../pbd/particle.h"
#include "../pbd/collision.h"
#include "../pbd/mask.h"
#include "../geometry/location.h"

JVR_NAMESPACE_OPEN_SCOPE

struct Body;
class Collision;
class SelfCollision;
class Constraint;


struct ConstraintTargets {
  VtArray<Body*> bodies;
  VtArray<int>   elements;
  TfToken        key;
};

struct ConstraintsGroup {
  UsdPrim               prim;
  TfToken               name;
  short                      type;
  VtArray<Constraint*>  constraints;
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
    CONTACT,
    LAST
  };

  static const int INVALID_INDEX = std::numeric_limits<int>::max();

  Constraint(size_t elementSize, float stiffness, float damping,
    const VtArray<int>& elems=VtArray<int>());

  virtual ~Constraint(){};
  virtual size_t GetTypeId() const override = 0;
  virtual size_t GetElementSize() const = 0;
  inline size_t GetNumElements() const{
    return _elements.size() / GetElementSize();
  };
  inline bool HaveElements(){return _elements.size() > 0;};
  const VtArray<int>& GetElements() {return _elements;};
  virtual void SetElements(const VtArray<int>& elements);
  virtual void SetStiffness(float stiffness);
  virtual void SetDamp(float damp);
  virtual void SetActive(bool active){_active=active;};

  bool IsActive() {return _active;};
  
  virtual void Reset(Particles* particles);
  virtual void SolvePosition(Particles* particles, float dt) = 0;
  virtual void SolveVelocity(Particles* particles, float dt){};

  virtual void GetPoints(Particles* particles, VtArray<GfVec3f>& positions, 
    VtArray<float>& radius, VtArray<GfVec3f>& colors) = 0;

  // this two has to be called serially 
  // as two constraints can move the same point
  virtual void ApplyPosition(Particles* particles);
  virtual void ApplyVelocity(Particles* particles);


protected:
  void _ResetCorrection();
  bool                          _active;
  VtArray<int>             _elements;
  VtArray<GfVec3f>    _gradient;
  VtArray<GfVec3f>    _correction;
  double                        _compliance;
  float                         _damp;
  TfToken                  _key;
  GfVec3f                  _color;
};


static ConstraintsGroup* CreateConstraintsGroup(Body* body, const TfToken& name, short type, 
  const VtArray<int>& allElements, size_t elementSize, size_t blockSize, void* data=NULL);


class AttachConstraint : public Constraint
{
public:
  AttachConstraint(Body* body, const VtArray<int>& elems, 
    float stiffness=0.5f, float damping=0.25f);

  size_t GetTypeId() const override { return TYPE_ID; };
  size_t GetElementSize() const override { return ELEM_SIZE; };

  void GetPoints(Particles* particles, VtArray<GfVec3f>& results,
    VtArray<float>& radius, VtArray<GfVec3f>& colors) override;


  void SolvePosition(Particles* particles, float dt) override;

  static size_t                 ELEM_SIZE;

protected:
  static size_t                 TYPE_ID;
  Body*                         _body;
};

ConstraintsGroup* CreateAttachConstraints(Body* body, float stiffness=0.5f, float damping=0.1f, 
  const VtArray<int> *elements=nullptr);

class PinConstraint : public Constraint
{
public:
  PinConstraint(Body* body, const VtArray<int>& elems, Geometry* target, 
    float stiffness=0.f, float damping=0.f);

  size_t GetTypeId() const override { return TYPE_ID; };
  size_t GetElementSize() const override { return ELEM_SIZE; };

  void GetPoints(Particles* particles, VtArray<GfVec3f>& results,
    VtArray<float>& radius, VtArray<GfVec3f>& colors) override;

  void SolvePosition(Particles* particles, float dt) override;

  static size_t                 ELEM_SIZE;

protected:
  static size_t                 TYPE_ID;
  VtArray<Location>        _location;
  VtArray<GfVec3f>    _offset;
  Geometry*                     _target;
};

ConstraintsGroup* CreatePinConstraints(Body* body, Geometry* target, float stiffness=0.5f, float damping=0.1f,
  const VtArray<int> *elements=nullptr);


class StretchConstraint : public Constraint
{
public:
  StretchConstraint(Body* body, const VtArray<int>& elems, 
    float stiffness=0.5f, float damping=0.25f);

  size_t GetTypeId() const override { return TYPE_ID; };
  size_t GetElementSize() const override { return ELEM_SIZE; };

  void GetPoints(Particles* particles, VtArray<GfVec3f>& results,
    VtArray<float>& radius, VtArray<GfVec3f>& colors) override;

  void Reset(Particles* particles) override;
  void SolvePosition(Particles* particles, float dt) override;

  static size_t                 ELEM_SIZE;

protected:
  static size_t                 TYPE_ID;
  VtArray<float>           _rest;
};

ConstraintsGroup* CreateStretchConstraints(Body* body, float stiffness=0.5f, float damping=0.1f);

class ShearConstraint : public StretchConstraint
{
public:
 ShearConstraint(Body* body, const VtArray<int>& elems, 
    float stiffness=0.5f, float damping=0.25f);

  size_t GetTypeId() const override { return TYPE_ID; };

protected:
  static size_t                 TYPE_ID;

};

ConstraintsGroup* CreateShearConstraints(Body* body, float stiffness=0.5f, float damping=0.1f);

class BendConstraint : public Constraint
{
public:
  BendConstraint(Body* body, const VtArray<int>& elems, 
    float stiffness = 0.1f, float damping=0.25f);

  size_t GetTypeId() const override { return TYPE_ID; };
  size_t GetElementSize() const override { return ELEM_SIZE; };

  void GetPoints(Particles* particles, VtArray<GfVec3f>& results, 
    VtArray<float>& radius, VtArray<GfVec3f>& colors) override;

  void SolvePosition(Particles* particles, float dt) override;

  static size_t                 ELEM_SIZE;

protected:
  static size_t                 TYPE_ID;
  VtArray<float>           _rest;
};

ConstraintsGroup* CreateBendConstraints(Body* body, float stiffness=1000.f, float damping=0.05f);


class DihedralConstraint : public Constraint
{
public:
  DihedralConstraint(Body* body, const VtArray<int>& elems,
    float stiffness=0.1f, float damping=0.05f);

  size_t GetTypeId() const override { return TYPE_ID; };
  size_t GetElementSize() const override { return ELEM_SIZE; };

  void GetPoints(Particles* particles, VtArray<GfVec3f>& results,
    VtArray<float>& radius, VtArray<GfVec3f>& colors) override;

  void SolvePosition(Particles* particles, float dt) override;

  static size_t                 ELEM_SIZE;

protected:
  static size_t                 TYPE_ID;
  VtArray<float>           _rest;

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

  CollisionConstraint(Body* body, Collision* collision, const VtArray<int>& elems,
    float stiffness=0.f, float damping = 0.25f, float restitution = 0.2f, float friction = 0.2f);

  CollisionConstraint(Particles* particles, SelfCollision* collision, const VtArray<int>& elems,
    float stiffness=0.f, float damping = 0.5f, float restitution = 0.2f, float friction = 0.5f);

  size_t GetTypeId() const override { return TYPE_ID; };
  size_t GetElementSize() const override { return 1; };

  Collision* GetCollision() {return _collision;};
  const Collision* GetCollision() const { return _collision; };
  void GetPoints(Particles* particles, VtArray<GfVec3f>& results,
    VtArray<float>& radius, VtArray<GfVec3f>& colors) override;

  void SolvePosition(Particles* particles, float dt) override;
  void SolveVelocity(Particles* particles, float dt) override;

  // this one has to be called serially 
  // as two constraints can move the same point
  void ApplyPosition(Particles* particles) override;
  void ApplyVelocity(Particles* particles) override;


protected:
  void _SolvePositionGeom(Particles* particles, float dt);
  void _SolvePositionSelf(Particles* particles, float dt);

  void _SolveVelocityGeom(Particles* particles, float dt);
  void _SolveVelocitySelf(Particles* particles, float dt);

  GfVec3f _ComputeFriction(const float friction, const GfVec3f& correction, 
    const GfVec3f& relativeVelocity);

  static size_t                 TYPE_ID;
  Mode                          _mode;
  Collision*                    _collision;
  SolveFunc                     _SolvePosition;
  SolveFunc                     _SolveVelocity;

};

class ContactConstraint : public Constraint
{
public:
  ContactConstraint(Body* body, const VtArray<int>& elems, Collision* collision, 
    const VtArray<Contact*>& contact, float stiffness=0.f, float damping=0.f);

  size_t GetTypeId() const override { return TYPE_ID; };
  size_t GetElementSize() const override { return ELEM_SIZE; };

  void GetPoints(Particles* particles, VtArray<GfVec3f>& results,
    VtArray<float>& radius, VtArray<GfVec3f>& colors) override;

  void SolvePosition(Particles* particles, float dt) override;

  static size_t                 ELEM_SIZE;

protected:
  static size_t                 TYPE_ID;
  VtArray<Contact>         _contacts;
  Collision*                    _collision;
};

ConstraintsGroup* CreateContactConstraints(Body* body, Collision* collision, float stiffness=0.f, float damping=1.f,
  const VtArray<int> *elements=nullptr);

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H