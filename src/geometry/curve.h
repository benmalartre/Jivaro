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
#include "../geometry/triangle.h"
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

struct Sample;
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

  void SetRadius(size_t curveIdx, size_t cvIdx, float radius);
  void SetRadii(size_t curveIdx, float radius);
  void SetRadii(size_t curveIdx, const pxr::VtArray<float>& radii);

  void SetTopology(
    const pxr::VtArray<pxr::GfVec3f>& positions,
    const pxr::VtArray<int>& cvCounts
  );

  void SetTopology(
    const pxr::VtArray<pxr::GfVec3f>& positions,
    const pxr::VtArray<float>& radius,
    const pxr::VtArray<int>& cvCounts
  );

  const pxr::VtArray<int>& GetCvCounts() const { return _cvCounts;};
  pxr::VtArray<int>& GetCvCounts() { return _cvCounts;};

  size_t GetNumCurves() const { return _cvCounts.size(); };
  size_t GetNumCVs(uint32_t curveIndex)const;
  size_t GetNumSegments(uint32_t curveIndex)const;

  size_t GetTotalNumCVs()const;
  size_t GetTotalNumSegments()const;

  float GetSegmentLength(uint32_t curveIndex, uint32_t segmentIndex);

  void MaterializeSamples(const pxr::VtArray<Sample>& samples, int N,
    const pxr::GfVec3f* positions, const pxr::GfVec3f* normals, float width);

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
  size_t                              _PointIndex(size_t curveIdx, size_t cvIdx);

  // curves description
  pxr::VtArray<int>                   _cvCounts;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_CURVE_H
