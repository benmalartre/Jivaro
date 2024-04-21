#ifndef JVR_PBD_CONTACT_H
#define JVR_PBD_CONTACT_H

#include "../geometry/location.h"

JVR_NAMESPACE_OPEN_SCOPE

class Contact : public Location
{

private:
  pxr::GfVec3f      _normal;   // contact normal
  float             _d;        // penetration depth
  pxr::GfVec3f      _vrel;     // relative velocity
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_CONTACT_H
