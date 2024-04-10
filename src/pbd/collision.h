#ifndef JVR_PBD_COLLISION_H
#define JVR_PBD_COLLISION_H

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4f.h>

#include "../common.h"
#include "../geometry/intersection.h"
#include "../pbd/mask.h"
#include "../pbd/contact.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

struct Particles;
struct Body;
class Constraint;

class Collision : public Mask
{
public:
  Collision(float restitution=0.f, float friction=0.f) 
    : _restitution(restitution)
    , _friction(friction) {};
/*
  bool HasMask() const { return _mask.size() > 0; };
  void SetMask(const pxr::VtArray<int>& mask) { _mask = mask; };
  void RemoveMask() { _mask.clear(); }
  size_t MaskSize() const { return _mask.size(); };
  const int* GetMaskCPtr() const { return &_mask[0]; };
  bool HasWeights() const { return _weights.size() > 0; };
  void SetWeights(const pxr::VtArray<float>& weights) { _weights = weights; };
  void RemoveWeights() { _weights.clear(); };

  bool Affects(size_t index) const;

  void AddBody(Particles* particles, Body* body);
  void RemoveBody(Particles* particles, Body* body);
  */

  virtual void FindContacts(Particles* particles, const pxr::VtArray<Body*>& bodies,
    pxr::VtArray<Constraint*>& constraints, float dt);
  virtual void FindContactsSerial(Particles* particles, const pxr::VtArray<Body*>& bodies,
    pxr::VtArray<Constraint*>& constraint, float dt);

  virtual void StoreContactsLocation(Particles* particles, int* elements, size_t n, const Body* body, size_t geomId, float dt);
  virtual void UpdateContactsLocation(float it);

  virtual void SolveVelocities(Particles* particles, float dt);
  virtual void SolveContactResponses(Particles* particles, float dt);

  virtual const pxr::GfVec3f& GetContactPosition(size_t index) const {
    return _contacts[_p2c[index]].GetPosition();};
  virtual const pxr::GfVec3f& GetContactNormal(size_t index) const {
    return _contacts[_p2c[index]].GetNormal();};
  virtual float GetContactT(size_t index) const {
    return _contacts[_p2c[index]].GetT();};
  virtual const pxr::GfVec3f& GetContactResponse(size_t index) const {
    return _contacts[_p2c[index]].GetResponse();};

  size_t GetNumContacts(){return _contacts.size();};
  Contact& GetContact(size_t index){return _contacts[_p2c[index]];};
  const std::vector<int>& GetP2C(){return _p2c;};
  const std::vector<int>& GetC2P(){return _c2p;};

  inline bool CheckHit(size_t index) {
    return BIT_CHECK(_hits[index / sizeof(int)], index % sizeof(int));
  };
  inline void SetHit(size_t index, bool hit) {
    if(hit) BIT_SET(_hits[index / sizeof(int)], index % sizeof(int));
    else BIT_CLEAR(_hits[index / sizeof(int)], index % sizeof(int));
  };

protected:
  virtual void _ResetContacts(Particles* particles);
  virtual void _BuildContacts(Particles* particles, const pxr::VtArray<Body*>& bodies,
    pxr::VtArray<Constraint*>& contacts, float dt);
  virtual void _FindContacts(size_t begin, size_t end, Particles* particles, float dt);
  
  virtual void _FindContact(size_t index, Particles* particles, float dt) = 0;
  virtual void _StoreContactLocation(Particles* particles, int elem, const Body* body, Contact& location, float dt) = 0;
  virtual void _UpdateContactLocation(Contact& location, float it) = 0;
  virtual void _SolveVelocity(Particles* particles, size_t index, float dt) = 0;

  // hits encode vertex hit in the int list bits
  pxr::VtArray<int>           _hits;
  std::vector<int>            _p2c;
  std::vector<int>            _c2p;
  size_t                      _numContacts;
  std::vector<Contact>        _contacts;
  float                       _restitution;
  float                       _friction;
};

class PlaneCollision : public Collision
{
public:
  PlaneCollision(const float restitution=0.5f, const float friction= 0.5f,
    const pxr::GfVec3f& normal=pxr::GfVec3f(0.f, 1.f, 0.f), 
    const pxr::GfVec3f& position = pxr::GfVec3f(0.f));

  inline void Set(const pxr::GfVec3f& position, const pxr::GfVec3f& normal);
  inline void SetPosition(const pxr::GfVec3f& position);
  inline void SetNormal(const pxr::GfVec3f& normal);

protected:
  void _FindContact(size_t index, Particles* particles, float dt) override;
  void _StoreContactLocation(Particles* particles, int elem, const Body* body, Contact& location, float dt) override;
  void _UpdateContactLocation(Contact& location, float it) override;
  void _SolveVelocity(Particles* particles, size_t index, float dt) override;

private:
  pxr::GfVec3f                 _position;
  pxr::GfVec3f                 _normal;
};

void PlaneCollision::Set(const pxr::GfVec3f& position, const pxr::GfVec3f& normal) 
{ 
  _position = position; 
  _normal = normal;
};

void PlaneCollision::SetPosition(const pxr::GfVec3f& position) 
{ 
  _position = position; 
};

void PlaneCollision::SetNormal(const pxr::GfVec3f& normal) 
{ 
  _normal = normal; 
};

class SphereCollision : public Collision
{
public:
  SphereCollision(const float restitution=0.5f, const float friction= 0.5f,
    const pxr::GfMatrix4f& xform=pxr::GfMatrix4f(1.f), float radius=1.f);

  inline void Set(const pxr::GfMatrix4f& xform, float radius);
  inline void SetXform(const pxr::GfMatrix4f& xform);
  inline void SetRadius(float radius);

protected:
  void _FindContact(size_t index, Particles* particles, float dt) override;
  void _StoreContactLocation(Particles* particles, int elem, const Body* body, Contact& location, float dt) override;
  void _UpdateContactLocation(Contact& location, float it) override;
  void _SolveVelocity(Particles* particles, size_t index, float dt) override;

private:
  pxr::GfMatrix4f             _xform;
  pxr::GfMatrix4f             _invXform;
  float                       _radius;
};

void SphereCollision::Set(const pxr::GfMatrix4f& xform, float radius) 
{ 
  _xform = xform;
  _invXform = xform.GetInverse(); 
  _radius = radius; 
};

void SphereCollision::SetXform(const pxr::GfMatrix4f& xform)
{
  _xform = xform;
  _invXform = xform.GetInverse();
};

void SphereCollision::SetRadius(float radius) 
{ 
  _radius = radius; 
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_COLLISION_H
