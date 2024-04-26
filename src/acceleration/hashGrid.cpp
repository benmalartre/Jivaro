#include "../acceleration/hashGrid.h"
#include "../utils/timer.h"

JVR_NAMESPACE_OPEN_SCOPE

void 
HashGrid::Init(size_t n, const pxr::GfVec3f* points, float radius)
{
  uint64_t T = CurrentTime();
  _points = points;
  _n = n;
  _tableSize = 2 * n;
  _cellStart.resize(_tableSize + 1, 0);
  _cellEntries.resize(n, 0);
  _mapping.resize(n);

  std::vector<int64_t> hashes(n);

  size_t elemIdx = 0;
  for (size_t pointIdx = 0; pointIdx < _n; ++pointIdx) {
    
    hashes[elemIdx] = (this->*_HashCoords)(_IntCoords(points[pointIdx]));
    _mapping[elemIdx] = pointIdx;
    _cellStart[hashes[elemIdx]]++;
    elemIdx++;
  }

  size_t start = 0;
  for (size_t tableIdx = 0; tableIdx < _tableSize; ++tableIdx) {
    start += _cellStart[tableIdx];
    _cellStart[tableIdx] = start;
  }
  _cellStart[_tableSize] = start;

  for (size_t elemIdx = 0; elemIdx < _n; ++elemIdx) {
    const int64_t h = hashes[elemIdx];
    _cellStart[h]--;
    _cellEntries[_cellStart[h]] = elemIdx;
  }

  std::cout << "  update took " << ((CurrentTime() - T) * 1e-9) << " seconds for " << n << std::endl;
  std::cout << "  method : " << _hashMethod << std::endl;
  std::cout << "  num elements : " << n << std::endl;
  std::cout << "  table size : " << _tableSize << std::endl;

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
        int64_t h = (this->*_HashCoords)(pxr::GfVec3i(xi, yi, zi));

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
  return _points[elemIdx];
}

pxr::GfVec3f
HashGrid::GetColor(const pxr::GfVec3f& point)
{
  int64_t h = (this->*_HashCoords)(_IntCoords(point));
  return UnpackColor3<pxr::GfVec3f>(h);
}

JVR_NAMESPACE_CLOSE_SCOPE
