#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>

#include "../acceleration/kdtree.h"
#include "../geometry/deformable.h"

JVR_NAMESPACE_OPEN_SCOPE

KDTree::Cell*
KDTree::AddCell( const KDTree::IndexPoint &point)
{
  _cells.push_back(KDTree::Cell(point));
  return &_cells.back();
}

void
KDTree::Init(const std::vector<Geometry*> &geometries) 
{
  switch(_distanceType) {
    case DistanceType::CHEBYSHEV:
      _distance = new DistanceChebyshev();
      break;

    case DistanceType::MANHATTAN:
      _distance = new DistanceManhattan();
      break;

    case DistanceType::EUCLIDEAN:
      _distance = new DistanceEuclidean();
      break;
    
    default:
      std::cerr << "KDTree initialize DistanceMesure fail: invalid type!";
      return;
  }

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

  SetMin(range.GetMin());
  SetMax(range.GetMax());

  _root = _BuildTreeRecursively(range, 0, 0, _points.size());
  std::cout << "kdtree root : " << (intptr_t)_root << std::endl;

  std::cout << "bbox minimum : " << GetMin() << std::endl;
  std::cout << "bbox maximum : " << GetMax() << std::endl;

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
    std::cout << "nearest = " << nearest->point.index << std::endl;
    size_t cellIndex = nearest->point.index;
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
    cell->point = _points[begin];
  } else {
    size_t middle = (begin + end) >> 1;
    std::nth_element(_points.begin() + begin, _points.begin() + middle,
                     _points.begin() + end, _CompareDimension(cell->axis));
    cell->point = _points[middle];

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
  /*
  double curdist, dist;

  curdist = distance->distance(point, node->point);
  if (!(searchpredicate && !(*searchpredicate)(allnodes[node->dataindex]))) {
    if (neighborheap->size() < k) {
      neighborheap->push(nn4heap(node->dataindex, curdist));
    } else if (curdist < neighborheap->top().distance) {
      neighborheap->pop();
      neighborheap->push(nn4heap(node->dataindex, curdist));
    }
  }
  // first search on side closer to point
  if (point[node->cutdim] < node->point[node->cutdim]) {
    if (node->loson)
      if (neighbor_search(point, node->loson, k, neighborheap)) return true;
  } else {
    if (node->hison)
      if (neighbor_search(point, node->hison, k, neighborheap)) return true;
  }
  // second search on farther side, if necessary
  if (neighborheap->size() < k) {
    dist = std::numeric_limits<double>::max();
  } else {
    dist = neighborheap->top().distance;
  }
  if (point[node->cutdim] < node->point[node->cutdim]) {
    if (node->hison && bounds_overlap_ball(point, dist, node->hison))
      if (neighbor_search(point, node->hison, k, neighborheap)) return true;
  } else {
    if (node->loson && bounds_overlap_ball(point, dist, node->loson))
      if (neighbor_search(point, node->loson, k, neighborheap)) return true;
  }

  if (neighborheap->size() == k) dist = neighborheap->top().distance;
  return ball_within_bounds(point, dist, node);
  */
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
  
  return geomIdx != INVALID_INDEX ?
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
  return INVALID_INDEX;
}

bool
KDTree::_IntersectSphere(const pxr::GfVec3f& center, double radius, KDTree::Cell *cell) 
{
  const pxr::GfVec3d& minimum = cell->GetMin();
  const pxr::GfVec3d& maximum = cell->GetMax();

  // maximum distance needs different treatment
  if (_distanceType == DistanceType::CHEBYSHEV) {
    double maxDist = 0.0;
    double currDist = 0.0;
    for (size_t i = 0; i < 2; ++i) {
      if (center[i] < minimum[i])   // lower than low boundary
        currDist = _distance->Compute1D(center[i], minimum[i], i);
      
      else if (center[i] > maximum[i])   // higher than high boundary
        currDist = _distance->Compute1D(center[i], maximum[i], i);
      
      if (currDist > maxDist) 
        maxDist = currDist;
      
      if (maxDist > radius) return false;
    }
    return true;
  } else {
    double sum = 0.0;    
    for (size_t i = 0; i < 3; ++i) {
      if (center[i] < minimum[i]) {  // lower than low boundary
        sum += _distance->Compute1D(center[i], minimum[i], i);
        if (sum > radius) return false;
      }
      else if (center[i] > maximum[i]) {  // higher than high boundary
        sum += _distance->Compute1D(center[i], maximum[i], i);
        if (sum > radius) return false;
      }
    }
    return true;
  }
}

// returns true when the bounds of *node* completely contain the
// ball with radius *dist* around *point*
bool 
KDTree::_ContainsSphere(const pxr::GfVec3f& center, double radius, KDTree::Cell *cell) 
{
  const pxr::GfVec3d& minimum = cell->GetMin();
  const pxr::GfVec3d& maximum = cell->GetMax();

  for (size_t i = 0; i < 3; ++i)
    if (_distance->Compute1D(center[i], minimum[i], i) <= radius ||
        _distance->Compute1D(center[i], maximum[i], i) <= radius)
      return false;
  return true;
}

JVR_NAMESPACE_CLOSE_SCOPE