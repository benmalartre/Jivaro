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
  Polygon(uint32_t index, const pxr::VtArray<Triangles>& tris)
    : Component(index)
    , triangles(tris){};

  std::vector<Triangle*> triangles;    

  pxr::GfVec3f GetCenter(Deformable* geom);
  pxr::GfVec3f GetPosition(Deformable* geom, short idx);
  pxr::GfVec3f GetNormal(Deformable* geom);

  bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Location* hit) const override;
  bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Location* hit) const override;
  bool Touch(const pxr::GfVec3f* points, const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize) const override;

  bool Intersect(const Edge& other, float epsilon=0.0001);

  pxr::GfRange3f GetBoundingBox(const pxr::GfVec3f* positions, const pxr::GfMatrix4d& m) const override;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_POINT_H