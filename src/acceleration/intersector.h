#ifndef JVR_ACCELERATION_INTERSECTOR_H
#define JVR_ACCELERATION_INTERSECTOR_H

#include <vector>
#include <limits>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/range3f.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/bbox3d.h>
#include "../geometry/intersection.h"

JVR_NAMESPACE_OPEN_SCOPE

class Component;

class Intersector : public pxr::GfRange3d
{ 
public:
  enum ElementType {
    POINT,
    EDGE,
    TRIANGLE,
    POLYGON,
    INVALID
  };

public:
  virtual void Init(const std::vector<Geometry*>& geometries) {};
  virtual void Update(const std::vector<Geometry*>& geometries) {};
  virtual bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
    double maxDistance=-1, double* minDistance=NULL) const = 0;
  virtual bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
    double maxDistance=-1.f) const = 0;
}; 

JVR_NAMESPACE_CLOSE_SCOPE
#endif // JVR_ACCELERATION_INTERSECTOR_H