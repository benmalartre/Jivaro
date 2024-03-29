// Curve
//----------------------------------------------
#include "../geometry/curve.h"
#include "../geometry/utils.h"
#include "../geometry/sampler.h"

JVR_NAMESPACE_OPEN_SCOPE

Curve::Curve()
  : Points(Geometry::CURVE, pxr::GfMatrix4d(1.0))
{
}

Curve::Curve(const Curve* other, bool normalize)
  : Points(other, normalize)
{
  size_t numCurves = other->_cvCounts.size();
  _cvCounts.resize(numCurves);
  memcpy(&_cvCounts[0], &other->_cvCounts[0], numCurves * sizeof(int));
}

Curve::Curve(const pxr::UsdGeomBasisCurves& curve, const pxr::GfMatrix4d& world)
  : Points(Geometry::CURVE, world)
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
Curve::Init(
  const pxr::VtArray<pxr::GfVec3f>& positions, 
  const pxr::VtArray<int>& counts)
{
  _cvCounts = counts;
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
Curve::SetTopology(
  const pxr::VtArray<pxr::GfVec3f>& positions,
  const pxr::VtArray<int>& cvCounts)
{
  _positions = positions;
  _normals = positions;
  _radius.resize(_positions.size());
  _cvCounts = cvCounts;
}

void 
Curve::SetTopology(
  const pxr::VtArray<pxr::GfVec3f>& positions,
  const pxr::VtArray<float>& radii,
  const pxr::VtArray<int>& cvCounts)
{
  _positions = positions;
  _normals = positions;
  _radius = radii;
  _cvCounts = cvCounts;
}

void
Curve::MaterializeSamples(const pxr::VtArray<Sample>& samples, int N, 
  const pxr::GfVec3f* positions, const pxr::GfVec3f* normals, float width)
{
  size_t numCurves = samples.size();
  _cvCounts.resize(numCurves);
  size_t numCVs = N * numCurves;
  _radius.resize(numCVs);
  _positions.resize(numCVs);

  std::fill(_cvCounts.begin(), _cvCounts.end(), N);

  for (size_t s = 0; s < numCurves; ++s) {
    const pxr::GfVec3f& origin = samples[s].GetPosition(positions);
    const pxr::GfVec3f& normal = samples[s].GetNormal(normals);
    const pxr::GfVec3f& tangent = samples[s].GetTangent(positions, normals);

    _positions[s * 4] = origin;
    _positions[s * 4 + 1] = origin + normal * 0.1 + tangent * 0.33;
    _positions[s * 4 + 2] = origin + normal * 0.5 + tangent * 0.66;
    _positions[s * 4 + 3] = origin + normal + tangent;

    _radius[s * 4] = width;
    _radius[s * 4 + 1] = width * 0.8;
    _radius[s * 4 + 2] = width * 0.4;
    _radius[s * 4 + 3] = width * 0.2;
  }
}

void
CurveLocation::GetPosition(const Geometry* geom, 
  pxr::GfVec3f* pos, bool worldSpace) const
{
  if(geom->GetType() != Geometry::CURVE) return;
  
  const Curve* curve = (const Curve*)geom;
  /*
  const Triangle* T = mesh->GetTriangle(_id);
  const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();

  for(uint32_t i = 0; i < 3; ++i) 
    *pos += positions[T->vertices[i]] * _baryCoords[i];

  if(worldSpace) {
    mesh->GetMatrix().Transform(*pos);
  }
  */
}

void
CurveLocation::GetNormal(const Geometry* geom,
  pxr::GfVec3f* nrm, bool worldSpace) const
{
  if(geom->GetType() != Geometry::CURVE) return;
  
  const Curve* curve = (const Curve*)geom;
  /*
  const Triangle* T = mesh->GetTriangle(_id);
  const pxr::GfVec3f* normals = mesh->GetNormalsCPtr();

  for(uint32_t i=0;i<3;i++) 
    *nrm += normals[T->vertices[i]] * _baryCoords[i];

  if(worldSpace) {
    mesh->GetMatrix().TransformDir(*nrm);
  }
  */
}

JVR_NAMESPACE_CLOSE_SCOPE