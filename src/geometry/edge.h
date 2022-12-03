//======================================================
// EDGE DECLARATION
//======================================================
#ifndef JVR_GEOMETRY_EDGE_H
#define JVR_GEOMETRY_EDGE_H

#include <algorithm>
#include <math.h>
#include <stdio.h>

#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/ray.h>
#include "../common.h"


JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
class Hit;
struct Edge {
  uint32_t id;
  pxr::GfVec2i vertices;    
  float radius;

  uint32_t GetIndex(){return id;};
  uint32_t GetStartIndex(){return vertices[0];};
  uint32_t GetEndIndex(){return vertices[1];};
  float GetRadius(){return radius;};
  pxr::GfVec3f GetCenter(Geometry* geom);
  pxr::GfVec3f GetPosition(Geometry* geom, short idx);
  pxr::GfVec3f GetNormal(Geometry* geom);

  bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const;
  bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const;

  bool Intersect(const Edge& other, float epsilon=0.0001);

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_POINT_H