#ifndef AMN_GEOMETRY_POINTS_H
#define AMN_GEOMETRY_POINTS_H


#include "../common.h"
#include "pxr/base/vt/array.h"
#include "pxr/base/tf/hashmap.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <float.h>
#include "triangle.h"
#include "geometry.h"

AMN_NAMESPACE_OPEN_SCOPE

class Points : public Geometry {
public:
  Points();
  Points(const Points* other, bool normalize = true);
  ~Points();


  void SetDisplayColor(GeomInterpolation interp, 
    const pxr::VtArray<pxr::GfVec3f>& colors);
  const pxr::VtArray<pxr::GfVec3f>& GetDisplayColor() const {return _colors;};
  GeomInterpolation GetDisplayColorInterpolation() const {
    return _colorsInterpolation;
  };

  void Init(
    const pxr::VtArray<pxr::GfVec3f>& positions, 
    const pxr::VtArray<float>& radius);

  void Update(const pxr::VtArray<pxr::GfVec3f>& positions);
  void Update(const pxr::VtArray<float>& radius);
  void Update(const pxr::VtArray<pxr::GfVec3f>& positions,
    const pxr::VtArray<float>& radius);

private:
  // curves description
  pxr::VtArray<float>                 _radius;  

  // colors
  pxr::VtArray<pxr::GfVec3f>          _colors;
  GeomInterpolation                   _colorsInterpolation;

};

AMN_NAMESPACE_CLOSE_SCOPE

#endif
