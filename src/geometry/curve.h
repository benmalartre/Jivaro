#ifndef JVR_GEOMETRY_CURVE_H
#define JVR_GEOMETRY_CURVE_H


#include "../common.h"
#include "pxr/base/vt/array.h"
#include "pxr/base/tf/hashmap.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/ray.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/basisCurves.h>

#include <float.h>
#include "triangle.h"
#include "geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

struct CurveLocation {
  uint32_t  cid;      // curve index
  uint32_t  sid;      // segment index
  float     u;        // u along segment
};

class Curve : public Geometry {
public:
  Curve();
  Curve(const Curve* other, bool normalize = true);
  Curve(const pxr::UsdGeomBasisCurves& curve);
  virtual ~Curve() {};

  const pxr::VtArray<int>& GetCvCounts() const { return _cvCounts;};
  pxr::VtArray<int>& GetCvCounts() { return _cvCounts;};

  void SetDisplayColor(GeomInterpolation interp, 
    const pxr::VtArray<pxr::GfVec3f>& colors);
  const pxr::VtArray<pxr::GfVec3f>& GetDisplayColor() const {return _colors;};
  GeomInterpolation GetDisplayColorInterpolation() const {
    return _colorsInterpolation;
  };

  uint32_t GetNumCurves() const { return _numCurves; };
  uint32_t GetNumCVs(uint32_t curveIndex)const;
  uint32_t GetNumSegments(uint32_t curveIndex)const;

  uint32_t GetTotalNumCVs()const;
  uint32_t GetTotalNumSegments()const;

  float GetSegmentLength(uint32_t curveIndex, uint32_t segmentIndex);

  void Init(
    const pxr::VtArray<pxr::GfVec3f>& positions, 
    const pxr::VtArray<int>& counts);

  void Update(const pxr::VtArray<pxr::GfVec3f>& positions);

  bool ClosestIntersection(const pxr::GfVec3f& origin, 
    const pxr::GfVec3f& direction, CurveLocation& location, float maxDistance);

  bool Closest(const pxr::GfVec3f& point, 
    CurveLocation& location, float maxDistance);

  // query 3d position on geometry (unaccelarated)
  bool Raycast(const pxr::GfRay& ray, Hit* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Hit* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;

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

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_CURVE_H
