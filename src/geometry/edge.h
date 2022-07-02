//======================================================
// POINT DECLARATION
//======================================================
#ifndef JVR_GEOMETRY_POINT_H
#define JVR_GEOMETRY_POINT_H

#include <algorithm>
#include <math.h>
#include <stdio.h>

#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/ray.h>
#include "../common.h"


JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
struct Edge {
  pxr::GfVec2i vertices;    
  float radius;

  void GetCenter(Geometry* geom, pxr::GfVec3f& center);
  void GetPosition(Geometry* geom, pxr::GfVec3f& center, short idx);
  void GetNormal(Geometry* geom, pxr::GfVec3f& normal);
  void Raycast(Geometry* geom, const pxr::GfRay& point , 
    pxr::GfVec3f& closest, double maxDistance=-1, double* minDistance=NULL);
  bool Intersect(const Edge& other, float epsilon=0.0001);

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_POINT_H