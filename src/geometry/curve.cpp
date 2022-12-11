// Curve
//----------------------------------------------
#include "../geometry/curve.h"
#include "../geometry/utils.h"


JVR_NAMESPACE_OPEN_SCOPE

Curve::Curve()
  : Geometry(Geometry::CURVE)
{
  _initialized = false;
  _numCurves = 0;
  _numSegments = 0;
}

Curve::Curve(const Curve* other, bool normalize)
  : Geometry(other, Geometry::CURVE, normalize)
{
  _initialized = true;
  _numCurves = other->_numCurves;
  _numSegments = other->_numSegments;

  _normals = other->_normals;

  _cvCounts.resize(_numCurves);
  memcpy(&_cvCounts[0], &other->_cvCounts[0], _numCurves * sizeof(int));
}

Curve::Curve(const pxr::UsdGeomBasisCurves& curve)
  : Geometry(Geometry::CURVE)
{
  _numCurves = curve.GetCurveCount();

  pxr::UsdAttribute pointsAttr = curve.GetPointsAttr();
  pointsAttr.Get(&_positions, pxr::UsdTimeCode::Default());
  _numPoints = _positions.size();

  pxr::UsdAttribute vertexCountsAttr = curve.GetCurveVertexCountsAttr();
  vertexCountsAttr.Get(&_cvCounts, pxr::UsdTimeCode::Default());

  _numSegments = 0;
  for (const auto& cvCount : _cvCounts) _numSegments += cvCount - 1;

  pxr::UsdAttribute normalsAttr = curve.GetNormalsAttr();
  if(normalsAttr.IsDefined() && normalsAttr.HasAuthoredValue())
    normalsAttr.Get(&_normals, pxr::UsdTimeCode::Default());
}

void Curve::SetDisplayColor(GeomInterpolation interp, 
  const pxr::VtArray<pxr::GfVec3f>& colors) 
{
  _colorsInterpolation = interp;
  _colors = colors;
}

uint32_t Curve::GetNumCVs(uint32_t curveIndex)const
{
  if(curveIndex >= _cvCounts.size())
    return 0;
  return _cvCounts[curveIndex];
}

uint32_t Curve::GetNumSegments(uint32_t curveIndex)const
{
  if (curveIndex >= _cvCounts.size())
    return 0;
  return _cvCounts[curveIndex] - 1;
}

uint32_t Curve::GetTotalNumCVs()const
{
  return _numPoints;
}

uint32_t Curve::GetTotalNumSegments()const
{
  return _numSegments;
}

float Curve::GetSegmentLength(uint32_t curveIndex, uint32_t segmentIndex)
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


void Curve::Init(
  const pxr::VtArray<pxr::GfVec3f>& positions, 
  const pxr::VtArray<int>& counts)
{
  _cvCounts = counts;
  _numCurves = counts.size();
  _numSegments = 0;
  for(const auto& count: counts) _numSegments += count - 1;
  _positions = positions;
  _normals = positions;
  _numPoints = _positions.size();
}

void Curve::Update(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _positions = positions;
}

bool Curve::ClosestIntersection(const pxr::GfVec3f& origin, 
  const pxr::GfVec3f& direction, CurveLocation& location, float maxDistance)
{
  return false;
}

bool Curve::Closest(const pxr::GfVec3f& point, 
  CurveLocation& location, float maxDistance)
{
  return false;
}


JVR_NAMESPACE_CLOSE_SCOPE