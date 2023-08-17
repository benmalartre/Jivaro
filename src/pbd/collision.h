#ifndef JVR_PBD_COLLISION_H
#define JVR_PBD_COLLISION_H

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>

#include "../common.h"
#include "../pbd/mask.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

struct Particles;
struct Body;

class Collision : public Mask
{
public:
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

  virtual void FindContactsSerial(Particles* particles);
  virtual void ResolveContactsSerial(Particles* particles, const float dt);

  inline bool CheckHit(size_t index) {
    return BITMASK_CHECK(_hits[index / sizeof(int)], index % sizeof(int));
  };
  inline void SetHit(size_t index) {
    BITMASK_SET(_hits[index / sizeof(int)], index % sizeof(int));
  };
  //virtual void Apply();

protected:
  virtual void _ResetContacts(Particles* particles);
  virtual void _BuildContacts(Particles* particles);
  virtual void _FindContacts(size_t begin, size_t end, Particles* particles) = 0;
  virtual void _ResolveContacts(size_t begin, size_t end, Particles* particles, const float dt) = 0;
  //pxr::VtArray<int>           _mask;        // mask is in the list of active point indices
  //pxr::VtArray<float>         _weights;     // if mask weights size must equals mask size 
                                              // else weights size must equals num particles

private:
  pxr::VtArray<int>           _hits;        // hits encode vertex hit in the int list bits
  pxr::VtArray<int>           _contacts;    // flat list of contact vertex
};

class PlaneCollision : public Collision
{
public:
  PlaneCollision();
  PlaneCollision(const pxr::GfVec3f& normal, const pxr::GfVec3f& position = pxr::GfVec3f(0.f));

protected:
  void _FindContacts(size_t begin, size_t end, Particles* particles) override;
  void _ResolveContacts(size_t begin, size_t end, Particles* particles, const float dt) override;

private:
  pxr::GfVec3f _position;
  pxr::GfVec3f _normal;
  float        _distance;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_COLLISION_H
