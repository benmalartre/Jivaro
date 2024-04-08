#ifndef JVR_GEOMETRY_INTERSECTION_H
#define JVR_GEOMETRY_INTERSECTION_H

#include <pxr/base/gf/plane.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/ray.h>
#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Intersector;
class Geometry;

class Location{
private:
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
  void SetCoordinates(const pxr::GfVec4f& coords) { 
    _coords = coords;
  };
  void SetT(float t) { _coords[3] = t;};

  // Getters
  int GetGeometryIndex() const { return _geomId; };
  int GetElementIndex() const { return _elemId; };
  const pxr::GfVec4f& GetBarycentricCoordinates() const {
    return _coords; };
  const pxr::GfVec3f& GetPointCoordinates() const { 
    return *(pxr::GfVec3f*)&_coords;};

  float GetT() const { return _coords[3]; };
  pxr::GfVec3f GetPosition(Geometry* geometry, bool worldSpace=true) const;
  pxr::GfVec3f GetPosition(const pxr::GfRay& ray, bool worldSpace=true) const;
  pxr::GfVec3f GetNormal(Geometry* geometry, bool worldSpace=true) const;
  bool IsValid() const { return _geomId >= 0 && _elemId >= 0; };
};

static pxr::GfPlane DEFAULT_PLANE(pxr::GfVec3d(0, 1, 0), pxr::GfVec3d(0));

bool IntersectDisc(const pxr::GfRay& localRay, const double radius,
  double* distance);

bool IntersectRing(const pxr::GfRay& localRay, const double radius,
  const double section, double* distance);

bool IntersectCylinder(const pxr::GfRay& localRay, const double radius, 
  const double height, double* distance);

bool IntersectTube(const pxr::GfRay& localRay, const double innerRadius,
  const double outerRadius, const double height, double* distance);

bool IntersectTorus( const pxr::GfRay& localRay, const double radius, 
  const double section, double* distance);

bool IntersectTorusApprox(const pxr::GfRay& localRay, const double radius,
  const double section, double* distance);

bool IntersectTriangle(const pxr::GfRay& ray, const pxr::GfVec3f& a,
  const pxr::GfVec3f& b, const pxr::GfVec3f& c, double* distance, pxr::GfVec3f* uvw);

PXR_NAMESPACE_CLOSE_SCOPE

#endif //JVR_GEOMETRY_INTERSECTION_H