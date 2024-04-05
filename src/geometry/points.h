#ifndef JVR_GEOMETRY_POINTS_H
#define JVR_GEOMETRY_POINTS_H

#include "../geometry/deformable.h"


JVR_NAMESPACE_OPEN_SCOPE

class Points : public Deformable {
public:
  Points();
  Points(short type, const pxr::GfMatrix4d& matrix);
  Points(const Points* other, bool normalize = true);
  Points(const pxr::UsdGeomPoints& points, const pxr::GfMatrix4d& world);
  virtual ~Points() {};


protected:
  // vertex data
  pxr::VtArray<pxr::GfVec3f>          _positions;
  pxr::VtArray<pxr::GfVec3f>          _normals;
  pxr::VtArray<pxr::GfVec3f>          _colors;
  pxr::VtArray<float>                 _radius;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif
