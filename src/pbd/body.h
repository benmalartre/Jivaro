#ifndef JVR_PBD_BODY_H
#define JVR_PBD_BODY_H

#include <pxr/base/gf/bbox3d.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

namespace PBD {
  
  struct Body
  {
    float          damping;
    float          radius;
    float          mass;

    size_t         offset;
    size_t         numPoints;
    
    Geometry*      geometry;
  };
} // namespace PBD

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_BODY_H
