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
  void AddBody(Particles* particles, Body* body);
  void RemoveBody(Particles* particles, Body* body);
  */

  virtual void FindContacts(Particles* particles, const pxr::VtArray<Body*>& bodies,
    pxr::VtArray<Constraint*>& constraints, float ft);
  virtual void FindContactsSerial(Particles* particles, const pxr::VtArray<Body*>& bodies,
    pxr::VtArray<Constraint*>& constraint, float ft);

  virtual void StoreContactsLocation(Particles* particles, int* elements, size_t n, const Body* body, size_t geomId, float ft);

  virtual void SolveVelocities(Particles* particles, float dt);

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

  const pxr::GfVec3f& GetContactNormal(size_t index) const {return _normal;};
  
  float GetValue(Particles* particles, size_t index) override;
  pxr::GfVec3f GetGradient(Particles* particles, size_t index) override;

protected:
  void _FindContact(size_t index, Particles* particles, float ft) override;
  void _StoreContactLocation(Particles* particles, int elem, const Body* body, Location& location, float ft) override;
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

  float GetValue(Particles* particles, size_t index) override;
  pxr::GfVec3f GetGradient(Particles* particles, size_t index) override;

protected:
  void _FindContact(size_t index, Particles* particles, float ft) override;
  void _StoreContactLocation(Particles* particles, int elem, const Body* body, Location& location, float ft) override;
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
