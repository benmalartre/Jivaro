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

  if(m)return m->TransformDir(result);
  else return result;
}

void
Location::TransformT(const pxr::GfMatrix4d &matrix)
{
  pxr::GfVec3d v(_coords[3], 0.0, 0.0);
  matrix.Transform(v);
  _coords[3] = v.GetLength();
}

PXR_NAMESPACE_CLOSE_SCOPE
