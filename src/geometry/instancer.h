#ifndef JVR_GEOMETRY_INSTANCER_H
#define JVR_GEOMETRY_INSTANCER_H

#include <pxr/usd/usdGeom/pointInstancer.h>


#include "../geometry/deformable.h"


JVR_NAMESPACE_OPEN_SCOPE

class Instancer : public Deformable {
public:
  Instancer(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Instancer(const pxr::UsdPrim& instancer, const pxr::GfMatrix4d& world);
  virtual ~Instancer() {};

  void Set(const pxr::VtArray<pxr::GfVec3f>&  positions, 
           const pxr::VtArray<int>*           protoIndices=nullptr,
           const pxr::VtArray<int64_t>*       indices=nullptr,
           const pxr::VtArray<pxr::GfVec3f>*  scales=nullptr,
           const pxr::VtArray<pxr::GfQuath>*  rotations=nullptr,
           const pxr::VtArray<pxr::GfVec3f>*  colors=nullptr);

  bool HaveIndices(){return _indices.size() > 0 && _positions.size() == _indices.size();};
  const pxr::VtArray<int>& GetProtoIndices() const {return _protoIndices;};
  const pxr::VtArray<int64_t>& GetIndices() const {return _indices;};
  const pxr::VtArray<pxr::GfVec3f>& GetScales() const {return _scales;};
  const pxr::VtArray<pxr::GfQuath>& GetRotations() const {return _rotations;};

private:
  pxr::VtArray<pxr::GfVec3f>      _scales;
  pxr::VtArray<int64_t>           _indices;
  pxr::VtArray<int>               _protoIndices;
  pxr::VtArray<pxr::GfQuath>      _rotations;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_INSTANCER_H
