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
  Polygon(uint32_t index, const VtArray<size_t>& vertices)
    : Component(index)
    , vertices(vertices){};

  std::vector<size_t> vertices;    

  GfVec3f GetCenter(Deformable* geom);
  GfVec3f GetPosition(Deformable* geom, short idx);
  GfVec3f GetNormal(Deformable* geom);

  virtual bool Raycast(const GfVec3f* points, const GfRay& ray, Location* hit) const override;
  virtual bool Closest(const GfVec3f* points, const GfVec3f& point, Location* hit) const override;
  virtual bool Touch(const GfVec3f* points, const GfVec3f& center, const GfVec3f& halfSize) const override;

  virtual bool Intersect(const Polygon& other, float epsilon=0.0001);

  virtual GfRange3f GetBoundingBox(const GfVec3f* positions, const GfMatrix4d& m) const override;
  virtual short GetType() const override { return Component::POLYGON; };

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_POINT_H