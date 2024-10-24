#include "../acceleration/hashGrid.h"
#include "../utils/color.h"
#include "../utils/timer.h"

JVR_NAMESPACE_OPEN_SCOPE

void 
HashGrid::Init(size_t n, const GfVec3f* points, float spacing)
{
  _spacing = spacing>1e-6 ? spacing : 1e-6;
  _scl = 1.f/_spacing;
  _n = n;
  _tableSize = 2 * _n;
  _cellStart.resize(_tableSize + 1);
  _cellEntries.resize(_n);

  Update(points);
}

void 
HashGrid::Update(const GfVec3f* points)
{
  uint64_t T = CurrentTime();
  memset(&_cellStart[0], 0, _cellStart.size() * sizeof(int));
  memset(&_cellEntries[0], 0, _cellEntries.size() * sizeof(int));

  for (size_t pointIdx = 0; pointIdx < _n; ++pointIdx) {
    int64_t hash = (this->*_HashCoords)(_IntCoords(points[pointIdx]));
    _cellStart[hash]++;
  }

  size_t start = 0;
  for (size_t tableIdx = 0; tableIdx < _tableSize; ++tableIdx) {
    start += _cellStart[tableIdx];
    _cellStart[tableIdx] = start;
  }
  _cellStart[_tableSize] = start;

  for (size_t pointIdx = 0; pointIdx < _n; ++pointIdx) {
    int64_t hash = (this->*_HashCoords)(_IntCoords(points[pointIdx]));
    _cellStart[hash]--;
    _cellEntries[_cellStart[hash]] = pointIdx;
  }
}

size_t 
HashGrid::Closests(size_t index, const GfVec3f* positions,
  std::vector<int>& closests, float distance) const
{
  const GfVec3f& point = positions[index];
  const GfVec3i minCoords = _IntCoords(point - GfVec3f(distance));
  const GfVec3i maxCoords = _IntCoords(point + GfVec3f(distance));
  const float distance2 = distance * distance;

  for (int x = minCoords[0]; x <= maxCoords[0]; ++x)
    for (int y = minCoords[1]; y <= maxCoords[1]; ++y)
      for (int z = minCoords[2]; z <= maxCoords[2]; ++z) {
        int64_t hash = (this->*_HashCoords)(GfVec3i(x, y, z));
        int start = _cellStart[hash];
        int end = _cellStart[hash + 1];

        for (int n = start; n < end; ++n) {
          if(_cellEntries[n] != index && 
            (point - positions[_cellEntries[n]]).GetLengthSq() < distance2)
              closests.push_back(_cellEntries[n]);
        }
      }

  return closests.size();
}

size_t 
HashGrid::Closests(size_t index, const GfVec3f* positions, const GfVec3f* velocities, float ft,
  std::vector<int>& closests, float distance) const
{
  const GfVec3f& point = positions[index] + velocities[index] * ft;
  const GfVec3i minCoords = _IntCoords(point - GfVec3f(distance));
  const GfVec3i maxCoords = _IntCoords(point + GfVec3f(distance));
  const float distance2 = distance * distance;

  for (int x = minCoords[0]; x <= maxCoords[0]; ++x)
    for (int y = minCoords[1]; y <= maxCoords[1]; ++y)
      for (int z = minCoords[2]; z <= maxCoords[2]; ++z) {
        int64_t hash = (this->*_HashCoords)(GfVec3i(x, y, z));
        int start = _cellStart[hash];
        int end = _cellStart[hash + 1];

        for (int n = start; n < end; ++n) {
          if(_cellEntries[n] != index && 
            (point - (positions[_cellEntries[n]] + velocities[_cellEntries[n]])).GetLengthSq() < distance2)
              closests.push_back(_cellEntries[n]);
        }
      }

  return closests.size();
}

GfVec3f
HashGrid::GetColor(const GfVec3f& point)
{
  int64_t h = (this->*_HashCoords)(_IntCoords(point));
  return UnpackColor3<GfVec3f>(h);
}

JVR_NAMESPACE_CLOSE_SCOPE
