// Points
//----------------------------------------------
#include "../geometry/points.h"
#include "../geometry/utils.h"
#include "../geometry/voxels.h"

#include <pxr/base/gf/ray.h>

JVR_NAMESPACE_OPEN_SCOPE

Points::Points()
  : Deformable(Geometry::POINT, pxr::GfMatrix4d(1.0))
{
}

Points::Points(const Points* other, bool normalize)
  : Deformable(other, normalize)
{
  size_t numPoints = _positions.size();
  _positions = other->_positions;
  _previous = _positions;
  _haveRadius = other->HaveRadius();
  if (_haveRadius)_radius = other->GetRadius();
  _haveNormals = other->HaveNormals();
  if (_haveNormals)_normals = other->GetNormals();
  _haveColors = other->HaveColors();
  if (_haveColors)_colors = other->GetColors();
}

Points::Points(const Voxels* voxels)
  : Deformable(other, false)
{
  _positions = voxels->GetPositions();
  _previous = _positions;
  _haveRadius = voxels->HaveRadius();
  if(_haveRadius)_radius = voxels->GetRadius();
  _haveNormals = voxels->HaveNormals();
  if(_haveNormals)_normals = voxels->GetNormals();
  _haveColors = voxels->HaveColors();
  if(_haveColors)_colors = voxels->GetColors();
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

Points::Points(Voxels)


JVR_NAMESPACE_CLOSE_SCOPE