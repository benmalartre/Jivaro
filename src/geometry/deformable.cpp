// Points
//----------------------------------------------
#include "../geometry/deformable.h"
#include "../geometry/utils.h"
#include <pxr/base/gf/ray.h>

JVR_NAMESPACE_OPEN_SCOPE

Deformable::Deformable(short type, const pxr::GfMatrix4d& matrix)
  : Geometry(type, matrix)
{
}

Deformable::Deformable(const Deformable* other, bool normalize)
  : Geometry(other, other->GetType())
{
  size_t numPoints = _positions.size();
  _positions = other->_positions;
  _normals = other->_normals;
  _radius = other->_radius;
  _colors = other->_colors;
}

void
Deformable::SetPositions(const pxr::GfVec3f* positions, size_t n)
{
  if (n != _positions.size()) {
    _positions.resize(n);
  }
  memmove(&_positions[0], positions, n * sizeof(pxr::GfVec3f));
}

void
Deformable::SetRadii(const float* radii, size_t n)
{
  if (n != _radius.size()) {
    _radius.resize(n);
  }
  memmove(&_radius[0], radii, n * sizeof(float));
}

void
Deformable::SetColors(const pxr::GfVec3f* colors, size_t n)
{
  if (n != _colors.size()) {
    _colors.resize(n);
  }
  memmove(&_colors[0], colors, n * sizeof(pxr::GfVec3f));
}

void
Deformable::SetPositions(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _positions = positions;
}

void
Deformable::SetRadii(const pxr::VtArray<float>& radii)
{
  _radius = radii;
}

void
Deformable::SetColors(const pxr::VtArray<pxr::GfVec3f>& colors)
{
  _colors = colors;
}

void
Deformable::Normalize()
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
Deformable::GetPosition(uint32_t index) const
{
  return _positions[index];
}

pxr::GfVec3f 
Deformable::GetNormal(uint32_t index) const
{
  return _normals[index];
}

pxr::GfVec3f
Deformable::GetColor(uint32_t index) const
{
  if (HaveColors())
    return _colors[index];
  else
    return _wirecolor;
}

float
Deformable::GetRadius(uint32_t index) const
{
  return _radius[index];
}

void
Deformable::SetPosition(uint32_t index, const pxr::GfVec3f& position)
{
  _positions[index] = position;
}

void 
Deformable::SetNormal(uint32_t index, const pxr::GfVec3f& normal)
{
  _normals[index] = normal;
}

void 
Deformable::SetRadius(uint32_t index, float radius)
{
  _radius[index] = radius;
}


void 
Deformable::ComputeBoundingBox()
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
Deformable::Init(
  const pxr::VtArray<pxr::GfVec3f>& positions, 
  const pxr::VtArray<float>& radius)
{
  _radius = radius;
  _positions = positions;
  _normals = positions;
}

void 
Deformable::Update(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _positions = positions;
}

void 
Deformable::Update(const pxr::VtArray<float>& radius)
{
  _radius = radius;
}

void 
Deformable::Update(const pxr::VtArray<pxr::GfVec3f>& positions,
  const pxr::VtArray<float>& radius)
{
  _positions = positions;
  _radius = radius;
}

void
Deformable::AddPoint(const pxr::GfVec3f& pos)
{
  _positions.push_back(pos);
  _normals.push_back(pos);
}

void 
Deformable::RemovePoint(size_t index)
{
  _positions.erase(_positions.begin() + index);
  _normals.erase(_normals.begin() + index);
  _radius.erase(_radius.begin() + index);
}

void 
Deformable::RemoveAllPoints()
{
  _positions.clear();
  _normals.clear();
  _radius.clear();
}

void
Deformable::Init(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  size_t numPoints = positions.size();
  _positions = positions;
  _normals = positions;
  _radius.resize(numPoints);
  memset(&_radius[0], 0.1f, numPoints * sizeof(float));
}

Point Deformable::Get(uint32_t index)
{
  if(index < _positions.size())
    return Point(index);   
  else
    return Point(); 
}

JVR_NAMESPACE_CLOSE_SCOPE