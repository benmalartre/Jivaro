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
pxr::GfVec3f Point::GetPosition(Deformable* geom)
{
  return geom->GetPosition(id);
}

//-------------------------------------------------------
// Point Normal
//-------------------------------------------------------
pxr::GfVec3f Point::GetNormal(Deformable* geom)
{
  pxr::GfVec3f normal = pxr::GfVec3f(0.f, 1.f, 0.f);
  switch(geom->GetType()) {
    case Geometry::MESH:
    {
      Mesh* mesh = (Mesh*)geom;
      // get triangle edges
      pxr::GfVec3f AB = mesh->GetPosition(1) - mesh->GetPosition((size_t)0);
      pxr::GfVec3f AC = mesh->GetPosition(2) - mesh->GetPosition((size_t)0);
  
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
Point::Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Location* hit) const
{
  return false;
}

bool 
Point::Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Location* hit) const
{
  return false;
}

bool
Point::Touch(const pxr::GfVec3f* points, const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize) const
{

  return false;
}

pxr::GfRange3f 
Point::GetBoundingBox(const pxr::GfVec3f* positions, const pxr::GfMatrix4d& m) const
{
  const pxr::GfVec3f extent(0.05f);

  pxr::GfRange3f range;
  range.UnionWith(m.Transform(positions[id])-extent);
  range.UnionWith(m.Transform(positions[id])+extent);
  return range;
}


JVR_NAMESPACE_CLOSE_SCOPE