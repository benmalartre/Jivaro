#include "../geometry/edge.h"
#include "../geometry/deformable.h"
#include "../geometry/intersection.h"

JVR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------
// Edge Center
//-------------------------------------------------------
pxr::GfVec3f 
Edge::GetCenter(Deformable* geom)
{
  return (geom->GetPosition(vertices[0]) + geom->GetPosition(vertices[1])) * 0.5f;
}

//-------------------------------------------------------
// Edge Point Position
//-------------------------------------------------------
pxr::GfVec3f 
Edge::GetPosition(Deformable* geom, short idx)
{
  return geom->GetPosition(vertices[idx]%2);
}

//-------------------------------------------------------
// Point Normal
//-------------------------------------------------------
pxr::GfVec3f 
Edge::GetNormal(Deformable* geom)
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
Edge::Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Location* hit) const
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
Edge::Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Location* hit) const
{
  return false;
}

pxr::GfRange3f
Edge::GetWorldBoundingBox(const Geometry* geometry) const
{
  const pxr::GfVec3f* points = ((Deformable*)geometry)->GetPositionsCPtr();
  const pxr::GfMatrix4d& matrix = geometry->GetMatrix();
  const pxr::GfVec3f extent(0.01f);

  pxr::GfRange3f range;
  range.UnionWith(matrix.Transform(points[vertices[0]] - extent));
  range.UnionWith(matrix.Transform(points[vertices[0]] + extent));
  range.UnionWith(matrix.Transform(points[vertices[1]] - extent));
  range.UnionWith(matrix.Transform(points[vertices[1]] + extent));
  return range;
}

pxr::GfRange3f
Edge::GetLocalBoundingBox(const Geometry* geometry) const
{
  const pxr::GfVec3f* points = ((Deformable*)geometry)->GetPositionsCPtr();
  const pxr::GfVec3f extent(0.01f);

  pxr::GfRange3f range;
  range.UnionWith(points[vertices[0]] - extent);
  range.UnionWith(points[vertices[0]] + extent);
  range.UnionWith(points[vertices[1]] - extent);
  range.UnionWith(points[vertices[1]] + extent);
  return range;
}

JVR_NAMESPACE_CLOSE_SCOPE