#ifndef JVR_GEOMETRY_INSTANCER_H
#define JVR_GEOMETRY_INSTANCER_H

#include <pxr/base/gf/ray.h>
#include <pxr/usd/usdGeom/primvarsApi.h>
#include <pxr/usd/usdGeom/pointInstancer.h>

#include "../geometry/deformable.h"


JVR_NAMESPACE_OPEN_SCOPE

class Instancer : public Deformable {
public:
  Instancer(const GfMatrix4d& xfo=GfMatrix4d(1.0));
  Instancer(const UsdPrim& instancer, const GfMatrix4d& world);
  virtual ~Instancer() {};

  void Set(const VtArray<GfVec3f>&  positions, 
           const VtArray<int>*           protoIndices=nullptr,
           const VtArray<int64_t>*       indices=nullptr,
           const VtArray<GfVec3f>*  scales=nullptr,
           const VtArray<GfQuath>*  rotations=nullptr,
           const VtArray<GfVec3f>*  colors=nullptr);

  bool HaveIndices(){return _indices.size() > 0 && _positions.size() == _indices.size();};
  const VtArray<int>& GetProtoIndices() const {return _protoIndices;};
  const VtArray<int64_t>& GetIndices() const {return _indices;};
  const VtArray<GfVec3f>& GetScales() const {return _scales;};
  const VtArray<GfQuath>& GetRotations() const {return _rotations;};
  const VtArray<SdfPath>& GetPrototypes() const {return _prototypes;};

  void AddPrototype(SdfPath& path);
  void RemovePrototype(SdfPath& path);

protected:
  void _Inject(const GfMatrix4d& parent,
    const UsdTimeCode& code=UsdTimeCode::Default()) override;

private:
  VtArray<GfVec3f>      _scales;
  VtArray<int64_t>           _indices;
  VtArray<int>               _protoIndices;
  VtArray<GfQuath>      _rotations;
  VtArray<SdfPath>      _prototypes;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_INSTANCER_H
