#ifndef JVR_GEOMETRY_INSTANCER_H
#define JVR_GEOMETRY_INSTANCER_H

#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/quath.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/usd/usdGeom/pointInstancer.h>

#include "../geometry/deformable.h"


JVR_NAMESPACE_OPEN_SCOPE

class Instancer : public Points {
public:
  Instancer(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Instancer(const Deformable* other, bool normalize = true);
  Instancer(const pxr::UsdGeomPointInstancer& instancer, const pxr::GfMatrix4d& world);
  virtual ~Instancer() {};

  void Set(const pxr::VtArray<pxr::GfVec3f>&  positions, 
           const pxr::VtArray<int>*           protoIndices=nullptr,
           const pxr::VtArray<int64_t>*       indices=nullptr,
           const pxr::VtArray<pxr::GfVec3f>*  scales=nullptr,
           const pxr::VtArray<pxr::GfQuath>*  rotations=nullptr,
           const pxr::VtArray<pxr::GfVec3f>*  colors=nullptr);

private:
  pxr::VtArray<pxr::GfVec3f>      _scales;
  pxr::VtArray<int64_t>           _indices;
  pxr::VtArray<int>               _protoIndices;
  pxr::VtArray<pxr::GfQuath>      _rotations;
  pxr::VtArray<pxr::GfVec3f>      _colors;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_INSTANCER_H
