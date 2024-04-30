#include "../acceleration/hashGrid.h"
#include "../utils/color.h"
#include "../utils/timer.h"

JVR_NAMESPACE_OPEN_SCOPE

void 
HashGrid::Init(size_t n, const pxr::GfVec3f* points, float spacing)
{
  _spacing = spacing>1e-6 ? spacing : 1e-6;
  _scl = 1.f/_spacing;
  _n = n;
  _tableSize = 2 * _n;
  _cellStart.resize(_tableSize + 1);
  _cellEntries.resize(_n);
  _hashes.resize(_n);

  Update(points);
}

void 
HashGrid::Update(const pxr::GfVec3f* points)
{
  uint64_t T = CurrentTime();
  memset(&_cellStart[0], 0, _cellStart.size() * sizeof(int));
  memset(&_cellEntries[0], 0, _cellEntries.size() * sizeof(int));

  size_t elemIdx = 0;
  for (size_t pointIdx = 0; pointIdx < _n; ++pointIdx) {
    _hashes[elemIdx] = (this->*_HashCoords)(_IntCoords(points[pointIdx]));
    _cellStart[_hashes[elemIdx]]++;
    elemIdx++;
  }

  size_t start = 0;
  for (size_t tableIdx = 0; tableIdx < _tableSize; ++tableIdx) {
    start += _cellStart[tableIdx];
    _cellStart[tableIdx] = start;
  }
  _cellStart[_tableSize] = start;

  for (size_t elemIdx = 0; elemIdx < _n; ++elemIdx) {
    const int64_t h = _hashes[elemIdx];
    _cellStart[h]--;
    _cellEntries[_cellStart[h]] = elemIdx;
  }

}

void 
HashGrid::_ClosestsFromHash(size_t index, const pxr::GfVec3f* positions, 
  int64_t hash, std::vector<int>& closests) const 
{

  const pxr::GfVec3f& point = positions[index];
  int num = _cellStart[hash + 1] - _cellStart[hash];
  const float spacingTwo = _spacing * _spacing;
  for (size_t n = 0; n < num; ++n) {
    int neighborIdx = _cellEntries[_cellStart[hash] + n];
    if(neighborIdx == index)continue;
    if((point - positions[neighborIdx]).GetLengthSq() < spacingTwo)
      closests.push_back(neighborIdx);
  }
}

size_t 
HashGrid::Closests(size_t index, const pxr::GfVec3f* positions,
  std::vector<int>& closests) const
{
  closests.clear();
  const pxr::GfVec3i centerCoords = _IntCoords(positions[index]);

  const pxr::GfVec3i minCoords = centerCoords - pxr::GfVec3i(1);
  const pxr::GfVec3i maxCoords = centerCoords + pxr::GfVec3i(1);

  for (int x = minCoords[0]; x <= maxCoords[0]; ++x)
    for (int y = minCoords[1]; y <= maxCoords[1]; ++y)
      for (int z = minCoords[2]; z <= maxCoords[2]; ++z)
        _ClosestsFromHash(index, positions, 
          (this->*_HashCoords)(pxr::GfVec3i(x, y, z)), closests);

  return closests.size();
}

pxr::GfVec3f
HashGrid::GetColor(const pxr::GfVec3f& point)
{
  int64_t h = (this->*_HashCoords)(_IntCoords(point));
  return UnpackColor3<pxr::GfVec3f>(h);
}

JVR_NAMESPACE_CLOSE_SCOPE
