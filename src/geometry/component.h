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
#include <pxr/base/gf/range3f.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
class Location;

struct Component {
  static const int INVALID_INDEX = std::numeric_limits<int>::max();
  enum Type {
    POINT,
    EDGE,
    TRIANGLE,
    TRIANGLEPAIR,
    POLYGON
  };

  int id;

  Component() : id(INVALID_INDEX) {};
  Component(int index) : id(index) {};
  int GetIndex(){return id;};

  virtual bool Touch(const pxr::GfVec3f* points, const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize) const = 0;
  virtual bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Location* hit) const = 0;
  virtual bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Location* hit) const = 0;

  virtual pxr::GfRange3f GetBoundingBox(const pxr::GfVec3f* positions, const pxr::GfMatrix4d& m) const = 0;
  virtual short GetType() const = 0;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_COMPONENT_H