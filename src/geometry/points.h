#ifndef JVR_GEOMETRY_POINTS_H
#define JVR_GEOMETRY_POINTS_H

#include "../geometry/deformable.h"


JVR_NAMESPACE_OPEN_SCOPE
class Voxels;
class Points : public Deformable {
public:
  Points(short type=Geometry::POINT, const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Points(const Points& other, bool normalize = true);
  Points(const pxr::UsdGeomPoints& points, const pxr::GfMatrix4d& world);
  virtual ~Points() {};

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif
