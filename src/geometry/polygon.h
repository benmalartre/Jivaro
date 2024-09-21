//======================================================
// POLYGON COMPONENT
//======================================================
#ifndef JVR_GEOMETRY_POLYGON_H
#define JVR_GEOMETRY_POLYGON_H

#include "../geometry/component.h"
#include "../geometry/triangle.h


JVR_NAMESPACE_OPEN_SCOPE

class Deformable;
class Location;
struct Polygon : public Component {

  Polygon()
    : Component() {};
  Polygon(uint32_t index, const pxr::VtArray<size_t>& vertices)
    : Component(index)
    , vertices(vertices){};

  std::vector<size_t> vertices;    

  pxr::GfVec3f GetCenter(Deformable* geom);
  pxr::GfVec3f GetPosition(Deformable* geom, short idx);
  pxr::GfVec3f GetNormal(Deformable* geom);

  virtual bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Location* hit) const override;
  virtual bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, ClosestPoint* hit) const override;
  virtual bool Touch(const pxr::GfVec3f* points, const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize) const override;

  virtual bool Intersect(const Edge& other, float epsilon=0.0001);

  virtual pxr::GfRange3f GetBoundingBox(const pxr::GfVec3f* positions, const pxr::GfMatrix4d& m) const override;
  virtual short GetType() const override { return Component::POLYGON; };

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_POINT_H