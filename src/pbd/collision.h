#ifndef JVR_PBD_COLLISION_H
#define JVR_PBD_COLLISION_H

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4f.h>

#include "../common.h"
#include "../geometry/intersection.h"
#include "../pbd/mask.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

struct Particles;
struct Body;
class Constraint;
class Solver;

class Collision : public Mask
{
public:
  enum Type {
    PLANE = 1,
    SPHERE,
    MESH,
    SELF
  };

  Collision(Geometry* collider, float restitution=0.5f, float friction=0.5f) 
    : Mask(Element::COLLISION)
    , _collider(collider)
    , _restitution(restitution)
    , _friction(friction) {};

  /*
  void AddBody(Particles* particles, Body* body);
  void RemoveBody(Particles* particles, Body* body);
  */
  virtual ~Collision() {};
  virtual size_t GetTypeId() const = 0;

  virtual void Update(const pxr::UsdPrim& prim, double time);
  virtual void FindContacts(Particles* particles, const pxr::VtArray<Body*>& bodies,
    pxr::VtArray<Constraint*>& constraints, float ft);
  virtual void FindContactsSerial(Particles* particles, const pxr::VtArray<Body*>& bodies,
    pxr::VtArray<Constraint*>& constraint, float ft);

  virtual void StoreContactsLocation(Particles* particles, int* elements, size_t n, const Body* body, size_t geomId, float ft);

  virtual void SolveVelocities(Particles* particles, float dt);

  virtual Geometry* GetGeometry(){return _collider;};
  virtual const pxr::GfVec3f& GetContactPosition(size_t index) const {
    return _contacts[_p2c[index]].GetPointCoordinates();};
  virtual const pxr::GfVec3f& GetContactNormal(size_t index) const {
    return pxr::GfVec3f(0.f, 1.f, 0.f);};
  virtual float GetContactT(size_t index) const {
    return _contacts[_p2c[index]].GetT();};

  size_t GetNumContacts(){return _contacts.size();};
  Location& GetContact(size_t index){return _contacts[_p2c[index]];};
  const std::vector<int>& GetP2C(){return _p2c;};
  const std::vector<int>& GetC2P(){return _c2p;};

  virtual float GetValue(Particles* particles, size_t index) = 0;
  virtual pxr::GfVec3f GetGradient(Particles* particles, size_t index) = 0;

  inline bool CheckHit(size_t index) {
    return BIT_CHECK(_hits[index / sizeof(int)], index % sizeof(int));
  };
  inline void SetHit(size_t index, bool hit) {
    if(hit) BIT_SET(_hits[index / sizeof(int)], index % sizeof(int));
    else BIT_CLEAR(_hits[index / sizeof(int)], index % sizeof(int));
  };

protected:
  virtual void _Update(const pxr::UsdPrim& prim, double time) {};
  virtual void _UpdateParameters(const pxr::UsdPrim& prim, double time);
  virtual void _ResetContacts(Particles* particles);
  virtual void _BuildContacts(Particles* particles, const pxr::VtArray<Body*>& bodies,
    pxr::VtArray<Constraint*>& contacts, float dt);
  virtual void _FindContacts(size_t begin, size_t end, Particles* particles, float ft);
  
  virtual void _FindContact(size_t index, Particles* particles, float ft) = 0;
  virtual void _StoreContactLocation(Particles* particles, int elem, const Body* body, Location& location, float ft) = 0;

  virtual void _SolveVelocity(Particles* particles, size_t index, float dt) = 0;

  // hits encode vertex hit in the int list bits
  pxr::VtArray<int>           _hits;
  std::vector<int>            _p2c;
  std::vector<int>            _c2p;
  size_t                      _numContacts;
  std::vector<Location>       _contacts;
  float                       _restitution;
  float                       _friction;
  Geometry*                   _collider;
  pxr::TfToken                _key;

};

class PlaneCollision : public Collision
{
public:
  PlaneCollision(Geometry* collider, float restitution=0.5f, float friction= 0.5f);
  size_t GetTypeId() const override { return TYPE_ID; };

  float GetValue(Particles* particles, size_t index) override;
  pxr::GfVec3f GetGradient(Particles* particles, size_t index) override;
  void Update(const pxr::UsdPrim& prim, double time) override;

protected:
  void _UpdatePositionAndNormal();
  void _FindContact(size_t index, Particles* particles, float ft) override;
  void _StoreContactLocation(Particles* particles, int elem, const Body* body, Location& location, float ft) override;
  void _SolveVelocity(Particles* particles, size_t index, float dt) override;

private:
  static size_t                 TYPE_ID;
  pxr::GfVec3f                  _position;
  pxr::GfVec3f                  _normal;

};

class SphereCollision : public Collision
{
public:
  SphereCollision(Geometry* collider, float restitution=0.5f, float friction= 0.5f);
  size_t GetTypeId() const override { return TYPE_ID; };

  float GetValue(Particles* particles, size_t index) override;
  pxr::GfVec3f GetGradient(Particles* particles, size_t index) override;
  void Update(const pxr::UsdPrim& prim, double time) override;

protected:
  void _UpdateCenterAndRadius();
  void _FindContact(size_t index, Particles* particles, float ft) override;
  void _StoreContactLocation(Particles* particles, int elem, const Body* body, Location& location, float ft) override;
  void _SolveVelocity(Particles* particles, size_t index, float dt) override;
  

private:
  static size_t                 TYPE_ID;
  pxr::GfVec3f                  _center;
  float                         _radius;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_COLLISION_H
