#include <pxr/base/gf/math.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/plane.h>

#include "../geometry/intersection.h"
#include "../geometry/geometry.h"
#include "../geometry/implicit.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"

JVR_NAMESPACE_OPEN_SCOPE

//=================================================================================================
// LOCATION CLASS
//=================================================================================================
void 
Location::Set(const Location& other) {
  _geomId = other._geomId;
  _elemId = other._elemId;
  _coords = other._coords;
}

pxr::GfVec3f 
Location::GetPosition(Geometry* geometry) const 
{
  switch (geometry->GetType()) {
    case Geometry::PLANE:
    {
      return geometry->GetMatrix().Transform(pxr::GfVec3f(_coords[0], _coords[1], _coords[2]));
    }

    case Geometry::CUBE:
    {
      Cube* cube = (Cube*)geometry;
      return pxr::GfVec3f(0.f);
    }

    case Geometry::SPHERE:
    {
      Sphere* sphere = (Sphere*)geometry;
      return pxr::GfVec3f(0.f);
    }

    case Geometry::MESH:
    {
      Mesh* mesh = (Mesh*)geometry;

      const Triangle* triangle = mesh->GetTriangle(_elemId);
      const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
      return geometry->GetMatrix().Transform(
        pxr::GfVec3f( 
          pxr::GfVec3f(positions[triangle->vertices[0]]) * _coords[0] +
          pxr::GfVec3f(positions[triangle->vertices[1]]) * _coords[1] +
          pxr::GfVec3f(positions[triangle->vertices[2]]) * _coords[2])
        );
    }

    case Geometry::CURVE:
    {
     
    }
    case Geometry::POINT:
    {
      Points* points = (Points*)geometry;
      Point point = points->Get(_elemId);
      
      //Point*
    }
  }
  return pxr::GfVec3f();
}

pxr::GfVec3f
Location::GetPosition(const pxr::GfRay& ray) const
{
  return pxr::GfVec3f(ray.GetPoint(_coords[3]));
}

pxr::GfVec3f
Location::GetNormal(Geometry* geometry) const
{
  switch (geometry->GetType()) {
    case Geometry::MESH:
    {

    }
    case Geometry::CURVE:
    {

    }
    case Geometry::POINT:
    {

    }
  }
  return pxr::GfVec3f();
}

float 
Location::GetDistance(Geometry* geometry, const pxr::GfVec3f& other)
{
  const pxr::GfVec3f world = geometry->GetMatrix().Transform(GetPosition(geometry));
  return (world - other).GetLength();
}

PXR_NAMESPACE_CLOSE_SCOPE
