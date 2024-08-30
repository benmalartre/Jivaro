//======================================================
// EDGE DECLARATION
//======================================================
#ifndef JVR_GEOMETRY_EDGE_H
#define JVR_GEOMETRY_EDGE_H

#include "../geometry/component.h"


JVR_NAMESPACE_OPEN_SCOPE

class Deformable;
class Location;
struct Edge : public Component {

  Edge()
    : Component()
    , radius(1.f) {};
  Edge(uint32_t index, const pxr::GfVec2i& vertices, float radius = 1.f)
    : Component(index)
    , vertices(vertices)
    , radius(radius) {};

  pxr::GfVec2i vertices;    
  float radius;

  uint32_t GetStartIndex(){return vertices[0];};
  uint32_t GetEndIndex(){return vertices[1];};
  float GetRadius(){return radius;};
  pxr::GfVec3f GetCenter(Deformable* geom);
  pxr::GfVec3f GetPosition(Deformable* geom, short idx);
  pxr::GfVec3f GetNormal(Deformable* geom);

  virtual bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Location* hit) const override;
  virtual bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Location* hit) const override;
  virtual bool Touch(const pxr::GfVec3f* points, const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize) const override;

  bool Intersect(const Edge& other, float epsilon=0.0001);

  virtual pxr::GfRange3f GetBoundingBox(const pxr::GfVec3f* positions, const pxr::GfMatrix4d& m) const override;
  virtual short GetType() const override { return Component::EDGE; };

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_POINT_H