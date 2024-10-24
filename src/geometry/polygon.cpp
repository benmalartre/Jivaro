#include "../geometry/polygon.h"
#include "../geometry/deformable.h"
#include "../geometry/intersection.h"

JVR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------
// Polygon Center
//-------------------------------------------------------
GfVec3f 
Polygon::GetCenter(Deformable* geom)
{
  const GfVec3f* points = geom->GetPositionsCPtr();
  GfVec3f accum = points[triangles[t]->vertices[]];
  size_t numTriangles = triangles.size();
  for(size_t t = 0; t < numTriangles; ++t) {
    accum += points[triangles[t]->vertices[1]];
  }
  accum += points[triangles[numTriangles-1]->vertices[1]];
  accum /= triangles.size
}

//-------------------------------------------------------
// Polygon Point Position
//-------------------------------------------------------
GfVec3f 
Polygon::GetPosition(Deformable* geom, short idx)
{
  return geom->GetPosition(vertices[idx]%2);
}

//-------------------------------------------------------
// Point Normal
//-------------------------------------------------------
GfVec3f 
Polygon::GetNormal(Deformable* geom)
{
  GfVec3f normal(0.f,1.f,0.f);
  switch(geom->GetType()) {
    case Geometry::MESH:
    case Geometry::CURVE:
    {
      // get points normals
      GfVec3f norm0 = geom->GetNormal(vertices[0]);
      GfVec3f norm1 = geom->GetNormal(vertices[1]);
  
      // average
      normal = (norm0 + norm1).GetNormalized();
      break;
    }

    case Geometry::POINT:
    {
      // get point normals
      normal = geom->GetNormal(vertices[0]);
      break;
    }
  }
  return normal;
}


//-------------------------------------------------------
// Polygon raycast
//-------------------------------------------------------
bool
Polygon::Raycast(const GfVec3f* points, const GfRay& ray, Location* hit) const
{
  bool hitSometing = false;
  for(i = 1; i < vertices.size() - 1; ++i) {

  }
  if (left && left->Raycast(points, ray, hit))hitSometing = true;
  if (right && right->Raycast(points, ray, hit))hitSometing = true;
  return hitSometing;
}

//-------------------------------------------------------
// Polygon closest point
//-------------------------------------------------------
bool 
Polygon::Closest(const GfVec3f* points, const GfVec3f& point, Location* hit) const
{
  bool hitSometing = false;
  if (left && left->Closest(points, point, hit))hitSometing = true;
  if (right && right->Closest(points, point, hit))hitSometing = true;
  return hitSometing;
};

//-------------------------------------------------------
// Polygon touch box
//-------------------------------------------------------
bool 
Polygon::Touch(const GfVec3f* points, const GfVec3f& center, 
  const GfVec3f& boxhalfsize) const
{
  return false;
}

GfRange3f 
Edge::GetBoundingBox(const GfVec3f* positions, const GfMatrix4d& m) const
{
  const GfVec3f extent(radius);

  GfRange3f range;
  range.UnionWith(m.Transform(positions[vertices[0]] - extent));
  range.UnionWith(m.Transform(positions[vertices[0]] + extent));
  range.UnionWith(m.Transform(positions[vertices[1]] - extent));
  range.UnionWith(m.Transform(positions[vertices[1]] + extent));
  return range;
}



JVR_NAMESPACE_CLOSE_SCOPE