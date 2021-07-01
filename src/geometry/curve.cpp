// Curve
//----------------------------------------------
#include "curve.h"
#include "utils.h"
#include <pxr/base/gf/ray.h>

AMN_NAMESPACE_OPEN_SCOPE

Curve::~Curve()
{
};

Curve::Curve()
  : Geometry()
{
  _initialized = false;
  _numCurves = 0;
  _numSegments = 0;
  _type = CURVE;
}

Curve::Curve(const Curve* other, bool normalize)
  : Geometry(other, normalize)
{
  _initialized = true;
  _numCurves = other->_numCurves;
  _numSegments = other->_numSegments;
  _type = CURVE;

  _normal = other->_normal;

  _cvCounts.resize(_numCurves);
  memcpy(&_cvCounts[0], &other->_cvCounts[0], _numCurves * sizeof(int));
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

void Curve::Init(
  const pxr::VtArray<pxr::GfVec3f>& positions, 
  const pxr::VtArray<int>& counts)
{
  _cvCounts = counts;
  _numCurves = counts.size();
  _numSegments = 0;
  for(const auto& count: counts) _numSegments += count - 1;
  _position = positions;
  _normal = positions;
  _numPoints = _position.size();
}

void Curve::Update(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _position = positions;
}

bool Curve::ClosestIntersection(const pxr::GfVec3f& origin, 
  const pxr::GfVec3f& direction, CurveLocation& location, float maxDistance)
{
  return false;
}

bool Curve::ClosestPoint(const pxr::GfVec3f& point, 
  CurveLocation& location, float maxDistance)
{
  return false;
}


AMN_NAMESPACE_CLOSE_SCOPE