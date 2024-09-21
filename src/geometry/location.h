#ifndef JVR_GEOMETRY_LOCATION_H
#define JVR_GEOMETRY_LOCATION_H

#include <float.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec4d.h>
#include <pxr/base/gf/matrix4d.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Location {
public:
  static const size_t INVALID_INDEX = std::numeric_limits<size_t>::max();
  
  // Constructor
  Location() 
    : _geomId(INVALID_INDEX)
    , _compId(INVALID_INDEX)
    , _coords(pxr::GfVec4d(0.0, 0.0, 0.0, DBL_MAX)){};

  // Setters
  void Set(const Location& other);
  inline void SetGeometryIndex(int id) { _geomId = id; };
  inline void SetComponentIndex(int id) { _compId = id; };
  inline void SetCoordinates(const pxr::GfVec3d& coords) { 
    _coords[0] = coords[0]; 
    _coords[1] = coords[1];
    _coords[2] = coords[2];
  };
  inline void SetDistance(double t) { _coords[3] = t;};

  // Getters
  inline int GetGeometryIndex() const { return _geomId; };
  inline int GetComponentIndex() const { return _compId; };
  inline const pxr::GfVec3d GetCoordinates() const { 
    return pxr::GfVec3d(_coords[0], _coords[1], _coords[2]);};
  inline double GetDistance() const { return _coords[3]; };

  virtual pxr::GfVec3f ComputePosition(const pxr::GfVec3f* positions, 
    const int* elements, size_t sz, const pxr::GfMatrix4d* m=NULL) const;

  virtual pxr::GfVec3f ComputeNormal(const pxr::GfVec3f* positions, 
    const int* elements, size_t sz, const pxr::GfMatrix4d* m=NULL) const;
  
  inline bool IsValid() const { 
    return (_geomId != INVALID_INDEX) && (_compId != INVALID_INDEX); };

protected:
  size_t        _geomId;
  size_t        _compId;
  pxr::GfVec4d  _coords;
};

class ClosestPoint
{
public:
  static const size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

  // Constructor
  ClosestPoint()
    : _geomId(INVALID_INDEX)
    , _compId(INVALID_INDEX)
    , _coords(pxr::GfVec3d(0.0))
    , _point(pxr::GfVec3d(DBL_MAX)){};

  // Convert
  inline void ConvertToWorld(const pxr::GfMatrix4d &matrix, 
    const pxr::GfVec3f& query) {
    _point = matrix.Transform(_point);
  };

  inline void ConvertToLocal(const pxr::GfMatrix4d &invMatrix, 
    const pxr::GfVec3f& localQuery) {
    _point = invMatrix.Transform(_point);
  };

  // Setters
  void Set(const ClosestPoint& other);
  inline void SetGeometryIndex(int id) { _geomId = id; };
  inline void SetComponentIndex(int id) { _compId = id; };
  inline void SetCoordinates(const pxr::GfVec3d& coords) {_coords = coords;};
  inline void SetPoint(const pxr::GfVec3d& point) {_point = point;};

  // getters
  inline int GetGeometryIndex() const { return _geomId; };
  inline int GetComponentIndex() const { return _compId; };
  inline const pxr::GfVec3d& GetCoordinates(){return _coords;};
  inline const pxr::GfVec3d& GetPoint(){return _point;};

  inline bool IsValid() const { 
    return (_geomId != INVALID_INDEX) && (_compId != INVALID_INDEX); };

protected:
  size_t        _geomId;
  size_t        _compId;
  pxr::GfVec3d  _coords;
  pxr::GfVec3d  _point;
};



PXR_NAMESPACE_CLOSE_SCOPE

#endif //JVR_GEOMETRY_LOCATION_H