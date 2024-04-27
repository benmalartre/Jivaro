#ifndef JVR_PBD_CONTACT_H
#define JVR_PBD_CONTACT_H

#include "../geometry/location.h"

JVR_NAMESPACE_OPEN_SCOPE

class Particles;
class Plane;
class Sphere;
class Mesh;
class Collision;
class Contact : public Location
{
public:
  void Init(Collision* collision, Particles* particles, size_t index, size_t other);
  void Update(Collision* collision, Particles* particles, size_t index);

  const pxr::GfVec3f& GetNormal() const {return _normal;};
  const pxr::GfVec3f& GetVelocity() const {return _velocity;};
  float GetSpeed() const { return _speed; };
  float GetDepth() const {return _d;};

private:
  pxr::GfVec3f      _normal;   // contact normal
  pxr::GfVec3f      _velocity; // relative velocity

  float             _d;        // penetration depth
  float             _speed;    // normal speed
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_CONTACT_H
