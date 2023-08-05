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

class Tracer {
public:
  enum ElementType {
    POINT,
    EDGE,
    TRIANGLE,
    GEOMETRY
  };

  void SetGeometry(Geometry* geom) { _geometry = geom; };
  void SetElementType(short type) { _elemType = type; };

  Geometry* GetGeometry() { return _geometry; };
  short GetElementType() { return _elemType; };

private:
  friend class Intersector;
  Geometry*     _geometry;
  short         _elemType;
};

class Hit {
private:
  friend class Intersector;
  int           _geomId;
  int           _elemId;
  pxr::GfVec4f  _coords;

public:
  // Constructors
  Hit() 
    : _geomId(-1)
    , _elemId(-1)
    , _coords(pxr::GfVec4f(0.f, 0.f, 0.f, FLT_MAX)) {};

  Hit(const Hit& other)
    : _geomId(other._geomId)
    , _elemId(other._elemId)
    , _coords(other._coords) {};

  Hit(int geomId, int elemId, const pxr::GfVec4f& coords)
    : _geomId(geomId)
    , _elemId(elemId)
    , _coords(coords) {};

  // Setters
  void Set(const Hit& other);
  void SetGeometryIndex(int id) { _geomId = id; };
  void SetElementIndex(int id) { _elemId = id; };
  void SetBarycentricCoordinates(const pxr::GfVec3f& coords) { 
    _coords[0] = coords[0]; 
    _coords[1] = coords[1];
    _coords[2] = coords[2];
  };
  void SetT(float t) { _coords[3] = t;};

  // Getters
  int GetGeometryIndex() { return _geomId; };
  int GetElementIndex() { return _elemId; };
  const pxr::GfVec4f& GetBarycentricCoordinates() { return _coords; };
  float GetT() { return _coords[3]; };
  pxr::GfVec3f GetPosition(Geometry* geometry) const;
  pxr::GfVec3f GetPosition(const pxr::GfRay& ray) const;
  pxr::GfVec3f GetNormal(Geometry* geometry) const;
  bool HasHit() { return _elemId >= 0; };
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