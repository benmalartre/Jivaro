//======================================================
// POINT DECLARATION
//======================================================
#ifndef JVR_GEOMETRY_POINT_H
#define JVR_GEOMETRY_POINT_H

#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <limits>

#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/ray.h>
#include "../common.h"


JVR_NAMESPACE_OPEN_SCOPE

#define INVALID_POINT_ID std::numeric_limits<uint32_t>::max()

class Geometry;
class Points;
class Mesh;
class Hit;
struct Point {
  uint32_t id;    
  float radius;

  uint32_t GetIndex(){return id;};
  float GetRadius(){return radius;};
  pxr::GfVec3f GetPosition(Geometry* geom);
  pxr::GfVec3f GetNormal(Geometry* geom);
  
  bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray,
    pxr::GfVec3f& closest, double maxDistance = -1, double* minDistance = NULL);
  bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point,
    pxr::GfVec3f& closest, double maxDistance=-1.f);

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_POINT_H