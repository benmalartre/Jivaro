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

Geometry::DirtyState 
Points::_Sync(const pxr::GfMatrix4d& matrix, const pxr::UsdTimeCode& time)
{
  if(_prim.IsValid() && _prim.IsA<pxr::UsdGeomPoints>())
  {
    _previous = _positions;
    pxr::UsdGeomPoints usdPoints(_prim);
    const size_t nbPositions = _positions.size();
    usdPoints.GetPointsAttr().Get(&_positions, time);
  }
  return Geometry::DirtyState::DEFORM;
}

void 
Points::_Inject(const pxr::GfMatrix4d& parent,
    const pxr::UsdTimeCode& time)
{
  if(_prim.IsA<pxr::UsdGeomPoints>()) {
    pxr::UsdGeomPoints usdPoints(_prim);
    usdPoints.CreatePointsAttr().Set(GetPositions(), time);
    usdPoints.CreateWidthsAttr().Set(GetWidths(), time);
    usdPoints.SetWidthsInterpolation(pxr::UsdGeomTokens->varying);
  }
}

JVR_NAMESPACE_CLOSE_SCOPE