//======================================================
// POINT DECLARATION
//======================================================
#ifndef AMN_GEOMETRY_POINT_H
#define AMN_GEOMETRY_POINT_H

#include <algorithm>
#include <math.h>
#include <stdio.h>

#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/ray.h>
#include "../common.h"


AMN_NAMESPACE_OPEN_SCOPE

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
  void ClosestPoint(Geometry* geom, const pxr::GfVec3f& point , 
    pxr::GfVec3f& closest, double maxDistance=-1, double* minDistance=NULL);

};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_GEOMETRY_POINT_H