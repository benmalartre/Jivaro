#include <pxr/base/gf/ray.h>

#include "../geometry/geometry.h"
#include "../geometry/utils.h"


JVR_NAMESPACE_OPEN_SCOPE


Geometry::~Geometry()
{
};

Geometry::Geometry(short type)
{
  _initialized = false;
  _numPoints = 0;
  _type = type;
}

Geometry::Geometry(const Geometry* other, short type, bool normalize)
{
  _initialized = true;
  _numPoints = other->_numPoints;
  _type = type;

  _positions = other->_positions;
  _normals = other->_positions;

  _bbox = other->_bbox;

  if (normalize) {
    // compute center of mass
    pxr::GfVec3f center(0.f);
    for (size_t v = 0; v < other->_numPoints; ++v) {
      center += other->GetPosition(v);
    }
    
    center *= 1.0 / (float)_numPoints;

    // translate to origin
    for (size_t v = 0; v < other->_numPoints; ++v) {
      _positions[v]  = other->GetPosition(v) - center;
    }

    // determine radius
    float rMax = 0;
    for (size_t v = 0; v < other->_numPoints; ++v) {
      rMax = std::max(rMax, _positions[v].GetLength());
    }

    // rescale to unit sphere
    float invRMax = 1.f / rMax;
    for (size_t v = 0; v < other->_numPoints; ++v) {
      _positions[v] *= invRMax;
    }
  }
}

pxr::GfVec3f Geometry::GetPosition(uint32_t index) const
{
  return _positions[index];
}

pxr::GfVec3f Geometry::GetNormal(uint32_t index) const
{
  return _normals[index];
}

float Geometry::GetRadius(uint32_t index) const
{
  return _radius[index];
}

void Geometry::SetPosition(uint32_t index, const pxr::GfVec3f& position)
{
  _positions[index] = position;
}

void Geometry::SetNormal(uint32_t index, const pxr::GfVec3f& normal)
{
  _normals[index] = normal;
}

void Geometry::SetRadius(uint32_t index, float radius)
{
  _radius[index] = radius;
}


void Geometry::ComputeBoundingBox()
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
  _bbox.Set(range, pxr::GfMatrix4d(1.0));
   
}

void Geometry::Init(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _positions = positions;
  _normals = positions;
  _numPoints = _positions.size();
}

void Geometry::Update(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _positions = positions;
}

void Geometry::SetPositions(pxr::GfVec3f* positions, size_t n)
{
  if (n != _positions.size()) {
    _positions.resize(n);
  }
  memmove(&_positions[0], positions, n * sizeof(pxr::GfVec3f));
}

JVR_NAMESPACE_CLOSE_SCOPE