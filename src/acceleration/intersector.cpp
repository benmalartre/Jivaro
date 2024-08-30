#include "../acceleration/intersector.h"
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

void 
Intersector::SetGeometryCellIndices(size_t index, size_t cellIdx, size_t startIdx, size_t endIdx) {
  std::cout << "#############################################" << std::endl;
  std::cout << "cell index : " << cellIdx << std::endl;

  _geoms[index].index = cellIdx;
  _geoms[index].start = startIdx;
  _geoms[index].end = endIdx;
};

size_t
Intersector::GetIndexFromGeometry(Geometry* geom) const 
{
  for (size_t index = 0; index < _geoms.size(); ++index) {
    if(_geoms[index].geom == geom)return static_cast<int>(index);
  }
  return INVALID_GEOMETRY;
}

size_t
Intersector::GetGeometryCellIndex(size_t index) const 
{
  return _geoms[index].index;
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
  std::cout << "init intersector with " << geometries.size() << std::endl;
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