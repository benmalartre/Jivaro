
#include "voronoi.h"

JVR_NAMESPACE_OPEN_SCOPE

static void _GetDistances(const pxr::VtArray<pxr::GfVec3f>& points, pxr::VtArray<float>* distances)
{
  size_t numPoints = points.size();
  distances->resize(numPoints * (numPoints - 1));
  size_t index = 0;
  for (size_t i = 0; i < numPoints; ++i) {
    for (size_t j = 0; j < numPoints; ++j) {
      if (i == j) continue;
      (*distances)[index++] = (points[j] - points[i]).GetLength();
    }
  }
}

int Voronoi2D::GetNumSurroundingSites(size_t x, size_t y)
{
  std::vector<bool> n = {
    true, true, true,
    true      , true,
    true, true, true
  };
  std::set<int> sites;
  if (x <= 0) { n[0] = false; n[3] = false; n[5] = false; }
  else if (x >= _resolutionX - 1) { n[2] = false; n[4] = false; n[7] = false; }
  if (y <= 0) { n[0] = false; n[1] = false; n[2] = false; }
  else if (y >= _resolutionY-1) { n[5] = false; n[6] = false; n[7] = false; }
  for (size_t i = 0; i < 8; ++i) {
    if (n[i]) {
    }
  }
  return 0;
}

void Voronoi2D::Build(pxr::UsdStageRefPtr& stage, const pxr::VtArray<pxr::GfVec3f>& seeds)
{
  _resolutionX = 1024;
  _resolutionY = 1024;
  size_t numCells = _resolutionX * _resolutionY;
  _cells.resize(numCells);
  memset(&_cells[0], -1, numCells * sizeof(int));

  for (auto& seed : seeds) {

  }
}

JVR_NAMESPACE_CLOSE_SCOPE