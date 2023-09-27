#include <pxr/base/gf/ray.h>

#include "../geometry/geometry.h"
#include "../geometry/point.h"
#include "../geometry/edge.h"
#include "../geometry/triangle.h"
#include "../geometry/utils.h"


JVR_NAMESPACE_OPEN_SCOPE

Geometry::Geometry()
{
  _initialized = false;
  _type = INVALID;
  _wirecolor = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  _matrix = pxr::GfMatrix4d(1.0);
}

Geometry::Geometry(short type, const pxr::GfMatrix4d& world)
{
  _initialized = false;
  _type = type;
  _wirecolor = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  _matrix = world;
}

Geometry::Geometry(const Geometry* other, short type, bool normalize)
{
  _initialized = true;
  _type = type;
  _wirecolor = other->_wirecolor;

  _bbox = other->_bbox;
  _matrix = other->_matrix;
}

JVR_NAMESPACE_CLOSE_SCOPE