#include "../geometry/edge.h"
#include "../geometry/points.h"
#include "../geometry/mesh.h"
#include "../geometry/intersection.h"

JVR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------
// Edge Center
//-------------------------------------------------------
pxr::GfVec3f 
Edge::GetCenter(Geometry* geom)
{
  return (geom->GetPosition(vertices[0]) + geom->GetPosition(vertices[1])) * 0.5f;
}

//-------------------------------------------------------
// Edge Point Position
//-------------------------------------------------------
pxr::GfVec3f 
Edge::GetPosition(Geometry* geom, short idx)
{
  return geom->GetPosition(vertices[idx]%2);
}

//-------------------------------------------------------
// Point Normal
//-------------------------------------------------------
pxr::GfVec3f 
Edge::GetNormal(Geometry* geom)
{
  pxr::GfVec3f normal(0.f,1.f,0.f);
  switch(geom->GetType()) {
    case Geometry::MESH:
    case Geometry::CURVE:
    {
      // get points normals
      pxr::GfVec3f norm0 = geom->GetNormal(vertices[0]);
      pxr::GfVec3f norm1 = geom->GetNormal(vertices[1]);
  
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
Edge::Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
  double maxDistance, double* minDistance) const
{
  return false;
}

bool 
Edge::Touch(const pxr::GfVec3f* points, 
  const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize) const
{
  return false;
}


bool 
Edge::Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
  double maxDistance, double* minDistance) const
{
  pxr::GfVec3f closest;
  float distance = (point - closest).GetLength();
  if (distance < maxDistance && distance < *minDistance) {
    if (minDistance) *minDistance = distance;
    hit->SetBarycentricCoordinates(pxr::GfVec3f(0.5f));
    hit->SetElementIndex(id);
    hit->SetT(distance);
    return true;
  }
  return false;
}


JVR_NAMESPACE_CLOSE_SCOPE