#include "../geometry/instancer.h"
#include "../geometry/utils.h"


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

  void Instancer::AddPrototype(pxr::SdfPath& path)
  {
    for(auto& proto: _prototypes)
      if(path == proto)return;
    _prototypes.push_back(path);
  }

  void Instancer::RemoveProptotype(pxr::SdfPath& path)
  {
    _prototypes.erase(
      std::find(_prototypes.begin(), _prototypes.end(), path)
    );
  }

void Instancer::_Inject(pxr::UsdPrim& prim, const pxr::GfMatrix4d& parent, const pxr::UsdTimeCode& time)
{
  pxr::UsdGeomPointInstancer instancer(prim);
  
  instancer.CreatePositionsAttr().Set(_positions, time);
  instancer.CreateScalesAttr().Set(_scales, time);
  instancer.CreateOrientationsAttr().Set(_rotations, time);
  instancer.CreateProtoIndicesAttr().Set(_protoIndices, time);
  for(size_t p = 0; p < _prototypes.size(); ++p)
    instancer.CreatePrototypesRel().AddTarget(_prototypes[p]);

  if(_indices.size() == _positions.size())
    instancer.CreateIdsAttr().Set(_indices, time);

  //if(_haveColors)
  pxr::UsdGeomPrimvarsAPI primvarsApi(instancer);
  pxr::UsdGeomPrimvar colorPrimvar =
    primvarsApi.CreatePrimvar(pxr::UsdGeomTokens->primvarsDisplayColor, pxr::SdfValueTypeNames->Color3fArray);
  colorPrimvar.SetInterpolation(pxr::UsdGeomTokens->varying);
  colorPrimvar.SetElementSize(1);
  colorPrimvar.Set(_colors, time);

}

JVR_NAMESPACE_CLOSE_SCOPE