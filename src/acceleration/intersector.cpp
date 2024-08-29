#include "../acceleration/intersector.h"
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE


size_t
Intersector::GetGeometryIndex(Geometry* geom) const 
{
  for (size_t index = 0; index < _geoms.size(); ++index) {
    if(_geoms[index].geom == geom)return static_cast<int>(index);
  }
  return INVALID_GEOMETRY;
}

void
Intersector::_Init(const std::vector<Geometry*>& geometries)
{
  _geoms.resize(geometries.size());
  size_t start = 0, end = 0;
  for(size_t g = 0; g < _geoms.size(); ++g) {
    _geoms[g].geom = geometries[g];
    _geoms[g].index = g;
    _geoms[g].start = start;
    _geoms[g].end = end;
  }
}


JVR_NAMESPACE_CLOSE_SCOPE