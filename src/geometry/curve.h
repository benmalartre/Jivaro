#ifndef JVR_GEOMETRY_CURVE_H
#define JVR_GEOMETRY_CURVE_H

#include <pxr/usd/usdGeom/basisCurves.h>

#include "../geometry/triangle.h"
#include "../geometry/deformable.h"

JVR_NAMESPACE_OPEN_SCOPE

struct Sample;

class Curve : public Deformable {
public:
  Curve(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Curve(const pxr::UsdGeomBasisCurves& curve, const pxr::GfMatrix4d& world);
  virtual ~Curve() {};

  void SetCurveRadius(size_t curveIdx, size_t cvIdx, float radius);
  void SetCurveRadii(size_t curveIdx, float radius);
  void SetCurveRadii(size_t curveIdx, const pxr::VtArray<float>& radii);

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
    const pxr::GfVec3f& direction, Location& location, float maxDistance);

  bool Closest(const pxr::GfVec3f& point, 
    Location& location, float maxDistance);

  // query 3d position on geometry (unaccelarated)
  bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;

private:
  size_t                              _PointIndex(size_t curveIdx, size_t cvIdx);

  // curves description
  pxr::VtArray<int>                   _cvCounts;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_CURVE_H
