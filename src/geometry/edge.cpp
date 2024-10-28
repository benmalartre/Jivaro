#include "../geometry/edge.h"
#include "../geometry/deformable.h"
#include "../geometry/intersection.h"

JVR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------
// Edge Center
//-------------------------------------------------------
GfVec3f 
Edge::GetCenter(Deformable* geom)
{
  return (geom->GetPosition(vertices[0]) + geom->GetPosition(vertices[1])) * 0.5f;
}

//-------------------------------------------------------
// Edge Point Position
//-------------------------------------------------------
GfVec3f 
Edge::GetPosition(Deformable* geom, short idx)
{
  return geom->GetPosition(vertices[idx]%2);
}

//-------------------------------------------------------
// Point Normal
//-------------------------------------------------------
GfVec3f 
Edge::GetNormal(Deformable* geom)
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
// Intersect
//-------------------------------------------------------
bool
Edge::Intersect(const Edge& other, float epsilon)
{
  return false;
}

bool 
Edge::Raycast(const GfVec3f* points, const GfRay& ray, Location* hit) const
{
  return false;
}

bool 
Edge::Touch(const GfVec3f* points, const GfVec3f& center, const GfVec3f& halfSize) const
{
  return false;
}


bool 
Edge::Closest(const GfVec3f* points, const GfVec3f& point, Location* hit) const
{
  return false;
}

GfRange3f 
Edge::GetBoundingBox(const GfVec3f* positions, const GfMatrix4d& m) const
{
  const GfVec3f extent(radius);

  GfRange3f range;
  range.UnionWith(GfVec3f(m.Transform(positions[vertices[0]] - extent)));
  range.UnionWith(GfVec3f(m.Transform(positions[vertices[0]] + extent)));
  range.UnionWith(GfVec3f(m.Transform(positions[vertices[1]] - extent)));
  range.UnionWith(GfVec3f(m.Transform(positions[vertices[1]] + extent)));
  return range;
}



JVR_NAMESPACE_CLOSE_SCOPE