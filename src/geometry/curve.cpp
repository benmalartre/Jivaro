// Curve
//----------------------------------------------
#include <pxr/usd/usdGeom/basisCurves.h>
#include "../geometry/curve.h"
#include "../geometry/utils.h"
#include "../geometry/edge.h"
#include "../geometry/sampler.h"

JVR_NAMESPACE_OPEN_SCOPE

Curve::Curve(const GfMatrix4d& xfo)
  : Deformable(Geometry::CURVE, xfo)
{
}

Curve::Curve(const UsdGeomBasisCurves& curve, const GfMatrix4d& world)
  : Deformable(curve.GetPrim(), world)
{
  UsdAttribute pointsAttr = curve.GetPointsAttr();
  pointsAttr.Get(&_positions, UsdTimeCode::Default());

  UsdAttribute vertexCountsAttr = curve.GetCurveVertexCountsAttr();
  vertexCountsAttr.Get(&_cvCounts, UsdTimeCode::Default());

  UsdAttribute normalsAttr = curve.GetNormalsAttr();
  if(normalsAttr.IsDefined() && normalsAttr.HasAuthoredValue())
    normalsAttr.Get(&_normals, UsdTimeCode::Default());
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
  const VtArray<GfVec3f>& positions, 
  const VtArray<int>& counts)
{
  _cvCounts = counts;
  SetPositions(positions);
}

bool 
Curve::ClosestIntersection(const GfVec3f& origin, 
  const GfVec3f& direction, Location& location, float maxDistance)
{
  return false;
}

bool 
Curve::Closest(const GfVec3f& point, 
  Location& location, float maxDistance)
{
  return false;
}

bool 
Curve::Raycast(const GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{
  return false;
};

bool 
Curve::Closest(const GfVec3f& point, Location* hit,
  double maxDistance, double* minDistance) const
{
  return false;
};

void
Curve::_ComputeEdges()
{

  /*
  _edges.clear();
  _edges.reserve(_positions.size());

  size_t offset = 0;
  size_t index = 0;
  for(auto& cvCount: _cvCounts) {
    size_t start = offset;
    for(size_t c = 1; c < cvCount; ++c) {
      size_t end = offset + c;
      _edges.push_back(Edge(index++, GfVec2i(start, end), 0.1f));
      start = end;
    }
    offset += cvCount;
  } 
  */
}

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
  const VtArray<GfVec3f>& positions,
  const VtArray<int>& cvCounts)
{
  SetPositions(positions);
  _haveWidths = false;
  _haveNormals = false;
  _haveColors = false;
  _cvCounts = cvCounts;

  _ComputeEdges();
}

void 
Curve::SetTopology(
  const VtArray<GfVec3f>& positions,
  const VtArray<float>& widths,
  const VtArray<int>& cvCounts)
{
  SetPositions(positions);
  SetWidths(widths);
  _haveNormals = false;
  _haveColors = false;
  _cvCounts = cvCounts;

  _ComputeEdges();

}

void 
Curve::RemoveCurve(size_t index)
{
  size_t offset = 0, start, end, num;
  for(size_t i = 0; i < index; ++i) {
    offset += _cvCounts[i];
  }
  num = _cvCounts[index];
  _ComputeEdges();

}

void 
Curve::RemoveAllCurves()
{
  RemoveAllPoints();
  _cvCounts.clear();
  _edges.clear();
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
Curve::SetCurveWidths(size_t curveIdx, const VtArray<float>& widths)
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
Curve::_Sync( const GfMatrix4d& matrix, const UsdTimeCode& time)
{
  if(_prim.IsValid() && _prim.IsA<UsdGeomCurves>())
  {
    _previous = _positions;
    UsdGeomBasisCurves usdCurve(_prim);
    const size_t nbPositions = _positions.size();
    usdCurve.GetPointsAttr().Get(&_positions, time);
  }
  return Geometry::DirtyState::DEFORM;
}

void 
Curve::_Inject(const GfMatrix4d& parent, const UsdTimeCode& time)
{
  if(_prim.IsA<UsdGeomCurves>()) {
    UsdGeomCurves usdCurve(_prim);

    usdCurve.CreatePointsAttr().Set(GetPositions(), time);
    usdCurve.CreateCurveVertexCountsAttr().Set(GetCvCounts(), time);
  }
}


JVR_NAMESPACE_CLOSE_SCOPE