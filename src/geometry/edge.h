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
  Edge(uint32_t index, const GfVec2i& vertices, float radius = 1.f)
    : Component(index)
    , vertices(vertices)
    , radius(radius) {};

  GfVec2i vertices;    
  float radius;

/*
  uint32_t GetStartIndex(){return vertices[0];};
  uint32_t GetEndIndex(){return vertices[1];};
  float GetRadius(){return radius;};
  GfVec3f GetCenter(Deformable* geom);
  GfVec3f GetPosition(Deformable* geom, short idx);
  GfVec3f GetNormal(Deformable* geom);

  bool Intersect(const Edge& other, float epsilon=0.0001);
*/
  virtual bool Touch(const GfVec3f* points, const GfVec3f& center, const GfVec3f& halfSize) const override;
  virtual bool Raycast(const GfVec3f* points, const GfRay& ray, Location* hit) const override;
  virtual bool Closest(const GfVec3f* points, const GfVec3f& point, Location* hit) const override;

  virtual GfRange3f GetBoundingBox(const GfVec3f* positions, const GfMatrix4d& m) const override;
  virtual short GetType() const override;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_POINT_H