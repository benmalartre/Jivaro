#ifndef JVR_GEOMETRY_LOCATION_H
#define JVR_GEOMETRY_LOCATION_H

#include <pxr/base/gf/plane.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/plane.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Intersector;

class Location{
protected:
  friend class Intersector;
  int           _geomId;
  int           _elemId;
  pxr::GfVec4f  _coords;

public:
  // Constructors
  Location() 
    : _geomId(-1)
    , _elemId(-1)
    , _coords(pxr::GfVec4f(0.f, 0.f, 0.f, FLT_MAX)) {};

  Location(const Location& other)
    : _geomId(other._geomId)
    , _elemId(other._elemId)
    , _coords(other._coords) {};

  Location(int geomId, int elemId, const pxr::GfVec4f& coords)
    : _geomId(geomId)
    , _elemId(elemId)
    , _coords(coords) {};

  // Setters
  void Set(const Location& other);
  void SetGeometryIndex(int id) { _geomId = id; };
  void SetElementIndex(int id) { _elemId = id; };
  void SetCoordinates(const pxr::GfVec3f& coords) { 
    _coords[0] = coords[0]; 
    _coords[1] = coords[1];
    _coords[2] = coords[2];
  };
  void SetT(float t) { _coords[3] = t;};

  // Getters
  int GetGeometryIndex() const { return _geomId; };
  int GetElementIndex() const { return _elemId; };
  const pxr::GfVec3f& GetCoordinates() const { return *(pxr::GfVec3f*)&_coords;};
  float GetT() const { return _coords[3]; };

  virtual pxr::GfVec3f ComputePosition(const pxr::GfVec3f* positions, int* elements, size_t sz,
    const pxr::GfMatrix4d&) const;

  virtual pxr::GfVec3f ComputeNormal(const pxr::GfVec3f* positions, int* elements, size_t sz,
    const pxr::GfMatrix4d&) const;
  
  bool IsValid() const { return _geomId >= 0 && _elemId >= 0; };
};



PXR_NAMESPACE_CLOSE_SCOPE

#endif //JVR_GEOMETRY_LOCATION_H