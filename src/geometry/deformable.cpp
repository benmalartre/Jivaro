// Points
//----------------------------------------------
#include "../geometry/deformable.h"
#include "../geometry/utils.h"
#include <pxr/base/gf/ray.h>

JVR_NAMESPACE_OPEN_SCOPE

Deformable::Deformable(short type, const pxr::GfMatrix4d& matrix)
  : Geometry(type, matrix)
  , _haveRadius(false)
  , _haveNormals(false)
  , _haveColors(false)
{
}

Deformable::Deformable(const Deformable* other, bool normalize)
  : Geometry(other, other->GetType())
{
  size_t numPoints = _positions.size();
  _previous = other->_previous;
  _positions = other->_positions;
  _haveNormals = other->_haveNormals;
  _normals = other->_normals;
  _haveRadius = other->_haveRadius;
  _radius = other->_radius;
  _haveColors = other->_haveColors;
  _colors = other->_colors;
}

void Deformable::_ValidateNumPoints(size_t n)
{
  std::cout << "validate num points " << n << " vs " << GetNumPoints() << std::endl;
  if (n != GetNumPoints()) {
    _positions.resize(n);
    _previous.resize(n);
  }
  if(_haveNormals && n != _normals.size())_normals.resize(n);
  if(_haveRadius && n != _radius.size())_radius.resize(n);
  if (_haveColors && n != _colors.size())_colors.resize(n);

}

void
Deformable::SetPositions(const pxr::GfVec3f* positions, size_t n)
{
  _ValidateNumPoints(n);
  memcpy(&_previous[0], &_positions[0], n * sizeof(pxr::GfVec3f));
  memcpy(&_positions[0], positions, n * sizeof(pxr::GfVec3f));
}

void
Deformable::SetNormals(const pxr::GfVec3f* normals, size_t n)
{
  _haveNormals = true;
  _ValidateNumPoints(n);
  memcpy(&_normals[0], normals, n * sizeof(pxr::GfVec3f));
}

void
Deformable::SetRadii(const float* radii, size_t n)
{
  _haveRadius = true;
  _ValidateNumPoints(n);
  memcpy(&_radius[0], radii, n * sizeof(float));
}

void
Deformable::SetColors(const pxr::GfVec3f* colors, size_t n)
{
  _haveColors = true;
  _ValidateNumPoints(n);
  memcpy(&_colors[0], colors, n * sizeof(pxr::GfVec3f));
}

void
Deformable::SetPositions(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  const size_t n = positions.size();
  std::cout << "set position " << n << std::endl;
  _ValidateNumPoints(n);
  memcpy(&_previous[0], &_positions[0], n * sizeof(pxr::GfVec3f));
  memcpy(&_positions[0], &positions[0], n * sizeof(pxr::GfVec3f));
}

void
Deformable::SetNormals(const pxr::VtArray<pxr::GfVec3f>& normals)
{
  _haveNormals = true;
  const size_t n = normals.size();
  _ValidateNumPoints(n);
  memcpy(&_normals[0], &normals[0], n * sizeof(pxr::GfVec3f));
}

void
Deformable::SetRadii(const pxr::VtArray<float>& radius)
{
  _haveRadius = true;
  const size_t n = radius.size();
  _ValidateNumPoints(n);
  memcpy(&_radius[0], &radius[0], n * sizeof(float));
}

void
Deformable::SetColors(const pxr::VtArray<pxr::GfVec3f>& colors)
{
  _haveColors = true;
  const size_t n = colors.size();
  _ValidateNumPoints(n);
  memcpy(&_colors[0], &colors[0], n * sizeof(pxr::GfVec3f));
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
Deformable::GetPrevious(uint32_t index) const
{
  return _previous[index];
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
Deformable::SetPrevious(uint32_t index, const pxr::GfVec3f& previous)
{
  _previous[index] = previous;
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
Deformable::Update(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _previous = _positions;
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
Deformable::AddPoint(const pxr::GfVec3f& pos, float radius, const pxr::GfVec3f* normal, const pxr::GfVec3f* color)
{
  _positions.push_back(pos);
  _radius.push_back(radius);

  if(_haveNormals && normal)_normals.push_back(*normal);
  if(_haveColors && color)_colors.push_back(pos);
}

void 
Deformable::RemovePoint(size_t index)
{
  _positions.erase(_positions.begin() + index);
  if(_haveRadius)_radius.erase(_radius.begin() + index);
  if(_haveNormals)_normals.erase(_normals.begin() + index);
  if(_haveColors)_colors.erase(_colors.begin() + index);
}

void 
Deformable::RemoveAllPoints()
{
  _positions.clear();
  _radius.clear();
  _normals.clear();
  _colors.clear();
}

void
Deformable::Init(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  size_t numPoints = positions.size();
  _positions = positions;

}

void
Deformable::Init(const pxr::VtArray<pxr::GfVec3f>& positions, const pxr::VtArray<float>& radii)
{
  size_t numPoints = positions.size();
  _positions = positions;
  _haveRadius = true;
  _radius = radii;
}

void
Deformable::Init(const pxr::VtArray<pxr::GfVec3f>& positions, const pxr::VtArray<float>& radii,
                const pxr::VtArray<pxr::GfVec3f>& colors)
{
  size_t numPoints = positions.size();
  _positions = positions;
  _haveRadius = true;
  _radius = radii;
  _haveColors = true;
  _colors = colors;
}

void
Deformable::Init(const pxr::VtArray<pxr::GfVec3f>& positions, const pxr::VtArray<float>& radii,
                const pxr::VtArray<pxr::GfVec3f>& colors, const pxr::VtArray<pxr::GfVec3f>& normals)
{
  size_t numPoints = positions.size();
  _positions = positions;
  _haveRadius = true;
  _radius = radii;
  _haveColors = true;
  _colors = colors;
  _haveNormals = true;
  _normals = normals;
}

Point Deformable::Get(uint32_t index)
{
  if(index < _positions.size())
    return Point(index);   
  else
    return Point(); 
}

JVR_NAMESPACE_CLOSE_SCOPE