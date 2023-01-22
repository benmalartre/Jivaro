#include "../acceleration/hashGrid.h"
#include "../geometry/geometry.h"
#include "../utils/color.h"

JVR_NAMESPACE_OPEN_SCOPE

void 
HashGrid::Init(const std::vector<Geometry*>& geometries)
{
  Update(geometries);
}

void 
HashGrid::Update(const std::vector<Geometry*>& geometries)
{
  _geometries = geometries;
  size_t numElements = 0;
  for (const auto& geometry : _geometries) {
    numElements += geometry->GetNumPoints();
  }

  _tableSize = 2 * numElements;
  _cellStart.resize(_tableSize + 1, 0);
  _cellEntries.resize(numElements, 0);
  _mapping.resize(numElements);
  _points.resize(_geometries.size());

  std::vector<int64_t> hashes(numElements);
  
  size_t elemIdx = 0;
  for (size_t geomIdx = 0; geomIdx < _geometries.size(); ++geomIdx) {
    const Geometry* geometry = _geometries[geomIdx];
    _points[geomIdx] = geometry->GetPositionsCPtr();
    size_t numPoints = geometry->GetNumPoints();
    const pxr::GfVec3f* points = geometry->GetPositionsCPtr();
    for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
      _mapping[elemIdx] = _ComputeElementKey(geomIdx, pointIdx);
      hashes[elemIdx] = _HashCoords(_IntCoords(points[pointIdx]));
      _cellStart[hashes[elemIdx]]++;
      elemIdx++;
    }
    geomIdx++;
  }

  size_t start = 0;
  for (size_t tableIdx = 0; tableIdx < _tableSize; ++tableIdx) {
    start += _cellStart[tableIdx];
    _cellStart[tableIdx] = start;
  }
  _cellStart[_tableSize] = start;

  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    const int64_t h = hashes[elemIdx];
    _cellStart[h]--;
    _cellEntries[_cellStart[h]] = elemIdx;
  }
}

size_t 
HashGrid::Closests(const pxr::GfVec3f& point, 
  float maxDist, std::vector<int>& closests)
{
  const pxr::GfVec3i minCoord = _IntCoords(point - pxr::GfVec3f(maxDist));
  const pxr::GfVec3i maxCoord = _IntCoords(point + pxr::GfVec3f(maxDist));

  size_t closestIdx = 0;

  for (int xi = minCoord[0]; xi <= maxCoord[0]; ++xi) {
    for (int yi = minCoord[1]; yi <= maxCoord[1]; ++yi) {
      for (int zi = minCoord[2]; zi <= maxCoord[2]; ++zi) {
        int64_t h = _HashCoords(pxr::GfVec3i(xi, yi, zi));

        size_t start = _cellStart[h];
        size_t end = _cellStart[h + 1];

        for (size_t i = start; i < end; ++i) {
          closests[closestIdx] = _cellEntries[i];
          closestIdx++;
        }
      }
    }
  }
  return closestIdx;
}

const pxr::GfVec3f& 
HashGrid::_GetPoint(size_t elemIdx)
{
  std::cout << "ELEM  ID : " << elemIdx << std::endl;
  std::cout << "GEOM  ID : " << _GetGeometryIndexFromElementKey(elemIdx) << std::endl;
  std::cout << "POINT ID : " << _GetPointIndexFromElementKey(elemIdx) << std::endl;
  return _points[_GetGeometryIndexFromElementKey(elemIdx)][_GetPointIndexFromElementKey(elemIdx)];
}

pxr::GfVec3f
HashGrid::GetColor(const pxr::GfVec3f& point)
{
  int64_t h = _HashPos(point);
  return UnpackColor3<pxr::GfVec3f>(h);
}

JVR_NAMESPACE_CLOSE_SCOPE
