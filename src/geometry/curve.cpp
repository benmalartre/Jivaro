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
  : Deformable(curve.GetPrim(), world)
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
  _haveWidths = false;
  _haveNormals = false;
  _haveColors = false;
  _cvCounts = cvCounts;
}

void 
Curve::SetTopology(
  const pxr::VtArray<pxr::GfVec3f>& positions,
  const pxr::VtArray<float>& widths,
  const pxr::VtArray<int>& cvCounts)
{
  SetPositions(positions);
  SetWidths(widths);
  _haveNormals = false;
  _haveColors = false;
  _cvCounts = cvCounts;
}

void 
Curve::RemoveCurve(size_t index)
{
  size_t offset = 0, start, end, num;
  for(size_t i = 0; i < index; ++i) {
    offset += _cvCounts[i];
  }
  num = _cvCounts[index];
}

void 
Curve::RemoveAllCurves()
{
  RemoveAllPoints();
  _cvCounts.clear();
}

void
Curve::SetCurveWidth(size_t curveIdx, size_t cvIdx, float width)
{
  size_t pointIdx = _PointIndex(curveIdx, cvIdx);
  _widths[pointIdx] = width;
}

void
Curve::SetCurveWidths(size_t curveIdx, float width)
{
  size_t pointIdx = _PointIndex(curveIdx, 0);
  for (size_t p = pointIdx; p < pointIdx + _cvCounts[curveIdx]; ++p) {
    _widths[pointIdx] = width;
  }
}

void
Curve::SetCurveWidths(size_t curveIdx, const pxr::VtArray<float>& widths)
{
  size_t numCVs = _cvCounts[curveIdx];

  if (widths.size() == numCVs) {
    size_t startIdx = _PointIndex(curveIdx, 0);
    for (size_t cvIdx = 0; cvIdx < numCVs; ++cvIdx) {
      _widths[startIdx + cvIdx] = widths[cvIdx];
    }
  }
}

Geometry::DirtyState 
Curve::_Sync( const pxr::GfMatrix4d& matrix, const pxr::UsdTimeCode& time)
{
  if(_prim.IsValid() && _prim.IsA<pxr::UsdGeomCurves>())
  {
    _previous = _positions;
    pxr::UsdGeomBasisCurves usdCurve(_prim);
    const size_t nbPositions = _positions.size();
    usdCurve.GetPointsAttr().Get(&_positions, time);
  }
  return Geometry::DirtyState::DEFORM;
}

void 
Curve::_Inject(const pxr::GfMatrix4d& parent, const pxr::UsdTimeCode& time)
{
  if(_prim.IsA<pxr::UsdGeomCurves>()) {
    pxr::UsdGeomCurves usdCurve(_prim);

    usdCurve.CreatePointsAttr().Set(GetPositions(), time);
    usdCurve.CreateCurveVertexCountsAttr().Set(GetCvCounts(), time);
  }
}


JVR_NAMESPACE_CLOSE_SCOPE