// Curve
//----------------------------------------------
#include <pxr/usd/usdGeom/basisCurves.h>
#include "../geometry/curve.h"
#include "../geometry/utils.h"
#include "../geometry/sampler.h"

JVR_NAMESPACE_OPEN_SCOPE

Curve::Curve(const pxr::GfMatrix4d& xfo)
  : Deformable(Geometry::CURVE, xfo)
{
}

Curve::Curve(const pxr::UsdGeomBasisCurves& curve, const pxr::GfMatrix4d& world)
  : Deformable(Geometry::CURVE, world)
{
  pxr::UsdAttribute pointsAttr = curve.GetPointsAttr();
  pointsAttr.Get(&_positions, pxr::UsdTimeCode::Default());

  pxr::UsdAttribute vertexCountsAttr = curve.GetCurveVertexCountsAttr();
  vertexCountsAttr.Get(&_cvCounts, pxr::UsdTimeCode::Default());

  pxr::UsdAttribute normalsAttr = curve.GetNormalsAttr();
  if(normalsAttr.IsDefined() && normalsAttr.HasAuthoredValue())
    normalsAttr.Get(&_normals, pxr::UsdTimeCode::Default());
}

size_t 
Curve::GetNumCVs(uint32_t curveIndex)const
{
  if(curveIndex >= _cvCounts.size())
    return 0;
  return _cvCounts[curveIndex];
}

size_t 
Curve::GetNumSegments(uint32_t curveIndex)const
{
  if (curveIndex >= _cvCounts.size())
    return 0;
  return _cvCounts[curveIndex] - 1;
}

size_t 
Curve::GetTotalNumCVs()const
{
  return _positions.size();
}

size_t 
Curve::GetTotalNumSegments()const
{
  size_t numSegments = 0;
  for (const auto& cvCount : _cvCounts) {
    numSegments += cvCount - 1;
  }
  return numSegments;
}

float 
Curve::GetSegmentLength(uint32_t curveIndex, uint32_t segmentIndex)
{
  size_t numCurves = _cvCounts.size();
  if(curveIndex >= numCurves)
    return -1.f;
  uint32_t numCurveSegments = _cvCounts[curveIndex];
  if(segmentIndex >= numCurveSegments)
    return -1.f;

  size_t baseCvIndex = segmentIndex;
  for(size_t i=0; i < curveIndex - 1; ++i)baseCvIndex += _cvCounts[i];

  return (_positions[segmentIndex] - _positions[segmentIndex + 1]).GetLength();
}


void 
Curve::Set(
  const pxr::VtArray<pxr::GfVec3f>& positions, 
  const pxr::VtArray<int>& counts)
{
  _cvCounts = counts;
  SetPositions(positions);
}

bool 
Curve::ClosestIntersection(const pxr::GfVec3f& origin, 
  const pxr::GfVec3f& direction, Location& location, float maxDistance)
{
  return false;
}

bool 
Curve::Closest(const pxr::GfVec3f& point, 
  Location& location, float maxDistance)
{
  return false;
}

bool 
Curve::Raycast(const pxr::GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{
  return false;
};

bool 
Curve::Closest(const pxr::GfVec3f& point, Location* hit,
  double maxDistance, double* minDistance) const
{
  return false;
};

size_t
Curve::_PointIndex(size_t curveIdx, size_t cvIdx)
{
  size_t index = 0;
  for (size_t c = 0; c < curveIdx; ++c) {
    index += _cvCounts[c];
  }
  return index + cvIdx;
}

void 
Curve::SetTopology(
  const pxr::VtArray<pxr::GfVec3f>& positions,
  const pxr::VtArray<int>& cvCounts)
{
  SetPositions(positions);
  _haveRadius = false;
  _haveNormals = false;
  _haveColors = false;
  _cvCounts = cvCounts;
}

void 
Curve::SetTopology(
  const pxr::VtArray<pxr::GfVec3f>& positions,
  const pxr::VtArray<float>& radii,
  const pxr::VtArray<int>& cvCounts)
{
  SetPositions(positions);
  SetRadii(radii);
  _haveNormals = false;
  _haveColors = false;
  _cvCounts = cvCounts;
}

void
Curve::SetCurveRadius(size_t curveIdx, size_t cvIdx, float radius)
{
  size_t pointIdx = _PointIndex(curveIdx, cvIdx);
  _radius[pointIdx] = radius;
}

void
Curve::SetCurveRadii(size_t curveIdx, float radius)
{
  size_t pointIdx = _PointIndex(curveIdx, 0);
  for (size_t p = pointIdx; p < pointIdx + _cvCounts[curveIdx]; ++p) {
    _radius[pointIdx] = radius;
  }
}

void
Curve::SetCurveRadii(size_t curveIdx, const pxr::VtArray<float>& radii)
{
  size_t numCVs = _cvCounts[curveIdx];

  if (radii.size() == numCVs) {
    size_t startIdx = _PointIndex(curveIdx, 0);
    for (size_t cvIdx = 0; cvIdx < numCVs; ++cvIdx) {
      _radius[startIdx + cvIdx] = radii[cvIdx];
    }
  }
}

Geometry::DirtyState 
Curve::_Sync(pxr::UsdPrim& prim, const pxr::GfMatrix4d& matrix, const pxr::UsdTimeCode& time)
{
  if(prim.IsValid() && prim.IsA<pxr::UsdGeomCurves>())
  {
    _previous = _positions;
    pxr::UsdGeomBasisCurves usdCurve(prim);
    const size_t nbPositions = _positions.size();
    usdCurve.GetPointsAttr().Get(&_positions, time);
  }
  return Geometry::DirtyState::DEFORM;
}

void 
Curve::_Inject(pxr::UsdPrim& prim, const pxr::GfMatrix4d& parent,
    const pxr::UsdTimeCode& time)
{
  if(prim.IsA<pxr::UsdGeomCurves>()) {
    pxr::UsdGeomCurves usdCurve(prim);

    usdCurve.CreatePointsAttr().Set(GetPositions(), time);
    usdCurve.CreateCurveVertexCountsAttr().Set(GetCvCounts(), time);
  }
}


JVR_NAMESPACE_CLOSE_SCOPE