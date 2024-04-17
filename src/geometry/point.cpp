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
float Point::GetRadius(Deformable* geom)
{
  return geom->GetRadius(id);
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
Point::Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{
  return false;
}

bool 
Point::Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Location* hit,
  double maxDistance, double* minDistance) const
{
  const float distance = (point - points[id]).GetLength();
  if ((maxDistance <= 0.f || distance < maxDistance ) && distance < hit->GetT()) {
    hit->SetElementIndex(id);
    hit->SetCoordinates(pxr::GfVec3f(0.f));
    hit->SetT(distance);
    return true;
  }
  return false;
}

bool
Point::Touch(const pxr::GfVec3f* points, 
  const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize) const
{
  return false;
}

pxr::GfRange3f
Point::GetWorldBoundingBox(const Geometry* geometry) const
{
  const pxr::GfVec3f* points = ((Deformable*)geometry)->GetPositionsCPtr();
  const pxr::GfMatrix4d& matrix = geometry->GetMatrix();
  const pxr::GfVec3f extent(0.01f);

  return pxr::GfRange3f().UnionWith(matrix.Transform(points[id]-extent));
}

pxr::GfRange3f
Point::GetLocalBoundingBox(const Geometry* geometry) const
{
  const pxr::GfVec3f* points = ((Deformable*)geometry)->GetPositionsCPtr();
  const pxr::GfVec3f extent(0.01f);

  return pxr::GfRange3f().UnionWith(points[id]-extent);
}


JVR_NAMESPACE_CLOSE_SCOPE