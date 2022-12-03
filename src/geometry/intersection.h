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

class Hit {
public:
  enum ElementType {
    POINT,
    EDGE,
    TRIANGLE,
    GEOMETRY
  };

private:
  friend class Intersector;
  Geometry*     _geom;
  pxr::GfVec3f  _baryCoords;
  short         _elemType;
  int           _elemId;
  float         _t;

public:
  // Constructors
  Hit() 
    : _geom(NULL)
    , _baryCoords(pxr::GfVec3f(0.f))
    , _elemType(GEOMETRY)
    , _elemId(-1)
    , _t(-1.f) {};

  Hit(const Hit& other)
    : _geom(other._geom)
    , _baryCoords(other._baryCoords)
    , _elemType(other._elemType)
    , _elemId(other._elemId)
    , _t(other._t) {};

  Hit(Geometry* geom, const pxr::GfVec3f& baryCoords, short elemType, int elemId, int elemMapId, float t)
    : _geom(geom)
    , _baryCoords(baryCoords)
    , _elemType(elemType)
    , _elemId(elemId)
    , _t(t) {};

  // Setters
  void Set(const Hit& other);
  void SetGeometry(Geometry* geom) { _geom = geom; };
  void SetElementType(short type) { _elemType = type; };
  void SetElementIndex(int id) { _elemId = id; };
  void SetBarycentricCoordinates(const pxr::GfVec3f& coords) { _baryCoords = coords; };
  void SetT(float t) {_t = t;};

  // Getters
  Geometry* GetGeometry() { return _geom; };
  short GetElementType() { return _elemType; };
  int GetElementIndex() { return _elemId; };
  const pxr::GfVec3f& GetBarycentricCoordinates() { return _baryCoords; };
  float GetT() { return _t; };
  void GetPosition(pxr::GfVec3f* position) const;
  void GetPosition(const pxr::GfRay& ray, pxr::GfVec3f* position) const;
  void GetNormal(pxr::GfVec3f* normal) const;
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