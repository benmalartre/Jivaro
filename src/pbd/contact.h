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
  void Init(Collision* collision, Particles* particles, size_t index, size_t geomId);
  void Update(Collision* collision, Particles* particles, size_t index);

  pxr::GfVec3f GetNormal() const {return _normal;};
  float GetNormalVelocity() const { return _nrmV; };
  float GetDepth() const {return _d;};

private:
  pxr::GfVec3f      _normal;   // contact normal
  float             _d;        // penetration depth
  pxr::GfVec3f      _relV;     // relative velocity
  float             _nrmV;     // normal velocity
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_CONTACT_H
