#include "../geometry/edge.h"
#include "../geometry/points.h"
#include "../geometry/mesh.h"

PXR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------
// Edge Center
//-------------------------------------------------------
void Edge::GetCenter(Geometry* geom, pxr::GfVec3f& center)
{
  center = (geom->GetPosition(vertices[0]) + geom->GetPosition(vertices[1])) * 0.5f;
}

//-------------------------------------------------------
// Edge Point Position
//-------------------------------------------------------
void Edge::GetPosition(Geometry* geom, pxr::GfVec3f& position, short idx)
{
  position = geom->GetPosition(vertices[idx]%2);
}

//-------------------------------------------------------
// Point Normal
//-------------------------------------------------------
void Edge::GetNormal(Geometry* geom, pxr::GfVec3f& normal)
{
  switch(geom->GetType()) {
    case Geometry::MESH:
    case Geometry::CURVE:
    {
      Mesh* mesh = (Mesh*)geom;

      // get points normals
      pxr::GfVec3f norm0 = geom->GetNormal(vertices[0]);
      pxr::GfVec3f norm1 = geom->GetNormal(vertices[1]);
  
      // average
      normal = (norm0 + norm1).GetNormalized();
      break;
    }

    case Geometry::POINT:
    {
      Points* points = (Points*)geom;

      // get point normals
      normal = points->GetNormal(vertices[0]);
      break;
    }
  }
}

//-------------------------------------------------------
// Intersect
//-------------------------------------------------------
bool Edge::Intersect(const Edge& other, float epsilon)
{
  return false;
}

PXR_NAMESPACE_CLOSE_SCOPE