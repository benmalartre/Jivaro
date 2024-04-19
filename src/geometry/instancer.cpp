// Instancer
//----------------------------------------------
#include "../geometry/instancer.h"
#include "../geometry/utils.h"

#include <pxr/base/gf/ray.h>

JVR_NAMESPACE_OPEN_SCOPE

Instancer::Instancer(const pxr::GfMatrix4d& m)
  : Deformable(Geometry::INSTANCER, m)
{
  _haveNormals = false;
  _haveColors = false;
}

Instancer::Instancer(const pxr::UsdPrim& prim, const pxr::GfMatrix4d& world)
  : Deformable(prim, world)
{
  if (prim.IsA<pxr::UsdGeomPointInstancer>()) {
    pxr::UsdGeomPointInstancer instancer(prim);
    const size_t numInstances = _positions.size();

    pxr::UsdAttribute protoIndicesAttr = instancer.GetProtoIndicesAttr();
    protoIndicesAttr.Get(&_protoIndices, pxr::UsdTimeCode::Default());

    pxr::UsdAttribute idsAttr = instancer.GetIdsAttr();
    if (idsAttr.IsDefined() && idsAttr.HasAuthoredValue())
      idsAttr.Get(&_indices, pxr::UsdTimeCode::Default());

    pxr::UsdAttribute scalesAttr = instancer.GetScalesAttr();
    if (scalesAttr.IsDefined() && scalesAttr.HasAuthoredValue())
      scalesAttr.Get(&_scales, pxr::UsdTimeCode::Default());
    else _scales.resize(numInstances, pxr::GfVec3f(1.f));

    pxr::UsdAttribute orientationsAttr = instancer.GetOrientationsAttr();
    if (orientationsAttr.IsDefined() && orientationsAttr.HasAuthoredValue())
      orientationsAttr.Get(&_rotations, pxr::UsdTimeCode::Default());
    else _scales.resize(numInstances, pxr::GfVec3f(1.f));

  }
}

void Instancer::Set(
  const pxr::VtArray<pxr::GfVec3f>&  positions, 
  const pxr::VtArray<int>*           protoIndices,
  const pxr::VtArray<int64_t>*       indices,
  const pxr::VtArray<pxr::GfVec3f>*  scales,
  const pxr::VtArray<pxr::GfQuath>*  rotations,
  const pxr::VtArray<pxr::GfVec3f>*  colors)
{
  const size_t n = positions.size();
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
    for(size_t c = 0; c << _colors.size(); ++c)
      _colors[c] = pxr::GfVec3f(RANDOM_0_1,RANDOM_0_1,RANDOM_0_1);
  }

}

JVR_NAMESPACE_CLOSE_SCOPE