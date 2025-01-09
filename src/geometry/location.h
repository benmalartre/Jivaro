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
    , _coords(GfVec4d(0.0, 0.0, 0.0, DBL_MAX))
    , _point(GfVec3d(DBL_MAX)){};

  // Convert
  inline void ConvertToWorld(const GfMatrix4d &matrix) {
    _point = matrix.Transform(_point);
  };

  inline void ConvertToLocal(const GfMatrix4d &invMatrix) {
    _point = invMatrix.Transform(_point);
  };

  // Setters
  void Set(const Location& other);
  inline void SetGeometryIndex(int id) { _geomId = id; };
  inline void SetComponentIndex(int id) { _compId = id; };
  virtual inline void SetCoordinates(const GfVec3d& coords) { 
    _coords[0] = coords[0]; 
    _coords[1] = coords[1];
    _coords[2] = coords[2];
  };
  inline void SetPoint(const GfVec3d& point) {_point = point;};
  inline void SetDistance(double t) { _coords[3] = t;};

  // Getters
  inline int GetGeometryIndex() const { return _geomId; };
  inline int GetComponentIndex() const { return _compId; };
  inline const GfVec3d GetCoordinates() const { 
    return GfVec3d(_coords[0], _coords[1], _coords[2]);};
  inline const GfVec3d& GetPoint(){return _point;};
  inline double GetDistance() const { return _coords[3]; };

  virtual GfVec3f ComputePosition(const GfVec3f* positions, 
    const int* elements, size_t sz, const GfMatrix4d* m=NULL) const;

  virtual GfVec3f ComputeNormal(const GfVec3f* normals, 
    const int* elements, size_t sz, const GfMatrix4d* m=NULL) const;

  virtual GfVec3f ComputeVelocity(const GfVec3f* positions, const GfVec3f* previous, 
    const int* elements, size_t sz, const GfMatrix4d* m=NULL) const;

  template<typename T>
  T ComputeValue(const T* values, const int* elements, size_t sz) const;
  
  inline bool IsValid() const { 
    return (_geomId != INVALID_INDEX) && (_compId != INVALID_INDEX); };

protected:
  size_t        _geomId;
  size_t        _compId;
  GfVec4d  _coords;
  GfVec3d  _point;
};

template<typename T>
T Location::ComputeValue(const T* values, const int* elements, size_t sz) const
{
  T result(0);
  if (elements)
    for (size_t d = 0; d < sz; ++d)
      result += values[elements[d]] * _coords[d];
  else
    result += values[_compId];

  return result;
}

PXR_NAMESPACE_CLOSE_SCOPE

#endif //JVR_GEOMETRY_LOCATION_H