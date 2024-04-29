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
}

pxr::GfVec3f 
Location::ComputePosition(const pxr::GfVec3f* positions, const int* elements, size_t sz,
 const pxr::GfMatrix4d* m) const 
{
  pxr::GfVec3f result;
  if(elements)
    for(size_t d = 0; d < sz; ++d) 
      result += positions[elements[d]] * _coords[d];
  else 
    result += positions[_compId];


  return m ? m->Transform(result) : result;
}

pxr::GfVec3f
Location::ComputeNormal(const pxr::GfVec3f* normals, const int* elements, size_t sz, 
  const pxr::GfMatrix4d* m) const
{
  pxr::GfVec3f result;
  if (elements)
    for (size_t d = 0; d < sz; ++d)
      result += normals[elements[d]] * _coords[d];
  else
    result += normals[_compId];

  return m ? m->TransformDir(result) : result;
}

PXR_NAMESPACE_CLOSE_SCOPE
