#ifndef AMN_GEOMETRY_CURVE_H
#define AMN_GEOMETRY_CURVE_H


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

struct CurveLocation {
  Geometry* geometry; // geometry ptr
  uint32_t  cid;      // curve index
  uint32_t  sid;      // segment index
  float     u;        // u along segment
};

class Curve : public Geometry {
public:
  Curve();
  Curve(const Curve* other, bool normalize = true);
  ~Curve();

  const pxr::VtArray<int>& GetCVsCount() const { return _cvCounts;};
  pxr::VtArray<int>& GetCVsCounts() { return _cvCounts;};

  void SetDisplayColor(GeomInterpolation interp, 
    const pxr::VtArray<pxr::GfVec3f>& colors);
  const pxr::VtArray<pxr::GfVec3f>& GetDisplayColor() const {return _colors;};
  GeomInterpolation GetDisplayColorInterpolation() const {
    return _colorsInterpolation;
  };

  uint32_t GetNumCVs(uint32_t curveIndex)const;

  float SegmentLength(uint32_t index);

  void Init(
    const pxr::VtArray<pxr::GfVec3f>& positions, 
    const pxr::VtArray<int>& counts);

  void Update(const pxr::VtArray<pxr::GfVec3f>& positions);

  bool ClosestIntersection(const pxr::GfVec3f& origin, 
    const pxr::GfVec3f& direction, CurveLocation& location, float maxDistance);

  bool ClosestPoint(const pxr::GfVec3f& point, 
    CurveLocation& location, float maxDistance);

private:
  // infos
  uint32_t                            _numCurves;
  uint32_t                            _numSegments;

  // curves description
  pxr::VtArray<int>                   _cvCounts;  

  // colors
  pxr::VtArray<pxr::GfVec3f>          _colors;
  GeomInterpolation                   _colorsInterpolation;

};

AMN_NAMESPACE_CLOSE_SCOPE

#endif
