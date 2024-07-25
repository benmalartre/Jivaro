#include "../acceleration/intersector.h"
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE


size_t
Intersector::GetGeometryIndex(Geometry* geom) const 
{
  for (size_t index = 0; index < _geometries.size(); ++index) {
    if(_geometries[index] == geom)return static_cast<int>(index);
  }
  return INVALID_GEOMETRY;
}


JVR_NAMESPACE_CLOSE_SCOPE