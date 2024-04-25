#include "../acceleration/intersector.h"
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE


int
Intersector::GetGeometryIndex(Geometry* geom) const 
{
  for (size_t index = 0; index < _geometries.size(); ++index) {
    if(_geometries[index] == geom)return static_cast<int>(index);
  }
  return -1;
}


JVR_NAMESPACE_CLOSE_SCOPE