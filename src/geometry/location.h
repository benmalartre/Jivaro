#ifndef JVR_GEOMETRY_LOCATION_H
#define JVR_GEOMETRY_LOCATION_H

#include <float.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec4d.h>
#include <pxr/base/gf/matrix4d.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Intersector;

class Location{

public:
  static const size_t INVALID_INDEX = std::numeric_limits<size_t>::max();
  
  // Constructors
  Location() 
    : _geomId(INVALID_INDEX)
    , _compId(INVALID_INDEX)
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
  void SetCoordinates(const pxr::GfVec3d& coords) { 
    _coords[0] = coords[0]; 
    _coords[1] = coords[1];
    _coords[2] = coords[2];
  };
  void SetT(double t) { _coords[3] = t;};
  void TransformT(const pxr::GfMatrix4d& matrix);

  // Getters
  int GetGeometryIndex() const { return _geomId; };
  int GetComponentIndex() const { return _compId; };
  const pxr::GfVec3d GetCoordinates() const { 
    return pxr::GfVec3d(_coords[0], _coords[1], _coords[2]);};
  double GetT() const { return _coords[3]; };

  virtual pxr::GfVec3f ComputePosition(const pxr::GfVec3f* positions, 
    const int* elements, size_t sz, const pxr::GfMatrix4d* m=NULL) const;

  virtual pxr::GfVec3f ComputeNormal(const pxr::GfVec3f* positions, 
    const int* elements, size_t sz, const pxr::GfMatrix4d* m=NULL) const;
  
  inline bool IsValid() const { 
    return (_geomId != INVALID_INDEX) && (_compId != INVALID_INDEX); };

protected:
  int           _geomId;
  int           _compId;
  pxr::GfVec4d  _coords;
};



PXR_NAMESPACE_CLOSE_SCOPE

#endif //JVR_GEOMETRY_LOCATION_H