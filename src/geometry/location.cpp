#include "../geometry/location.h"


JVR_NAMESPACE_OPEN_SCOPE

//=================================================================================================
// LOCATION CLASS
//=================================================================================================
void 
Location::Set(const Location& other) {
  _geomId = other._geomId;
  _elemId = other._elemId;
  _coords = other._coords;
}

pxr::GfVec3f 
Location::GetPosition(const pxr::GfVec3f* positions, int* elements, size_t sz,
 const pxr::GfMatrix4d& m) const 
{
  pxr::GfVec3f result;
  if(elements) {
    for(size_t d = 0; d < sz; ++d) 
      result += positions[elements[d]] * _coords[d];
  else 
    result += positions[_elemIdx];

  return m.Transform(result);
}

pxr::GfVec3f
GetNormal(const pxr::GfVec3f* normals, int* elements, size_t sz, 
  const pxr::GfMatrix4d& m) const
{
  pxr::GfVec3f result;
  if(elements) {
    for(size_t d = 0; d < sz; ++d) 
      result += normals[elements[_elemIdx * dim + d]] * _coords[d];
  else 
    result += normals[_elemIdx];

  return m.TransformDir(result);
}

PXR_NAMESPACE_CLOSE_SCOPE
