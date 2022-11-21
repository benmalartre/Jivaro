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
struct Point {
  uint32_t id;    
  float radius;

  void GetPosition(Geometry* geom, pxr::GfVec3f& center);
  void GetNormal(Geometry* geom, pxr::GfVec3f& normal);
  void Raycast(Geometry* geom, const pxr::GfRay& point,
    pxr::GfVec3f& closest, double maxDistance = -1, double* minDistance = NULL);

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_POINT_H