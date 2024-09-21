#include "../acceleration/intersector.h"
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

void 
Intersector::SetGeometryCellIndices(size_t index, size_t start, size_t end) 
{
  _geoms[index].start = start;
  _geoms[index].end = end;
};

size_t
Intersector::GetGeometryIndex(Geometry* geom) const 
{
  for (size_t index = 0; index < _geoms.size(); ++index) {
    if(_geoms[index].geom == geom)return static_cast<int>(index);
  }
  return INVALID_INDEX;
}

size_t
Intersector::GetGeometryCellsStartIndex(size_t index) const 
{
  return _geoms[index].start;
}

size_t
Intersector::GetGeometryCellsEndIndex(size_t index) const 
{
  return _geoms[index].end;
}

void
Intersector::_Init(const std::vector<Geometry*>& geometries)
{
  _geoms.resize(geometries.size());
  size_t start = 0, end = 0;
  for(size_t g = 0; g < _geoms.size(); ++g) {
    _geoms[g].geom = geometries[g];
    _geoms[g].start = start;
    _geoms[g].end = end;
  }
}


JVR_NAMESPACE_CLOSE_SCOPE