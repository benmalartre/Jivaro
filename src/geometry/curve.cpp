// Curve
//----------------------------------------------
#include "../geometry/curve.h"
#include "../geometry/utils.h"
#include "../geometry/sampler.h"

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

  pxr::UsdAttribute vertexCountsAttr = curve.GetCurveVertexCountsAttr();
  vertexCountsAttr.Get(&_cvCounts, pxr::UsdTimeCode::Default());

  _numSegments = 0;
  for (const auto& cvCount : _cvCounts) _numSegments += cvCount - 1;

  pxr::UsdAttribute normalsAttr = curve.GetNormalsAttr();
  if(normalsAttr.IsDefined() && normalsAttr.HasAuthoredValue())
    normalsAttr.Get(&_normals, pxr::UsdTimeCode::Default());
}

uint32_t 
Curve::GetNumCVs(uint32_t curveIndex)const
{
  if(curveIndex >= _cvCounts.size())
    return 0;
  return _cvCounts[curveIndex];
}

uint32_t 
Curve::GetNumSegments(uint32_t curveIndex)const
{
  if (curveIndex >= _cvCounts.size())
    return 0;
  return _cvCounts[curveIndex] - 1;
}

uint32_t 
Curve::GetTotalNumCVs()const
{
  return _positions.size();
}

uint32_t 
Curve::GetTotalNumSegments()const
{
  return _numSegments;
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
Curve::Init(
  const pxr::VtArray<pxr::GfVec3f>& positions, 
  const pxr::VtArray<int>& counts)
{
  _cvCounts = counts;
  _numCurves = counts.size();
  _numSegments = 0;
  for(const auto& count: counts) _numSegments += count - 1;
  _positions = positions;
  _normals = positions;
}

void 
Curve::Update(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _positions = positions;
}

bool 
Curve::ClosestIntersection(const pxr::GfVec3f& origin, 
  const pxr::GfVec3f& direction, CurveLocation& location, float maxDistance)
{
  return false;
}

bool 
Curve::Closest(const pxr::GfVec3f& point, 
  CurveLocation& location, float maxDistance)
{
  return false;
}

bool 
Curve::Raycast(const pxr::GfRay& ray, Hit* hit,
  double maxDistance, double* minDistance) const
{
  return false;
};

bool 
Curve::Closest(const pxr::GfVec3f& point, Hit* hit,
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
Curve::SetRadius(size_t curveIdx, size_t cvIdx, float radius)
{
  size_t pointIdx = _PointIndex(curveIdx, cvIdx);
  _radius[pointIdx] = radius;
}

void 
Curve::SetRadii(size_t curveIdx, float radius)
{
  size_t pointIdx = _PointIndex(curveIdx, 0);
  for (size_t p = pointIdx; p < pointIdx + _cvCounts[curveIdx]; ++p) {
    _radius[pointIdx] = radius;
  }
}

void
Curve::SetRadii(size_t curveIdx, const pxr::VtArray<float>& radii)
{
  size_t numCVs = _cvCounts[curveIdx];

  if (radii.size() == numCVs) {
    size_t startIdx = _PointIndex(curveIdx, 0);
    for (size_t cvIdx = 0; cvIdx < numCVs; ++cvIdx) {
      _radius[startIdx + cvIdx] = radii[cvIdx];
    }
  }
}

void
Curve::MaterializeSamples(const pxr::VtArray<Sample>& samples, int N, 
  const pxr::GfVec3f* positions, const pxr::GfVec3f* normals)
{
  _numCurves = samples.size();
  _cvCounts.resize(_numCurves);
  std::fill(_cvCounts.begin(), _cvCounts.end(), N);
  _radius.resize(N * _numCurves);
  _positions.resize(N * _numCurves);

  for (size_t s = 0; s < samples.size(); ++s) {
    const pxr::GfVec3f& origin = samples[s].GetPosition(positions);
    const pxr::GfVec3f& normal = samples[s].GetNormal(normals);
    const pxr::GfVec3f& tangent = samples[s].GetTangent(positions, normals);

    _positions[s * 4] = origin;
    _positions[s * 4 + 1] = origin + normal * 0.1 + tangent * 0.33;
    _positions[s * 4 + 2] = origin + normal * 0.5 + tangent * 0.66;
    _positions[s * 4 + 3] = origin + normal + tangent;

    _radius[s * 4] = 0.1;
    _radius[s * 4 + 1] = 0.08;
    _radius[s * 4 + 2] = 0.04;
    _radius[s * 4 + 3] = 0.02;

  }
}


JVR_NAMESPACE_CLOSE_SCOPE