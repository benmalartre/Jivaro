// Points
//----------------------------------------------
#include "../geometry/points.h"
#include "../geometry/utils.h"
#include <pxr/base/gf/ray.h>

JVR_NAMESPACE_OPEN_SCOPE

Points::~Points()
{
};

Points::Points()
  : Geometry(Geometry::POINT)
{
  _initialized = false;
  _numPoints = 0;
}

Points::Points(const Points* other, bool normalize)
  : Geometry(other, Geometry::POINT, normalize)
{
  _initialized = true;
  _numPoints = other->_numPoints;

  _normals = other->_normals;

  _radius.resize(_numPoints);
  memcpy(&_radius[0], &other->_radius[0], _numPoints * sizeof(float));
}

Points::Points(const pxr::UsdGeomPoints& points)
  : Geometry(Geometry::POINT)
{

  pxr::UsdAttribute pointsAttr = points.GetPointsAttr();
  pointsAttr.Get(&_points, pxr::UsdTimeCode::Default());
  _numPoints = _points.size();

  pxr::UsdAttribute normalsAttr = points.GetNormalsAttr();
  if (normalsAttr.IsDefined() && normalsAttr.HasAuthoredValue())
    normalsAttr.Get(&_normals, pxr::UsdTimeCode::Default());

  pxr::UsdAttribute widthsAttr = points.GetWidthsAttr();
  if (widthsAttr.IsDefined() && widthsAttr.HasAuthoredValue())
    widthsAttr.Get(&_radius, pxr::UsdTimeCode::Default());
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
  _points = positions;
  _normals = positions;
  _numPoints = _points.size();
}

void Points::Update(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _points = positions;
}

void Points::Update(const pxr::VtArray<float>& radius)
{
  _radius = radius;
}

void Points::Update(const pxr::VtArray<pxr::GfVec3f>& positions,
  const pxr::VtArray<float>& radius)
{
  _points = positions;
  _radius = radius;
}

Point Points::Get(uint32_t index)
{
  if(index < _points.size())
    return Point(index, index < _radius.size() ? _radius[index] : 1.f);   
  else
    return Point(); 
}

JVR_NAMESPACE_CLOSE_SCOPE