#include "../geometry/edge.h"
#include "../geometry/points.h"
#include "../geometry/mesh.h"

AMN_NAMESPACE_OPEN_SCOPE

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
    {
      Mesh* mesh = (Mesh*)geom;

      // get triangle normals
      pxr::GfVec3f norm0 = mesh->GetTriangleNormal(0);
      pxr::GfVec3f norm1 = mesh->GetTriangleNormal(1);
  
      // average
      normal = (norm0 + norm1).GetNormalized();
      break;
    }
    case Geometry::CURVE:
      break;
    case Geometry::POINT:
      break;
  }
}

AMN_NAMESPACE_CLOSE_SCOPE