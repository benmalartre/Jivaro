// Points
//----------------------------------------------
#include "../geometry/points.h"
#include "../geometry/utils.h"
#include <pxr/base/gf/ray.h>

JVR_NAMESPACE_OPEN_SCOPE

Points::Points()
  : Deformable(Geometry::POINT, pxr::GfMatrix4d(1.0))
{
}

Points::Points(short type, const pxr::GfMatrix4d& matrix)
  : Deformable(type, matrix)
{
}

Points::Points(const Points* other, bool normalize)
  : Deformable(other, normalize)
{
  size_t numPoints = _positions.size();
  _positions = other->_positions;
  _normals = other->_normals;
  _radius = other->_radius;
  _colors = other->_colors;
}

Points::Points(const pxr::UsdGeomPoints& points, const pxr::GfMatrix4d& world)
  : Deformable(Geometry::POINT, world)
{
  pxr::UsdAttribute pointsAttr = points.GetPointsAttr();
  pointsAttr.Get(&_positions, pxr::UsdTimeCode::Default());

  pxr::UsdAttribute normalsAttr = points.GetNormalsAttr();
  if (normalsAttr.IsDefined() && normalsAttr.HasAuthoredValue())
    normalsAttr.Get(&_normals, pxr::UsdTimeCode::Default());

  pxr::UsdAttribute widthsAttr = points.GetWidthsAttr();
  if (widthsAttr.IsDefined() && widthsAttr.HasAuthoredValue())
    widthsAttr.Get(&_radius, pxr::UsdTimeCode::Default());
}

JVR_NAMESPACE_CLOSE_SCOPE