//======================================================
// COMPONENT DECLARATION
//======================================================
#ifndef JVR_GEOMETRY_COMPONENT_H
#define JVR_GEOMETRY_COMPONENT_H

#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <limits>

#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/ray.h>
#include "../common.h"

#define INVALID_POINT_ID std::numeric_limits<uint32_t>::max()


JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
class Hit;
struct Component {

  uint32_t id;

  Component() : id(INVALID_POINT_ID) {};
  Component(uint32_t index) : id(index) {};
  uint32_t GetIndex(){return id;};

  virtual bool Touch(const pxr::GfVec3f* points, 
    const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize) const = 0;
  virtual bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const = 0;
  virtual bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const = 0;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_POINT_H