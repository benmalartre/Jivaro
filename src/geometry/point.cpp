#include "../geometry/point.h"
#include "../geometry/points.h"
#include "../geometry/mesh.h"

JVR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------
// Point Position
//-------------------------------------------------------
pxr::GfVec3f Point::GetPosition(Geometry* geom)
{
  return geom->GetPosition(id);
}

//-------------------------------------------------------
// Point Normal
//-------------------------------------------------------
pxr::GfVec3f Point::GetNormal(Geometry* geom)
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
Point::Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray,
    pxr::GfVec3f& closest, double maxDistance, double* minDistance)
{
  return false;
}

bool 
Point::Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, pxr::GfVec3f& closest,
  double maxDistance)
{
  const float distance = (point - points[id]).GetLength();
  if (maxDistance <= 0.f || distance < maxDistance) {
    closest = points[id];
    return true;
  }
  return false;
}

JVR_NAMESPACE_CLOSE_SCOPE