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
  if(protoIndices && protoIndices->size() == n) {
    _protoIndices = *protoIndices;
  } else {
    _protoIndices.resize(n, 0);
  }
  
  if(indices && indices->size() == n) {
    _indices = *indices;
  }

  if(scales && scales->size() == n) {
    _scales = *scales;
  } else {
    _scales.resize(n, pxr::GfVec3f(1.f));
  }

  if(rotations && rotations->size() == n) {
    _rotations = *rotations;
  } else {
    _rotations.resize(n, pxr::GfQuath(1.f));
  }

  if(colors && colors->size() == n) {
    _colors = *colors;
  } else {
    _colors.resize(n);
    for(auto& color: _colors)
      color = pxr::GfVEc3f(RANDOM_0_1,RANDOM_0_1,RANDOM_0_1);
  }

}

JVR_NAMESPACE_CLOSE_SCOPE