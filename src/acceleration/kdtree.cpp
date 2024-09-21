#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>

#include "../acceleration/kdtree.h"
#include "../geometry/deformable.h"

JVR_NAMESPACE_OPEN_SCOPE

KDTree::Cell*
KDTree::AddCell( const KDTree::IndexPoint* point)
{
  _cells.push_back(KDTree::Cell(point));
  return &_cells.back();
}

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
  _points.reserve(totalNumPoints);
  _numComponents = totalNumPoints;

  pxr::GfRange3d range;
  size_t offset = 0;
  for(size_t g = 0; g < geometries.size(); ++g) {
    const Deformable* deformable = static_cast<const Deformable*>(geometries[g]);

    size_t numPoints = deformable->GetNumPoints();
    
    const pxr::GfMatrix4d& matrix = deformable->GetMatrix();
    const pxr::GfVec3f* positions = deformable->GetPositionsCPtr();
    for (size_t i = 0; i < numPoints; ++i)
    {
      const pxr::GfVec3f worldPoint(matrix.Transform(positions[i]));
      range.UnionWith(worldPoint);
      _points.push_back(KDTree::IndexPoint(i + offset, worldPoint));
    }
    offset += numPoints;
  }

  _root = _BuildTreeRecursively(range, 0, 0, _points.size());
  std::cout << "kdtree root : " << (intptr_t)_root << std::endl;

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
  double minDistanceSq = DBL_MAX;
  KDTree::Cell *nearest = nullptr;
  _RecurseClosest(_root, point, 0, minDistanceSq, nearest);

  if(nearest) {
    std::cout << "nearest = " << nearest->point->index << std::endl;
    size_t cellIndex = nearest->point->index;
    size_t geomIndex = GetGeometryIndexFromCell(&_cells[cellIndex]);
    size_t pntIndex = cellIndex - GetGeometryCellsStartIndex(geomIndex);
    const Geometry *geometry = GetGeometryFromCell(&_cells[cellIndex]);


    std::cout << "geometry = " << geomIndex << std::endl;
    std::cout << "component = " << pntIndex << std::endl;
    hit->SetComponentIndex(pntIndex);
    hit->SetGeometryIndex(geomIndex);
    hit->SetCoordinates(pxr::GfVec3f(1.f, 0.f, 0.f));
    //hit->SetCoordinates(((Deformable*)geometry)->GetPosition(pntIndex));
    return true;
  }
  return false;
}

KDTree::Cell* 
KDTree::_BuildTreeRecursively(const pxr::GfRange3d& range, size_t depth, size_t begin, size_t end)
{
  KDTree::Cell* cell = AddCell();
  cell->SetMin(range.GetMin());
  cell->SetMax(range.GetMax());
  cell->axis = depth % 3;

  if(end - begin <= 1) {
    cell->point = &_points[begin];
  } else {
    size_t middle = (begin + end) >> 1;
    std::nth_element(_points.begin() + begin, _points.begin() + middle,
                     _points.begin() + end, _CompareDimension(cell->axis));
    cell->point = &_points[middle];

    if (middle - begin > 0) {
      pxr::GfVec3d maximum(range.GetMax());
      maximum[cell->axis] = _points[middle].position[cell->axis];
      cell->left = _BuildTreeRecursively(pxr::GfRange3d(range.GetMin(), maximum), depth + 1, begin, middle);
    }
    if (end - middle > 1) {
      pxr::GfVec3d minimum(range.GetMin());
      minimum[cell->axis] = _points[middle].position[cell->axis];
      cell->right = _BuildTreeRecursively(pxr::GfRange3d(minimum, range.GetMax()), depth + 1, middle + 1, end);
    }
  }

  return cell;
}

void
KDTree::_RecurseClosest(const KDTree::Cell *cell, const pxr::GfVec3f &point, size_t index, 
  double &minDistanceSq, KDTree::Cell *&nearest) const
{

}

size_t 
KDTree::_GetIndex(const KDTree::Cell* cell) const
{
  return  ((intptr_t)cell - (intptr_t)&_cells[0]) / sizeof(KDTree::Cell);
}

const Geometry* 
KDTree::GetGeometryFromCell(const KDTree::Cell* cell) const
{
  size_t geomIdx = GetGeometryIndexFromCell(cell);
  
  return geomIdx != INVALID_GEOMETRY ?
    GetGeometry(geomIdx) : NULL;

}

size_t
KDTree::GetGeometryIndexFromCell(const KDTree::Cell* cell) const
{
  size_t cellIdx = _GetIndex(cell);
  size_t startIdx, endIdx;
  size_t start = 0;
  size_t end = GetNumGeometries();
  size_t middle = end >> 1;

  while(start != end)
  {
    startIdx = GetGeometryCellsStartIndex(middle);
    endIdx = GetGeometryCellsEndIndex(middle);
    if(startIdx <= cellIdx && cellIdx < endIdx )
      return middle;

    else if (cellIdx < startIdx)
      end = middle;

    else
      start = middle;

    middle = (start + end) >> 1;
  } 
  return INVALID_GEOMETRY;
}

JVR_NAMESPACE_CLOSE_SCOPE