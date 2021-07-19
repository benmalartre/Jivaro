#ifndef AMN_ACCELERATION_INTERSECTOR_H
#define AMN_ACCELERATION_INTERSECTOR_H

#include <vector>
#include <limits>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/range3f.h>
#include "../geometry/intersection.h"

AMN_NAMESPACE_OPEN_SCOPE

class Geometry;

class Intersector
{ 
public:
  virtual void Init(const std::vector<Geometry*>& geometries) = 0;
  virtual void Update(const std::vector<Geometry*>& geometries) = 0;
  virtual bool Raycast(const pxr::GfRay& ray, Hit* hit, 
    double maxDistance=-1, double* minDistance=NULL) const = 0;
  virtual bool Closest(const pxr::GfVec3f& point, Hit* hit, 
    double maxDistance=-1.f, double* minDistance=NULL) const = 0;

protected:
  std::vector<Geometry*>  _geometries;
}; 

AMN_NAMESPACE_CLOSE_SCOPE
#endif // AMN_ACCELERATION_INTERSECTOR_H