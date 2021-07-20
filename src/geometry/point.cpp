#include "../geometry/point.h"
#include "../geometry/points.h"
#include "../geometry/mesh.h"

AMN_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------
// Point Position
//-------------------------------------------------------
void Point::GetPosition(Geometry* geom, pxr::GfVec3f& center)
{
  center = geom->GetPosition(id);
}

//-------------------------------------------------------
// Point Normal
//-------------------------------------------------------
void Point::GetNormal(Geometry* geom, pxr::GfVec3f& normal)
{
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
}

AMN_NAMESPACE_CLOSE_SCOPE