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

  virtual void FindContacts(size_t begin, size_t end, Particles* particles, const float dt) = 0;
  virtual void ResolveContacts() = 0;
  //virtual void Apply();

protected:
  //pxr::VtArray<int>           _mask;        // mask is in the list of active point indices
  //pxr::VtArray<float>         _weights;     // if mask weights size must equals mask size 
                                            // else weights size must equals num particles
  
  pxr::VtArray<int>           _hits;        // hits encode vertex hit in the int list bits
  pxr::VtArray<pxr::GfVec3f>  _contacts[2]; // double buffer contacts (write/read)
  bool                        _flop;
};

class PlaneCollision : public Collision
{
public:
  PlaneCollision();
  PlaneCollision(const pxr::GfVec3f& normal, const pxr::GfVec3f& position = pxr::GfVec3f(0.f));

  void FindContacts(size_t begin, size_t end, Particles* particles, const float dt) override;
  void ResolveContacts() override;

private:
  pxr::GfVec3f _position;
  pxr::GfVec3f _normal;
  float        _distance;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_COLLISION_H
