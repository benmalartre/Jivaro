#ifndef JVR_PBD_CONTACT_H
#define JVR_PBD_CONTACT_H

#include "../geometry/location.h"

JVR_NAMESPACE_OPEN_SCOPE

class Particles;
class Plane;
class Sphere;
class Mesh;
class Contact : public Location
{
public:
  void Init(Geometry* geometry, Particles* particles, size_t index);
  void Update(Geometry* geometry, Particles* particles, size_t index, float t);

  size_t GetParticleIndex() const {return _id;};
  pxr::GfVec3f GetNormal() const {return _normal;};
  pxr::GfVec3f GetVelocity() const {return _velocity;};
  pxr::GfQuatf GetTorque() const {return _torque;};
  float GetNormalVelocity() const { return _nrmV; };
  float GetDepth() const {return _d;};

private:
  size_t            _id;       // particle id
  pxr::GfVec3f      _velocity; // contact linear velocity
  pxr::GfQuatf      _torque;   // contact angular velocity
  pxr::GfVec3f      _normal;   // contact normal
  float             _d;        // penetration depth
  pxr::GfVec3f      _relV;     // relative velocity
  float             _nrmV;     // normal velocity
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_CONTACT_H
