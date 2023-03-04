//======================================================
// POINT DECLARATION
//======================================================
#ifndef JVR_GEOMETRY_POINT_H
#define JVR_GEOMETRY_POINT_H

#include "../common.h"
#include "../geometry/component.h"


JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
class Hit;
struct Point : public Component {

  Point()
    : Component()
    , radius(1.f) {};
  Point(uint32_t index, float radius = 1.f)
    : Component(index)
    , radius(radius) {};

  float radius;

  float GetRadius(){return radius;};
  pxr::GfVec3f GetPosition(Geometry* geom);
  pxr::GfVec3f GetNormal(Geometry* geom);
  
  bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Touch(const pxr::GfVec3f* points, 
    const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize) const override;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_POINT_H