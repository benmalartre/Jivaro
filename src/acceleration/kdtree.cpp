#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>

#include "../acceleration/kdtree.h"
#include "../geometry/deformable.h"

JVR_NAMESPACE_OPEN_SCOPE


void
KDTree::Init(const std::vector<Geometry*> &geometries) 
{
  Intersector::_Init(geometries);

  size_t totalNumPoints = 0;
  for(size_t g = 0; g < geometries.size(); ++g) {
    if(geometries[g]->GetType() < Geometry::POINT)
      throw std::runtime_error("KDTree geometry type unsupported!");

    size_t numPoints = geometries[g]->GetNumPoints();
    SetGeometryCellIndices(g, totalNumPoints, totalNumPoints + numPoints);
    totalNumPoints += numPoints;
  }
  _cells.reserve(totalNumPoints);
  _numComponents = totalNumPoints;

  size_t offset = 0;
  for(size_t g = 0; g < geometries.size(); ++g) {
    const Deformable* deformable = static_cast<const Deformable*>(geometries[g]);

    size_t numPoints = deformable->GetNumPoints();
    
    const pxr::GfMatrix4d& matrix = deformable->GetMatrix();
    const pxr::GfVec3f* positions = deformable->GetPositionsCPtr();
    for (size_t i = 0; i < numPoints; ++i)
    {
      _cells.emplace_back(matrix.Transform(positions[i]), i + offset);
    }
    offset += numPoints;
  }

  _root = _BuildTreeRecursively(_cells.begin(), _cells.end(), 0UL);

  if (_root == nullptr)
  {
      throw std::runtime_error("KDTree is empty.");
  }
}

void
KDTree::Update()
{
}

bool 
KDTree::Raycast(const pxr::GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{
  return false;
}

bool 
KDTree::Closest(const pxr::GfVec3f& point, Location* hit,
  double maxDistance) const
{
  return false;
}

KDTree::Cell* 
KDTree::_BuildTreeRecursively(std::vector<Cell>::iterator begin,
  std::vector<Cell>::iterator end, std::size_t index) const
{
  if (begin >= end)
  {
    return nullptr;
  }

  auto middle = begin + std::distance(begin, end) / 2;

  std::nth_element(begin, middle, end, [&index](const Cell& n1, const Cell& n2) -> bool {
    return (n1.point[index] < n2.point[index]);
  });

  index = (index + 1) % 3;

  middle->left = _BuildTreeRecursively(begin, middle, index);
  middle->right = _BuildTreeRecursively(middle + 1, end, index);

  return &(*middle);
}

void
KDTree::_RecurseClosest(const KDTree::Cell *cell, const pxr::GfVec3f &point, size_t index, 
  double &minDistanceSq, KDTree::Cell *&nearest) const
{
  if (cell == nullptr)
    return;

  const double distanceSq = _DistanceSq(point, cell->point);
  if (distanceSq <= minDistanceSq)
  {
    minDistanceSq = distanceSq;
    nearest = const_cast<KDTree::Cell *>(cell);
  }

  const double delta = cell->point[index] - point[index];

  index = (index + 1) % 3;

  const bool isDeltaPositive = (delta > 0.0);
  _RecurseClosest(isDeltaPositive ? cell->left : cell->right, 
    point, index, minDistanceSq, nearest);

  if (delta * delta <= minDistanceSq)
    _RecurseClosest(isDeltaPositive ? cell->right : cell->left, 
      point, index, minDistanceSq, nearest);
}

JVR_NAMESPACE_CLOSE_SCOPE