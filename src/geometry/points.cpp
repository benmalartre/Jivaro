// Points
//----------------------------------------------
#include "points.h"
#include "utils.h"
#include <pxr/base/gf/ray.h>

AMN_NAMESPACE_OPEN_SCOPE

Points::~Points()
{
};

Points::Points()
  : Geometry()
{
  _initialized = false;
  _numPoints = 0;
  _type = POINT;
}

Points::Points(const Points* other, bool normalize)
  : Geometry(other, normalize)
{
  _initialized = true;
  _numPoints = other->_numPoints;
  _type = POINT;

  _normal = other->_normal;

  _radius.resize(_numPoints);
  memcpy(&_radius[0], &other->_radius[0], _numPoints * sizeof(float));
}

void Points::SetDisplayColor(GeomInterpolation interp, 
  const pxr::VtArray<pxr::GfVec3f>& colors) 
{
  _colorsInterpolation = interp;
  _colors = colors;
}


void Points::Init(
  const pxr::VtArray<pxr::GfVec3f>& positions, 
  const pxr::VtArray<float>& radius)
{
  _radius = radius;
  _position = positions;
  _normal = positions;
  _numPoints = _position.size();
}

void Points::Update(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _position = positions;
}

void Points::Update(const pxr::VtArray<float>& radius)
{
  _radius = radius;
}

void Points::Update(const pxr::VtArray<pxr::GfVec3f>& positions,
  const pxr::VtArray<float>& radius)
{
  _position = positions;
  _radius = radius;
}

AMN_NAMESPACE_CLOSE_SCOPE