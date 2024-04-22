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
  void Update(Geometry* geometry, Particles* particles, size_t index);

  pxr::GfVec3f GetNormal() const {return _normal;};
  float GetNormalVelocity() const { return _nrmV; };
  float GetPenetrationDepth() const {return _d;};

private:
  pxr::GfVec3f      _normal;   // contact normal
  float             _d;        // penetration depth
  pxr::GfVec3f      _relV;     // relative velocity
  float             _nrmV;     // normal velocity
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_CONTACT_H
