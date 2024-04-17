//======================================================
// EDGE DECLARATION
//======================================================
#ifndef JVR_GEOMETRY_EDGE_H
#define JVR_GEOMETRY_EDGE_H

#include "../common.h"
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

  bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Touch(const pxr::GfVec3f* points, 
    const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize) const override;

  bool Intersect(const Edge& other, float epsilon=0.0001);

  pxr::GfRange3f GetWorldBoundingBox(const Geometry* geometry) const override;
  pxr::GfRange3f GetLocalBoundingBox(const Geometry* geometry) const override;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_POINT_H