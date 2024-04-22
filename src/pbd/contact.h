#ifndef JVR_PBD_CONTACT_H
#define JVR_PBD_CONTACT_H

#include "../geometry/location.h"

JVR_NAMESPACE_OPEN_SCOPE

class Particles;
class Constraint;
class CollisionConstraint;
class Contact : public Location
{
public:
  void Init(Particles* particles, const pxr::GfVec3f* positions);
  void Update(Particles* particles, const pxr::GfVec3f* positions);

  float GetNormalVelocity() const { return _nrmV; };

private:
  pxr::GfVec3f      _normal;   // contact normal
  float             _d;        // penetration depth
  pxr::GfVec3f      _relV;     // relative velocity
  float             _nrmV;     // normal velocity
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_CONTACT_H
