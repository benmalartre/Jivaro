#include "../geometry/location.h"


JVR_NAMESPACE_OPEN_SCOPE

//=================================================================================================
// LOCATION CLASS
//=================================================================================================
void 
Location::Set(const Location& other) {
  _geomId = other._geomId;
  _compId = other._compId;
  _coords = other._coords;
  _point  = other._point;
}

pxr::GfVec3f 
Location::ComputePosition(const pxr::GfVec3f* positions, const int* elements, size_t sz,
 const pxr::GfMatrix4d* m) const 
{
  pxr::GfVec3f result(0.f);
  if(elements)
    for(size_t d = 0; d < sz; ++d) 
      result += positions[elements[d]] * _coords[d];
  else 
    result += positions[_compId];

  if(m)return m->Transform(result);
  else return result;
}

pxr::GfVec3f
Location::ComputeNormal(const pxr::GfVec3f* normals, const int* elements, size_t sz, 
  const pxr::GfMatrix4d* m) const
{
  pxr::GfVec3f result(0.f);
  if (elements)
    for (size_t d = 0; d < sz; ++d)
      result += normals[elements[d]] * _coords[d];
  else
    result += normals[_compId];

  if(m)return m->TransformDir(result).GetNormalized();
  else return result.GetNormalized();
}

pxr::GfVec3f
Location::ComputeVelocity(const pxr::GfVec3f* positions, const pxr::GfVec3f* previous,
  const int* elements, size_t sz, const pxr::GfMatrix4d* m) const
{
  pxr::GfVec3f result(0.f);
  if (elements)
    for (size_t d = 0; d < sz; ++d)
      result += (positions[elements[d]] - previous[elements[d]]) * _coords[d];
  else
    result += positions[_compId] - previous[_compId];

  if(m)return m->TransformDir(result);
  else return result;
}

template<typename T>
T Location::ComputeValue(const T* values, const int* elements, size_t sz) const
{
  T result;;
  if (elements)
    for (size_t d = 0; d < sz; ++d)
      result += values[elements[d]] * _coords[d];
  else
    result += value[_compId];

  return result;
}

PXR_NAMESPACE_CLOSE_SCOPE
