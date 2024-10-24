#ifndef JVR_GEOMETRY_POINTS_H
#define JVR_GEOMETRY_POINTS_H

#include <pxr/usd/usdGeom/points.h>

#include "../geometry/deformable.h"


JVR_NAMESPACE_OPEN_SCOPE
class Voxels;
class Points : public Deformable {
public:
  Points(const GfMatrix4d& xfo=GfMatrix4d(1.0));
  Points(const UsdPrim& prim, const GfMatrix4d& world);
  virtual ~Points() {};

protected:
  DirtyState _Sync(const GfMatrix4d& matrix, 
    const UsdTimeCode& code=UsdTimeCode::Default()) override;
  void _Inject(const GfMatrix4d& parent,
    const UsdTimeCode& code=UsdTimeCode::Default()) override;
    
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif
