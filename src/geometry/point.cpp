#include "../geometry/point.h"
#include "../geometry/deformable.h"
#include "../geometry/mesh.h"
#include "../geometry/points.h"
#include "../geometry/curve.h"
#include "../geometry/intersection.h"

JVR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------
// Point Radius
//-------------------------------------------------------
float Point::GetWidth(Deformable* geom)
{
  return geom->GetWidth(id);
}

//-------------------------------------------------------
// Point Position
//-------------------------------------------------------
GfVec3f Point::GetPosition(Deformable* geom)
{
  return geom->GetPosition(id);
}

//-------------------------------------------------------
// Point Normal
//-------------------------------------------------------
GfVec3f Point::GetNormal(Deformable* geom)
{
  GfVec3f normal = GfVec3f(0.f, 1.f, 0.f);
  switch(geom->GetType()) {
    case Geometry::MESH:
    {
      Mesh* mesh = (Mesh*)geom;
      // get triangle edges
      GfVec3f AB = mesh->GetPosition(1) - mesh->GetPosition((size_t)0);
      GfVec3f AC = mesh->GetPosition(2) - mesh->GetPosition((size_t)0);
  
      // cross product
      normal = AB ^ AC;
  
      // normalize
      normal.Normalize();
      break;
    }
    case Geometry::CURVE:
      break;
    case Geometry::POINT:
      break;
  }
  return normal;
}

bool 
Point::Raycast(const GfVec3f* points, const GfRay& ray, Location* hit) const
{
  return false;
}

bool 
Point::Closest(const GfVec3f* points, const GfVec3f& point, Location* hit) const
{
  return false;
}

bool
Point::Touch(const GfVec3f* points, const GfVec3f& center, const GfVec3f& halfSize) const
{

  return false;
}

GfRange3f 
Point::GetBoundingBox(const GfVec3f* positions, const GfMatrix4d& m) const
{
  const GfVec3f extent(0.05f);

  GfRange3f range;
  range.UnionWith(m.Transform(positions[id])-extent);
  range.UnionWith(m.Transform(positions[id])+extent);
  return range;
}


JVR_NAMESPACE_CLOSE_SCOPE