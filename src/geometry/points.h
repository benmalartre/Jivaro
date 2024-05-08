#ifndef JVR_GEOMETRY_POINTS_H
#define JVR_GEOMETRY_POINTS_H

#include <pxr/usd/usdGeom/points.h>

#include "../geometry/deformable.h"


JVR_NAMESPACE_OPEN_SCOPE
class Voxels;
class Points : public Deformable {
public:
  Points(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Points(const pxr::UsdPrim& prim, const pxr::GfMatrix4d& world);
  virtual ~Points() {};

protected:
  DirtyState _Sync(const pxr::GfMatrix4d& matrix, 
    const pxr::UsdTimeCode& code=pxr::UsdTimeCode::Default()) override;
  void _Inject(const pxr::GfMatrix4d& parent,
    const pxr::UsdTimeCode& code=pxr::UsdTimeCode::Default()) override;
    
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif
