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

class Collision : public Mask
{
public:
  struct _Hit {
    pxr::GfVec3f delta;
    pxr::GfVec3f normal;
  };
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

  size_t GetNumContacts() { return _contacts.size(); };
  const pxr::VtArray<int>& GetContacts() const { return _contacts; };
  pxr::VtArray<int>& GetContacts() { return _contacts; };

  virtual void FindContacts(Particles* particles);
  virtual void ResolveContacts(Particles* particles, const float dt);
  virtual void UpdateVelocity(Particles* particles, const float dt);

  virtual void FindContactsSerial(Particles* particles);
  virtual void ResolveContactsSerial(Particles* particles, const float dt);
  virtual void UpdateVelocitySerial(Particles* particles, const float dt);

  inline bool CheckHit(size_t index) {
    return BIT_CHECK(_hits[index / sizeof(int)], index % sizeof(int));
  };
  inline void SetHit(size_t index) {
    BIT_SET(_hits[index / sizeof(int)], index % sizeof(int));
  };
  //virtual void Apply();

protected:
  virtual void _ResetContacts(Particles* particles);
  virtual void _BuildContacts(Particles* particles);
  virtual void _FindContacts(size_t begin, size_t end, Particles* particles);
  virtual void _ResolveContacts(size_t begin, size_t end, Particles* particles, const float dt);
  
  virtual void _FindContact(size_t index, Particles* particles) = 0;
  virtual void _ResolveContact(size_t index, Particles* particles, const float dt, _Hit* hit) = 0;

private:
  pxr::VtArray<int>           _hits;        // hits encode vertex hit in the int list bits
  pxr::VtArray<int>           _contacts;    // flat list of contact vertex
  pxr::VtArray<pxr::GfVec3f>  _deltas;
  pxr::VtArray<pxr::GfVec3f>  _normals;

  float                       _restitution;
  float                       _friction;
};

class PlaneCollision : public Collision
{
public:
  PlaneCollision();
  PlaneCollision(const pxr::GfVec3f& normal, const pxr::GfVec3f& position = pxr::GfVec3f(0.f));

  inline void Set(const pxr::GfVec3f& position, const pxr::GfVec3f& normal);
  inline void SetPosition(const pxr::GfVec3f& position);
  inline void SetNormal(const pxr::GfVec3f& normal);
  inline void SetDistance(const float distance);

protected:
  void _FindContact(size_t index, Particles* particles) override;
  void _ResolveContact(size_t index, Particles* particles, const float dt, _Hit* hit) override;

private:
  pxr::GfVec3f _position;
  pxr::GfVec3f _normal;
  float        _distance;
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

void PlaneCollision::SetDistance(const float distance) 
{ 
  _distance = distance; 
};


class SphereCollision : public Collision
{
public:
  SphereCollision();
  SphereCollision(const pxr::GfMatrix4f& xform, float radius);

  inline void Set(const pxr::GfMatrix4f& xform, const float radius);
  inline void SetXform(const pxr::GfMatrix4f& xform);
  inline void SetRadius(const float radius);

protected:
  void _FindContact(size_t index, Particles* particles) override;
  void _ResolveContact(size_t index, Particles* particles, const float dt, _Hit* hit) override;

private:
  pxr::GfMatrix4f _xform;
  pxr::GfMatrix4f _invXform;
  float           _radius;
};

void SphereCollision::Set(const pxr::GfMatrix4f& xform, const float radius) 
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

void SphereCollision::SetRadius(const float radius) 
{ 
  _radius = radius; 
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_COLLISION_H
