//======================================================
// POINT DECLARATION
//======================================================
#ifndef JVR_GEOMETRY_POINT_H
#define JVR_GEOMETRY_POINT_H

#include "../geometry/component.h"


JVR_NAMESPACE_OPEN_SCOPE

class Deformable;
class Location;
struct Point : public Component {

  Point()
    : Component(){};
  Point(uint32_t index)
    : Component(index){};

  float GetWidth(Deformable* geom);
  pxr::GfVec3f GetPosition(Deformable* geom);
  pxr::GfVec3f GetNormal(Deformable* geom);
  
  virtual bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Location* hit) const override;
  virtual bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Location* hit) const override;
  virtual bool Touch(const pxr::GfVec3f* points, const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize) const override;

  virtual pxr::GfRange3f GetBoundingBox(const pxr::GfVec3f* positions, const pxr::GfMatrix4d& m) const override;
  virtual short GetType() const override {return Component::POINT;};
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_POINT_H