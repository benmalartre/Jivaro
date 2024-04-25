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

public:
  static const int INVALID_INDEX = -1;
  static const int AWAITING_INDEX = INT_MAX;
  
  // Constructors
  Location() 
    : _geomId(-1)
    , _compId(-1)
    , _coords(pxr::GfVec4f(0.f, 0.f, 0.f, FLT_MAX)) {};

  Location(const Location& other)
    : _geomId(other._geomId)
    , _compId(other._compId)
    , _coords(other._coords) {};

  Location(int geomId, int compId, const pxr::GfVec4f& coords)
    : _geomId(geomId)
    , _compId(compId)
    , _coords(coords) {};

  // Setters
  void Set(const Location& other);
  void SetGeometryIndex(int id) { _geomId = id; };
  void SetComponentIndex(int id) { _compId = id; };
  void SetCoordinates(const pxr::GfVec3f& coords) { 
    _coords[0] = coords[0]; 
    _coords[1] = coords[1];
    _coords[2] = coords[2];
  };
  void SetT(float t) { _coords[3] = t;};

  // Getters
  int GetGeometryIndex() const { return _geomId; };
  int GetComponentIndex() const { return _compId; };
  const pxr::GfVec3f GetCoordinates() const { return pxr::GfVec3f(_coords[0], _coords[1], _coords[2]);};
  float GetT() const { return _coords[3]; };

  virtual pxr::GfVec3f ComputePosition(const pxr::GfVec3f* positions, const int* elements, size_t sz,
    const pxr::GfMatrix4d&) const;

  virtual pxr::GfVec3f ComputeNormal(const pxr::GfVec3f* positions, const int* elements, size_t sz,
    const pxr::GfMatrix4d&) const;
  
  bool IsValid() const { return _geomId != INVALID_INDEX && _compId != INVALID_INDEX; };

protected:
  friend class Intersector;
  int           _geomId;
  int           _compId;
  pxr::GfVec4f  _coords;
};



PXR_NAMESPACE_CLOSE_SCOPE

#endif //JVR_GEOMETRY_LOCATION_H