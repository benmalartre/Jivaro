// Points
//----------------------------------------------

#include <pxr/base/gf/ray.h>

#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/pointBased.h>
#include <pxr/usd/usdGeom/pointInstancer.h>

#include "../geometry/deformable.h"
#include "../geometry/utils.h"



JVR_NAMESPACE_OPEN_SCOPE

Deformable::Deformable(short type, const GfMatrix4d& matrix)
  : Geometry(type, matrix)
  , _haveWidths(false)
  , _haveNormals(false)
  , _haveColors(false)
{
}

Deformable::Deformable(const UsdPrim& prim, const GfMatrix4d& matrix)
  : Geometry(prim, matrix)
{
  if(prim.IsA<UsdGeomPointBased>()) {
    UsdGeomPointBased pointBased(prim);

    UsdAttribute pointsAttr = pointBased.GetPointsAttr();
    pointsAttr.Get(&_positions, UsdTimeCode::Default());
    _previous = _positions;
    _points.resize(_positions.size());
    size_t pointIdx = 0;
    for(auto& point: _points)point.id = pointIdx++;

    UsdAttribute normalsAttr = pointBased.GetNormalsAttr();
    if(normalsAttr.IsDefined() && normalsAttr.HasAuthoredValue()) {
      _haveNormals = true;
      normalsAttr.Get(&_normals, UsdTimeCode::Default());
    }
  }

  if(prim.IsA<UsdGeomPoints>()) {
    UsdGeomPoints points(prim);
    UsdAttribute widthsAttr = points.GetWidthsAttr();
      if(widthsAttr.IsDefined() && widthsAttr.HasAuthoredValue()) {
        _haveWidths = true;
        widthsAttr.Get(&_widths, UsdTimeCode::Default());
      }
  }
}

void Deformable::_ValidateNumPoints(size_t n)
{
  if (n != _positions.size()) {
    _positions.resize(n);
    _previous.resize(n);
    _points.resize(n);

    size_t pointIdx = 0;
    for(auto& point: _points)point.id = pointIdx++;
  }

  if(_haveNormals && n != _normals.size())_normals.resize(n);
  if(_haveWidths && n != _widths.size())_widths.resize(n);
  if (_haveColors && n != _colors.size())_colors.resize(n);

}

void
Deformable::SetPositions(const GfVec3f* positions, size_t n)
{
  _ValidateNumPoints(n);
  memcpy(&_previous[0], &_positions[0], n * sizeof(GfVec3f));
  memcpy(&_positions[0], positions, n * sizeof(GfVec3f));
}


void
Deformable::SetNormals(const GfVec3f* normals, size_t n)
{
  _haveNormals = true;
  _ValidateNumPoints(n);
  memcpy(&_normals[0], normals, n * sizeof(GfVec3f));
}

void
Deformable::SetWidths(const float* widths, size_t n)
{
  _haveWidths = true;
  _ValidateNumPoints(n);
  memcpy(&_widths[0], widths, n * sizeof(float));
}

void
Deformable::SetColors(const GfVec3f* colors, size_t n)
{
  _haveColors = true;
  _ValidateNumPoints(n);
  memcpy(&_colors[0], colors, n * sizeof(GfVec3f));
}

void
Deformable::SetPositions(const VtArray<GfVec3f>& positions)
{
  const size_t n = positions.size();
  _ValidateNumPoints(n);
  memcpy(&_previous[0], &_positions[0], n * sizeof(GfVec3f));
  memcpy(&_positions[0], &positions[0], n * sizeof(GfVec3f));
}

void
Deformable::SetNormals(const VtArray<GfVec3f>& normals)
{
  _haveNormals = true;
  const size_t n = normals.size();
  _ValidateNumPoints(n);
  memcpy(&_normals[0], &normals[0], n * sizeof(GfVec3f));
}

void
Deformable::SetWidths(const VtArray<float>& widths)
{
  _haveWidths = true;
  const size_t n = widths.size();
  _ValidateNumPoints(n);
  memcpy(&_widths[0], &widths[0], n * sizeof(float));
}

void
Deformable::SetColors(const VtArray<GfVec3f>& colors)
{
  _haveColors = true;
  const size_t n = colors.size();
  _ValidateNumPoints(n);
  memcpy(&_colors[0], &colors[0], n * sizeof(GfVec3f));
}

void
Deformable::Normalize()
{
  if (!_positions.size())return;

  // compute center of mass
  GfVec3f center(0.f);
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

GfVec3f 
Deformable::GetPrevious(uint32_t index) const
{
  return _previous[index];
}

GfVec3f 
Deformable::GetPosition(uint32_t index) const
{
  return _positions[index];
}

GfVec3f 
Deformable::GetNormal(uint32_t index) const
{
  return _normals[index];
}

GfVec3f
Deformable::GetColor(uint32_t index) const
{
  if (HaveColors())
    return _colors[index];
  else
    return _wirecolor;
}

GfVec3f
Deformable::GetVelocity(uint32_t index) const
{
  return _positions[index] - _previous[index];
}

float
Deformable::GetWidth(uint32_t index) const
{
  if(HaveWidths())
    return _widths[index];
  else
    return 0.01f;
}

void
Deformable::SetPrevious(uint32_t index, const GfVec3f& previous)
{
  _previous[index] = previous;
}

void
Deformable::SetPosition(uint32_t index, const GfVec3f& position)
{
  _positions[index] = position;
}

void 
Deformable::SetNormal(uint32_t index, const GfVec3f& normal)
{
  _normals[index] = normal;
}

void 
Deformable::SetWidth(uint32_t index, float width)
{
  _widths[index] = width;
}


void 
Deformable::ComputeBoundingBox()
{
  GfRange3d range;
  for (const auto& position : _positions)
    range.ExtendBy(position);
  
  range.SetMin(range.GetMin() - GfVec3f(FLT_EPSILON));
  range.SetMax(range.GetMax() + GfVec3f(FLT_EPSILON));
  _bbox.Set(range, _matrix);
   
}

void
Deformable::AddPoint(const GfVec3f& pos, float width, 
  const GfVec3f* normal, const GfVec3f* color)
{
  _previous.push_back(pos);
  _positions.push_back(pos);
  _widths.push_back(width);
  _points.push_back(_previous.size()-1);

  if(_haveNormals && normal)_normals.push_back(*normal);
  if(_haveColors && color)_colors.push_back(*color);
}

void 
Deformable::RemovePoint(size_t index)
{
  _positions.erase(_positions.begin() + index);
  _previous.erase(_previous.begin() + index);
  _points.erase(_points.begin() + index);
  if(_haveWidths)_widths.erase(_widths.begin() + index);
  if(_haveNormals)_normals.erase(_normals.begin() + index);
  if(_haveColors)_colors.erase(_colors.begin() + index);
}

void 
Deformable::RemovePoints(size_t start, size_t end)
{
  _positions.erase(_positions.begin() + start, _positions.begin() + end);
  _previous.erase(_previous.begin() + start, _previous.begin() + end);
  _points.erase(_points.begin() + start, _points.begin() + end);
  if(_haveWidths)_widths.erase(_widths.begin() + start, _widths.begin() + end);
  if(_haveNormals)_normals.erase(_normals.begin() + start, _normals.begin() + end);
  if(_haveColors)_colors.erase(_colors.begin() + start, _colors.begin() + end);
}

void 
Deformable::RemoveAllPoints()
{
  _positions.clear();
  _widths.clear();
  _normals.clear();
  _colors.clear();
}

Point Deformable::Get(uint32_t index)
{
  if(index < _points.size())
    return _points[index];   
  else
    return Point(); 
}

JVR_NAMESPACE_CLOSE_SCOPE