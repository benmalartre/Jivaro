//======================================================
// POINT DECLARATION
//======================================================
#ifndef JVR_GEOMETRY_POINT_H
#define JVR_GEOMETRY_POINT_H

#include "../common.h"
#include "../geometry/component.h"


JVR_NAMESPACE_OPEN_SCOPE

class Deformable;
class Location;
struct Point : public Component {

  Point()
    : Component(){};
  Point(uint32_t index)
    : Component(index){};

  float GetRadius(Deformable* geom);
  pxr::GfVec3f GetPosition(Deformable* geom);
  pxr::GfVec3f GetNormal(Deformable* geom);
  
  bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Touch(const pxr::GfVec3f* points, 
    const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize) const override;

  pxr::GfRange3f GetBoundingBox(const pxr::GfVec3f* points) const override;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_POINT_H