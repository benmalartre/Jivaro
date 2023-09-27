// Points
//----------------------------------------------
#include "../geometry/points.h"
#include "../geometry/utils.h"
#include <pxr/base/gf/ray.h>

JVR_NAMESPACE_OPEN_SCOPE

Points::Points()
  : Geometry(Geometry::POINT, pxr::GfMatrix4d(1.0))
{
  _initialized = false;
}

Points::Points(Geometry::Type type, const pxr::GfMatrix4d& matrix)
  : Geometry(type, matrix)
{
  _initialized = false;
}

Points::Points(const Points* other, bool normalize)
  : Geometry(other, other->GetType(), normalize)
{
  size_t numPoints = _positions.size();
  _initialized = true;

  _positions = other->_positions;
  _normals = other->_normals;
  _radius = other->_radius;
  _colors = other->_colors;
}

Points::Points(const pxr::UsdGeomPoints& points, const pxr::GfMatrix4d& world)
  : Geometry(Geometry::POINT, world)
{

  pxr::UsdAttribute pointsAttr = points.GetPointsAttr();
  pointsAttr.Get(&_positions, pxr::UsdTimeCode::Default());

  pxr::UsdAttribute normalsAttr = points.GetNormalsAttr();
  if (normalsAttr.IsDefined() && normalsAttr.HasAuthoredValue())
    normalsAttr.Get(&_normals, pxr::UsdTimeCode::Default());

  pxr::UsdAttribute widthsAttr = points.GetWidthsAttr();
  if (widthsAttr.IsDefined() && widthsAttr.HasAuthoredValue())
    widthsAttr.Get(&_radius, pxr::UsdTimeCode::Default());
}

void
Points::SetPositions(const pxr::GfVec3f* positions, size_t n)
{
  if (n != _positions.size()) {
    _positions.resize(n);
  }
  memmove(&_positions[0], positions, n * sizeof(pxr::GfVec3f));
}

void
Points::SetRadii(const float* radii, size_t n)
{
  if (n != _radius.size()) {
    _radius.resize(n);
  }
  memmove(&_radius[0], radii, n * sizeof(float));
}

void
Points::SetColors(const pxr::GfVec3f* colors, size_t n)
{
  if (n != _colors.size()) {
    _colors.resize(n);
  }
  memmove(&_colors[0], colors, n * sizeof(pxr::GfVec3f));
}

void
Points::SetPositions(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _positions = positions;
}

void
Points::SetRadii(const pxr::VtArray<float>& radii)
{
  _radius = radii;
}

void
Points::SetColors(const pxr::VtArray<pxr::GfVec3f>& colors)
{
  _colors = colors;
}

void
Points::Normalize()
{
  if (!_positions.size())return;

  // compute center of mass
  pxr::GfVec3f center(0.f);
  for (const auto& position: _positions) {
    center += position;
  }

  center *= 1.0 / (float)_positions.size();

  // translate to origin
  for (auto& position: _positions) {
    position -= center;
  }

  // determine radius
  float rMax = 0;
  for (const auto& position: _positions) {
    rMax = std::max(rMax, position.GetLength());
  }

  // rescale to unit sphere
  float invRMax = 1.f / rMax;
  for (auto& position: _positions) {
    position *= invRMax;
  }
}

pxr::GfVec3f 
Points::GetPosition(uint32_t index) const
{
  return _positions[index];
}

pxr::GfVec3f 
Points::GetNormal(uint32_t index) const
{
  return _normals[index];
}

pxr::GfVec3f
Points::GetColor(uint32_t index) const
{
  if (HasColors())
    return _colors[index];
  else
    return _wirecolor;
}

float
Points::GetRadius(uint32_t index) const
{
  return _radius[index];
}

void
Points::SetPosition(uint32_t index, const pxr::GfVec3f& position)
{
  _positions[index] = position;
}

void 
Points::SetNormal(uint32_t index, const pxr::GfVec3f& normal)
{
  _normals[index] = normal;
}

void 
Points::SetRadius(uint32_t index, float radius)
{
  _radius[index] = radius;
}


void 
Points::ComputeBoundingBox()
{
  pxr::GfRange3d range;
  range.SetEmpty();
  auto min = range.GetMin();
  auto max = range.GetMax();
  for (const auto& point : _positions) {
    for (short i = 0; i < 3; ++i) {
      if (point[i] < min[i])min[i] = point[i];
      if (point[i] > max[i])max[i] = point[i];
    }
  }
  range.SetMin(min);
  range.SetMax(max);
  _bbox.Set(range, _matrix);
   
}


void 
Points::Init(
  const pxr::VtArray<pxr::GfVec3f>& positions, 
  const pxr::VtArray<float>& radius)
{
  _radius = radius;
  _positions = positions;
  _normals = positions;
}

void 
Points::Update(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _positions = positions;
}

void 
Points::Update(const pxr::VtArray<float>& radius)
{
  _radius = radius;
}

void 
Points::Update(const pxr::VtArray<pxr::GfVec3f>& positions,
  const pxr::VtArray<float>& radius)
{
  _positions = positions;
  _radius = radius;
}

void
Points::AddPoint(const pxr::GfVec3f& pos)
{
  _positions.push_back(pos);
  _normals.push_back(pos);
}

void 
Points::RemovePoint(size_t index)
{
  _positions.erase(_positions.begin() + index);
  _normals.erase(_normals.begin() + index);
  _radius.erase(_radius.begin() + index);
}

void 
Points::RemoveAllPoints()
{
  _positions.clear();
  _normals.clear();
  _radius.clear();
}

void
Points::Init(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  size_t numPoints = positions.size();
  _positions = positions;
  _normals = positions;
  _radius.resize(numPoints);
  memset(&_radius[0], 0.1f, numPoints * sizeof(float));
}

Point Points::Get(uint32_t index)
{
  if(index < _positions.size())
    return Point(index);   
  else
    return Point(); 
}

JVR_NAMESPACE_CLOSE_SCOPE