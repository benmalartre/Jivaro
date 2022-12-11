#ifndef JVR_GEOMETRY_POINTS_H
#define JVR_GEOMETRY_POINTS_H


#include "../common.h"
#include "pxr/base/vt/array.h"
#include "pxr/base/tf/hashmap.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/points.h>
#include <float.h>
#include "../geometry/point.h"
#include "../geometry/triangle.h"
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

class Point;
class Points : public Geometry {
public:
  Points();
  Points(const Points* other, bool normalize = true);
  Points(const pxr::UsdGeomPoints& points);
  virtual ~Points() {};

  void SetDisplayColor(GeomInterpolation interp, 
    const pxr::VtArray<pxr::GfVec3f>& colors);
  const pxr::VtArray<pxr::GfVec3f>& GetDisplayColor() const {return _colors;};
  GeomInterpolation GetDisplayColorInterpolation() const {
    return _colorsInterpolation;
  };

  Point Get(uint32_t index);

  void Init(
    const pxr::VtArray<pxr::GfVec3f>& positions, 
    const pxr::VtArray<float>& radius);

  void Update(const pxr::VtArray<pxr::GfVec3f>& positions);
  void Update(const pxr::VtArray<float>& radius);
  void Update(const pxr::VtArray<pxr::GfVec3f>& positions,
    const pxr::VtArray<float>& radius);

  // query 3d position on geometry
  bool Raycast(const pxr::GfRay& ray, Hit* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override {
    return false;
  };
  bool Closest(const pxr::GfVec3f& point, Hit* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override {
    return false;
  };

private:
  // radius description
  pxr::VtArray<float>                 _radius;  

  // colors
  pxr::VtArray<pxr::GfVec3f>          _colors;
  GeomInterpolation                   _colorsInterpolation;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif
