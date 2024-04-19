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

Points::Points(const Points* other, bool normalize)
  : Deformable(other, normalize)
{
  const Deformable* deformable = (const Deformable*)other;
  _positions = deformable->GetPositions();
  _previous = _positions;
  _haveRadius = deformable->HaveRadius();
  if (_haveRadius)_radius = deformable->GetRadius();
  _haveNormals = deformable->HaveNormals();
  if (_haveNormals)_normals = deformable->GetNormals();
  _haveColors = deformable->HaveColors();
  if (_haveColors)_colors = deformable->GetColors();
}

Points::Points(const Voxels* voxels)
  : Deformable(voxels, false)
{
  const Deformable* deformable = (const Deformable*)voxels;
  _positions = deformable->GetPositions();
  _previous = _positions;
  _haveRadius = deformable->HaveRadius();
  if(_haveRadius)_radius = deformable->GetRadius();
  _haveNormals = deformable->HaveNormals();
  if(_haveNormals)_normals = deformable->GetNormals();
  _haveColors = deformable->HaveColors();
  if(_haveColors)_colors = deformable->GetColors();
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