#include "../geometry/instancer.h"
#include "../geometry/utils.h"


JVR_NAMESPACE_OPEN_SCOPE

Instancer::Instancer(const GfMatrix4d& m)
  : Deformable(Geometry::INSTANCER, m)
{
  _haveNormals = false;
  _haveColors = false;
}

Instancer::Instancer(const UsdPrim& prim, const GfMatrix4d& world)
  : Deformable(prim, world)
{
  if (prim.IsA<UsdGeomPointInstancer>()) {
    UsdGeomPointInstancer instancer(prim);
    const size_t numInstances = _positions.size();

    UsdAttribute protoIndicesAttr = instancer.GetProtoIndicesAttr();
    protoIndicesAttr.Get(&_protoIndices, UsdTimeCode::Default());

    UsdAttribute idsAttr = instancer.GetIdsAttr();
    if (idsAttr.IsDefined() && idsAttr.HasAuthoredValue())
      idsAttr.Get(&_indices, UsdTimeCode::Default());

    UsdAttribute scalesAttr = instancer.GetScalesAttr();
    if (scalesAttr.IsDefined() && scalesAttr.HasAuthoredValue())
      scalesAttr.Get(&_scales, UsdTimeCode::Default());
    else _scales.resize(numInstances, GfVec3f(1.f));

    UsdAttribute orientationsAttr = instancer.GetOrientationsAttr();
    if (orientationsAttr.IsDefined() && orientationsAttr.HasAuthoredValue())
      orientationsAttr.Get(&_rotations, UsdTimeCode::Default());
    else _scales.resize(numInstances, GfVec3f(1.f));

  }
}

void Instancer::Set(
  const VtArray<GfVec3f>&  positions, 
  const VtArray<int>*           protoIndices,
  const VtArray<int64_t>*       indices,
  const VtArray<GfVec3f>*  scales,
  const VtArray<GfQuath>*  rotations,
  const VtArray<GfVec3f>*  colors)
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
    _scales.resize(n, GfVec3f(1.f));
  }

  if(rotations && rotations->size() == n) {
    _rotations = *rotations;
  } else {
    _rotations.resize(n, GfQuath(1.f));
  }

  if(colors && colors->size() == n) {
    _colors = *colors;
  } else {
    _colors.resize(n);
    for(size_t c = 0; c << _colors.size(); ++c)
      _colors[c] = GfVec3f(RANDOM_0_1,RANDOM_0_1,RANDOM_0_1);
  }
}

  void Instancer::AddPrototype(SdfPath& path)
  {
    for(auto& proto: _prototypes)
      if(path == proto)return;
    _prototypes.push_back(path);
  }

  void Instancer::RemovePrototype(SdfPath& path)
  {
    _prototypes.erase(
      std::find(_prototypes.begin(), _prototypes.end(), path)
    );
  }

void Instancer::_Inject(const GfMatrix4d& parent, const UsdTimeCode& time)
{
  UsdGeomPointInstancer instancer(_prim);
  
  instancer.CreatePositionsAttr().Set(_positions, time);
  instancer.CreateScalesAttr().Set(_scales, time);
  instancer.CreateOrientationsAttr().Set(_rotations, time);
  instancer.CreateProtoIndicesAttr().Set(_protoIndices, time);

  std::cout << "INJECT INSTANCER PROTO INDICES : " << _protoIndices.size() << std::endl;
  for(size_t p = 0; p < _prototypes.size(); ++p)
    instancer.CreatePrototypesRel().AddTarget(_prototypes[p]);

  if(_indices.size() == _positions.size())
    instancer.CreateIdsAttr().Set(_indices, time);

  //if(_haveColors)
  UsdGeomPrimvarsAPI primvarsApi(instancer);
  UsdGeomPrimvar colorPrimvar =
    primvarsApi.CreatePrimvar(UsdGeomTokens->primvarsDisplayColor, SdfValueTypeNames->Color3fArray);
  colorPrimvar.SetInterpolation(UsdGeomTokens->varying);
  colorPrimvar.SetElementSize(1);
  colorPrimvar.Set(_colors, time);

}

JVR_NAMESPACE_CLOSE_SCOPE