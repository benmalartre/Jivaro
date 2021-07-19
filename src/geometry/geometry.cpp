#include <pxr/base/gf/ray.h>

#include "../geometry/geometry.h"
#include "../geometry/utils.h"


AMN_NAMESPACE_OPEN_SCOPE


Geometry::~Geometry()
{
};

Geometry::Geometry()
{
  _initialized = false;
  _numPoints = 0;
  _type = POINT;
}

Geometry::Geometry(const Geometry* other, bool normalize)
{
  _initialized = true;
  _numPoints = other->_numPoints;
  _type = POINT;

  _position = other->_position;
  _normal = other->_position;

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
      _position[v]  = other->GetPosition(v) - center;
    }

    // determine radius
    float rMax = 0;
    for (size_t v = 0; v < other->_numPoints; ++v) {
      rMax = std::max(rMax, _position[v].GetLength());
    }

    // rescale to unit sphere
    float invRMax = 1.f / rMax;
    for (size_t v = 0; v < other->_numPoints; ++v) {
      _position[v] *= invRMax;
    }
  }
}

pxr::GfVec3f Geometry::GetPosition(uint32_t index) const
{
  return _position[index];
}

pxr::GfVec3f Geometry::GetNormal(uint32_t index) const
{
  return _normal[index];
}


void Geometry::ComputeBoundingBox()
{
  /*
  SubMesh* subMesh;
  _bbox.clear();
  for(uint32_t i=0;i<getNumMeshes();i++)
  {
    subMesh = getSubMesh(i);
    subMesh->_bbox.compute(subMesh);
    _bbox.addInPlace(subMesh->_bbox);
  }
  */
}

void Geometry::Init(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _position = positions;
  _normal = positions;
  _numPoints = _position.size();
}

void Geometry::Update(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _position = positions;
}

AMN_NAMESPACE_CLOSE_SCOPE