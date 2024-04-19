// Instancer
//----------------------------------------------
#include "../geometry/instancer.h"
#include "../geometry/utils.h"

#include <pxr/base/gf/ray.h>

JVR_NAMESPACE_OPEN_SCOPE

Instancer::Instancer(const pxr::GfMatrix4d& m)
  : Points(Geometry::INSTANCER, m)
{
  _haveNormals = false;
  _haveColors = false;
}

Instancer::Instancer(const Deformable* other)
  : Points(Geometry::INSTANCER)

  _positions = other->GetPositions();
  _previous = _positions;
  _haveRadius = deformable->HaveRadius();
  if (_haveRadius)_radius = deformable->GetRadius();
  _haveNormals = deformable->HaveNormals();
  if (_haveNormals)_normals = deformable->GetNormals();
  _haveColors = deformable->HaveColors();
  if (_haveColors)_colors = deformable->GetColors();
}

Instancer::Instancer(const pxr::UsdGeomPoints& points, const pxr::GfMatrix4d& world)
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

void Instancer::Set(
  const pxr::VtArray<pxr::GfVec3f>&  positions, 
  const pxr::VtArray<int>*           protoIndices=nullptr,
  const pxr::VtArray<int64_t>*       indices=nullptr,
  const pxr::VtArray<pxr::GfVec3f>*  scales=nullptr,
  const pxr::VtArray<pxr::GfQuath>*  rotations=nullptr,
  const pxr::VtArray<pxr::GfVec3f>*  colors=nullptr)
{
  const size_t n = positions.size();
  Deformable::_ValidateNumPoints(n);
  SetPositions(positions);
  
}



JVR_NAMESPACE_CLOSE_SCOPE