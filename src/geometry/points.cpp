// Points
//----------------------------------------------
#include "../geometry/points.h"
#include "../geometry/utils.h"
#include "../geometry/voxels.h"

#include <pxr/base/gf/ray.h>

JVR_NAMESPACE_OPEN_SCOPE

Points::Points(const pxr::GfMatrix4d& m)
  : Deformable(Geometry::POINT, m)
{
}

Points::Points(const pxr::UsdPrim& prim, const pxr::GfMatrix4d& world)
  : Deformable(prim, world)
{
}


JVR_NAMESPACE_CLOSE_SCOPE