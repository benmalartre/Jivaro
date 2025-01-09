// Points
//----------------------------------------------
#include "../geometry/points.h"
#include "../geometry/utils.h"
#include "../geometry/voxels.h"

#include <pxr/base/gf/ray.h>

JVR_NAMESPACE_OPEN_SCOPE

Points::Points(const GfMatrix4d& m)
  : Deformable(Geometry::POINT, m)
{
}

Points::Points(const UsdPrim& prim, const GfMatrix4d& world)
  : Deformable(prim, world)
{
}

Geometry::DirtyState 
Points::_Sync(const GfMatrix4d& matrix, const UsdTimeCode& time)
{
  if(_prim.IsValid() && _prim.IsA<UsdGeomPoints>())
  {
    _previous = _positions;
    UsdGeomPoints usdPoints(_prim);
    const size_t nbPositions = _positions.size();
    usdPoints.GetPointsAttr().Get(&_positions, time);
  }
  return Geometry::DirtyState::DEFORM;
}

void 
Points::_Inject(const GfMatrix4d& parent,
    const UsdTimeCode& time)
{
  if(_prim.IsA<UsdGeomPoints>()) {
    UsdGeomPoints usdPoints(_prim);
    usdPoints.CreatePointsAttr().Set(GetPositions(), time);
    usdPoints.CreateWidthsAttr().Set(GetWidths(), time);
    usdPoints.SetWidthsInterpolation(UsdGeomTokens->varying);
  }
}

JVR_NAMESPACE_CLOSE_SCOPE