#ifndef JVR_GEOMETRY_CURVE_H
#define JVR_GEOMETRY_CURVE_H

#include <pxr/usd/usdGeom/basisCurves.h>

#include "../geometry/component.h"
#include "../geometry/triangle.h"
#include "../geometry/edge.h"
#include "../geometry/deformable.h"

JVR_NAMESPACE_OPEN_SCOPE

class  Location;

class Curve : public Deformable {
public:
  Curve(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Curve(const pxr::UsdGeomBasisCurves& curve, const pxr::GfMatrix4d& world);
  virtual ~Curve() {};

  void SetCurveWidth(size_t curveIdx, size_t cvIdx, float width);
  void SetCurveWidths(size_t curveIdx, float width);
  void SetCurveWidths(size_t curveIdx, const pxr::VtArray<float>& widths);

  void SetTopology(
    const pxr::VtArray<pxr::GfVec3f>& positions,
    const pxr::VtArray<int>& cvCounts
  );

  void SetTopology(
    const pxr::VtArray<pxr::GfVec3f>& positions,
    const pxr::VtArray<float>& widths,
    const pxr::VtArray<int>& cvCounts
  );

  void RemoveCurve(size_t index);
  void RemoveAllCurves();

  const pxr::VtArray<int>& GetCvCounts() const { return _cvCounts;};
  pxr::VtArray<int>& GetCvCounts() { return _cvCounts;};

  size_t GetNumCurves() const { return _cvCounts.size(); };
  size_t GetNumCVs(uint32_t curveIndex)const;
  size_t GetNumSegments(uint32_t curveIndex)const;

  size_t GetTotalNumCVs()const;
  size_t GetTotalNumSegments()const;

  float GetSegmentLength(uint32_t curveIndex, uint32_t segmentIndex);
  const Edge* GetEdge(size_t index) const {return &_edges[index];};
  Edge* GetEdge(size_t index) {return &_edges[index];};

  void Set(
    const pxr::VtArray<pxr::GfVec3f>& positions, 
    const pxr::VtArray<int>& counts);

  bool ClosestIntersection(const pxr::GfVec3f& origin, 
    const pxr::GfVec3f& direction, Location& location, float maxDistance);

  bool Closest(const pxr::GfVec3f& point, 
    Location& location, float maxDistance);

  // query 3d position on geometry (unaccelarated)
  bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;

protected:
  DirtyState _Sync(const pxr::GfMatrix4d& matrix, 
    const pxr::UsdTimeCode& time=pxr::UsdTimeCode::Default()) override;
  void _Inject(const pxr::GfMatrix4d& parent,
    const pxr::UsdTimeCode& code=pxr::UsdTimeCode::Default()) override;

  void _ComputeEdges();

private:
  size_t                              _PointIndex(size_t curveIdx, size_t cvIdx);

  // curves description
  pxr::VtArray<int>                   _cvCounts;
  pxr::VtArray<Edge>                  _edges;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_CURVE_H
