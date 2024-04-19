// Points
//----------------------------------------------
#include "../geometry/points.h"
#include "../geometry/utils.h"
#include "../geometry/voxels.h"

#include <pxr/base/gf/ray.h>

JVR_NAMESPACE_OPEN_SCOPE

Points::Points(short type, const pxr::GfMatrix4d& m)
  : Deformable(type, m)
{
}

Points::Points(const pxr::UsdPrim& prim, const pxr::GfMatrix4d& world)
  : Deformable(prim, world)
{
}


JVR_NAMESPACE_CLOSE_SCOPE