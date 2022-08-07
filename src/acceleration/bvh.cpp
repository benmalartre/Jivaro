#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/ray.h>
#include "../acceleration/bvh.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"

JVR_NAMESPACE_OPEN_SCOPE

// distance
static double 
_GetDistance1D(double value, double lower, double upper)
{
  if (value < lower)return lower - value;
  if (value > upper)return value - upper;
  return pxr::GfMin(value - lower, upper - value);
};

// get distance
static double 
_GetDistance(const pxr::GfRange3d* lhs, const pxr::GfRange3d* rhs)
{
  pxr::GfVec3d half = lhs->GetSize() * 0.5;
  pxr::GfVec3d center = lhs->GetMidpoint();
  const pxr::GfVec3d& minimum = rhs->GetMin();
  const pxr::GfVec3d& maximum = rhs->GetMax();
  float dx = _GetDistance1D(center[0], minimum[0] + half[0], maximum[0] - half[0]);
  float dy = _GetDistance1D(center[1], minimum[1] + half[1], maximum[1] - half[1]);
  float dz = _GetDistance1D(center[2], minimum[2] + half[2], maximum[2] - half[2]);
  return pxr::GfSqrt(dx * dx + dy * dy + dz * dz);
}

static pxr::GfVec3d 
_GetBoundingMinimum(const pxr::GfRange3d* lhs, const pxr::GfRange3d* rhs)
{
  return pxr::GfRange3d::GetUnion(*lhs, *rhs).GetMin();
}

static pxr::GfVec3d 
_GetBoundingMaximum(const pxr::GfRange3d* lhs, const pxr::GfRange3d* rhs)
{
  return pxr::GfRange3d::GetUnion(*lhs, *rhs).GetMax();
}

static pxr::GfRange3d 
_GetBoundingBox(const pxr::GfRange3d* lhs, const pxr::GfRange3d* rhs)
{
  return pxr::GfRange3d::GetUnion(*lhs, *rhs);
}

BVH::Cell* 
BVH::Cell::GetRoot()
{
  Cell* parent = _parent;
  while (parent) {
    if (parent->IsRoot()) return this;
    parent = parent->_parent;
  }
  return parent;
}

const BVH::Cell*
BVH::Cell::GetRoot() const
{
  Cell* parent = _parent;
  while (parent) {
    if (parent->IsRoot()) return parent;
    parent = parent->_parent;
  }
  return parent;
}

BVH::Root::Root(Geometry* geometry, BVH::Cell* parent) 
  : BVH::Branch(parent)
  , _geom(geometry)
{
  
}

BVH::Leaf::Leaf(BVH::Cell* parent, Geometry* geometry)
  : BVH::Cell(parent)
{
  pxr::GfRange3d range = geometry->GetBoundingBox().GetRange();
  SetMin(pxr::GfVec3d(range.GetMin()));
  SetMax(pxr::GfVec3d(range.GetMax()));

  BVH* bvh = new BVH();
  bvh->Init(geometry, this);
  _raycastImpl = &BVH::Leaf::_RaycastGeometry;
  _data = (void*)bvh;
  SetMin(bvh->GetBoundingBox().GetMin());
  SetMax(bvh->GetBoundingBox().GetMax());
}

BVH::Leaf::Leaf(BVH::Cell* parent, TrianglePair* pair, const pxr::GfRange3d& range)
  : BVH::Cell(parent)
{
  SetMin(range.GetMin());
  SetMax(range.GetMax());
  _data = (void*)pair;
  _raycastImpl = &BVH::Leaf::_RaycastTrianglePair;
}

BVH::Leaf::Leaf(BVH::Cell* parent, Triangle* tri, const pxr::GfRange3d& range)
{
  SetMin(range.GetMin());
  SetMax(range.GetMax());
  _data = (void*)tri;
  _raycastImpl = &BVH::Leaf::_RaycastTrianglePair;
}

Geometry*
BVH::Leaf::GetGeometry()
{
  BVH::Cell* root = GetRoot();
  return ((BVH::Root*)root)->GetGeometry();
}

const Geometry*
BVH::Leaf::GetGeometry() const
{
  const BVH::Cell* root = GetRoot();
  return ((BVH::Root*)root)->GetGeometry();
}

bool
BVH::Leaf::_RaycastGeometry(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
  double maxDistance, double* minDistance) const
{
  BVH* bvh = (BVH*)_data;
  return bvh->Raycast(ray, hit, maxDistance, minDistance);
}

bool
BVH::Leaf::_RaycastTrianglePair(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
  double maxDistance, double* minDistance) const
{
  TrianglePair* pair = (TrianglePair*)_data;
  return pair->Raycast(points, ray, hit, maxDistance, minDistance);
}

bool
BVH::Leaf::Raycast(const pxr::GfRay& ray, Hit* hit,
  double maxDistance, double* minDistance) const
{
  Geometry* geom = (Geometry*)GetGeometry();
  double distance;
  if (ray.Intersect(*this, &distance)) {
    return (this->*_raycastImpl)(geom->GetPositionsCPtr(), ray, hit, maxDistance, minDistance);
  }
  return false;
}

bool 
BVH::Leaf::_ClosestGeometry(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
  double maxDistance, double* minDistance) const
{
  BVH* bvh = (BVH*)_data;
  return bvh->Closest(point, hit, maxDistance, minDistance);
}

bool 
BVH::Leaf::_ClosestTrianglePair(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
  double maxDistance, double* minDistance) const
{
  TrianglePair* pair = (TrianglePair*)_data;
  return pair->Closest(points, point, hit, maxDistance, minDistance);
}

bool
BVH::Leaf::Closest(const pxr::GfVec3f& point, Hit* hit,
  double maxDistance, double* minDistance) const
{
  std::cout << "LEAF CLOSEST ..." << *(pxr::GfRange3d*)this << std::endl;
  if (maxDistance < 0.f || _GetDistance(this, &pxr::GfRange3d(point, point)) < maxDistance) {
    Geometry* geom = (Geometry*)GetGeometry();
    return (this->*_closestImpl)(geom->GetPositionsCPtr(), point, hit, maxDistance, minDistance);
  }
  return false;
}

BVH::Branch::Branch(BVH::Cell* parent)
  : BVH::Cell(parent)
{
  SetLeft(NULL);
  SetRight(NULL);
}


BVH::Branch::Branch(BVH::Cell* parent, BVH::Cell* lhs)
  : BVH::Cell(parent)
{
  SetMin(lhs->GetMin());
  SetMax(lhs->GetMax());
  SetLeft(lhs);
  SetRight(NULL);
  lhs->SetParent(this);
}

BVH::Branch::Branch(BVH::Cell* parent, BVH::Cell* lhs, BVH::Cell* rhs)
  : BVH::Cell(parent)
{
  SetMin(_GetBoundingMinimum(lhs, rhs));
  SetMax(_GetBoundingMaximum(lhs, rhs));
  SetLeft(lhs);
  SetRight(rhs);
  lhs->SetParent(this);
  rhs->SetParent(this);
}

bool BVH::Branch::Raycast(const pxr::GfRay& ray, Hit* hit, 
  double maxDistance, double* minDistance) const
{
  std::cout << "RAYCAST : " << *(pxr::GfRange3d*)this << std::endl;
  std::cout << "RAY : " << ray << std::endl;
  double enterDistance, exitDistance;
  if (ray.Intersect(*this, &enterDistance, &exitDistance)) {
    std::cout << "RAY HIT SOMETINH.." << std::endl;
    Hit leftHit(*hit), rightHit(*hit);
    if (_left)_left->Raycast(ray, &leftHit);
    if (_right)_right->Raycast(ray, &rightHit);
    if (leftHit.GetGeometry() && rightHit.GetGeometry()) {
      if (leftHit.GetT() < rightHit.GetT()) {
        hit->Set(leftHit); return true;
      } else {
        hit->Set(rightHit);; return true;
      }
    } else if (leftHit.GetGeometry()) {
      hit->Set(leftHit); return true;
    } else if (rightHit.GetGeometry()) {
      hit->Set(rightHit); return true;
    } else {
      return false;
    }
  }
  else {
    std::cout << "RAY MISS THE BOX..." << std::endl;
  }
  return false;
}

bool BVH::Branch::Closest(const pxr::GfVec3f& point, Hit* hit,
  double maxDistance, double* minDistance) const
{
  std::cout << "BRANCH CLOSEST ..." << *(pxr::GfRange3d*)this << std::endl;
  std::cout << "MAX DISTANCE : " << maxDistance << std::endl;
  double leftMinDistance, rightMinDistance;
  if (maxDistance < 0 || _GetDistance(this, &pxr::GfRange3d(point, point)) < maxDistance) {
    std::cout << "CLOSEST LEFT RIFGT..." << std::endl;
    Hit leftHit(*hit), rightHit(*hit);
    if (_left)_left->Closest(point, &leftHit, maxDistance);
    if (_right)_right->Closest(point, &rightHit, maxDistance);
    if (leftHit.GetGeometry() && rightHit.GetGeometry()) {
      if (leftHit.GetT() < rightHit.GetT()) {
        hit->Set(leftHit); return true;
      }
      else {
        hit->Set(rightHit); return true;
      }
    } else if(leftHit.GetGeometry()) {
      hit->Set(leftHit); return true;
    } else if (rightHit.GetGeometry()) {
      hit->Set(rightHit); return true;
    } else {
      return false;
    }
  }
  return false;
}

void BVH::_SortCellsByPair(std::vector<BVH::Cell*>& cells,
  std::vector<BVH::Cell*>& results)
{
  size_t numCells = cells.size();
  std::vector<short> used;
  used.resize(numCells);
  memset(&used[0], 0x0, numCells * sizeof(bool));

  for (size_t i = 0; i < numCells; ++i) {
    if (used[i]) continue;
    BVH::Cell* cell = cells[i];
    float min_distance = std::numeric_limits<float>::max();
    int closest_idx = -1;
    for (size_t j = 0; j < numCells; ++j) {
      BVH::Cell* other = cells[j];
      if (i == j)continue;
      float distance = _GetDistance(cell, other);
      if (distance < min_distance && !used[j]) {
        min_distance = distance;
        closest_idx = j;
      }
    }

    if (closest_idx >= 0) {
      used[closest_idx] = true;
      used[i] = true;
      results.push_back(new BVH::Branch(_root, cell, cells[closest_idx]));
    }
    else {
      used[i] = true;
      results.push_back(new BVH::Branch(_root, cell));
    }
  }
}

void BVH::_SortTrianglesByPair(std::vector<BVH::Cell*>& leaves, 
  Geometry* geometry)
{
  if (geometry->GetType() != Geometry::MESH)return;
  Mesh* mesh = (Mesh*)geometry;
  const pxr::GfVec3f* points = geometry->GetPositionsCPtr();

  pxr::VtArray<TrianglePair>& trianglePairs = mesh->GetTrianglePairs();
  leaves.reserve(trianglePairs.size());
  for (auto& trianglePair : trianglePairs) {
    BVH::Leaf* leaf = 
      new BVH::Leaf(_root, &trianglePair, trianglePair.GetBoundingBox(points));
    leaves.push_back(leaf);
  }

  for (auto& leaf : leaves) {
    std::cout << *(pxr::GfRange3d*)leaf << std::endl;
  }
}

void BVH::Init(const std::vector<Geometry*>& geometries)
{
  _root = new BVH::Root(NULL, NULL);
  _elemType = BVH::GEOMETRY;
  std::vector<Cell*> trees;
  trees.reserve(geometries.size());

  for (Geometry* geom : geometries) {
    trees.push_back(new BVH::Leaf(_root, geom));
    std::cout << "TREE BBOX : " << *(pxr::GfRange3d*)trees.back() << std::endl;
  }

  std::vector<BVH::Cell*> cells = trees;
  std::vector<BVH::Cell*> results;
  _SortCellsByPair(trees, results);
  while (results.size() > 2) {
    cells = results;
    results.clear();
    _SortCellsByPair(cells, results);
  }
  if (results.size() == 1) {
    std::cout << "BVH ONE GEOM" << std::endl;
    std::cout << "RESULT BBOX : " << *(pxr::GfRange3d*)results[0] << std::endl;
    _root->SetLeft(results[0]);
    _root->SetMin(results[0]->GetMin());
    _root->SetMax(results[0]->GetMax());
  }
  else {
    std::cout << "BVH MULTI GEOM" << std::endl;
    std::cout << "RESULT BBOX : " << *(pxr::GfRange3d*)results[0] << std::endl;
    std::cout << "RESULT BBOX : " << *(pxr::GfRange3d*)results[1] << std::endl;
    _root->SetLeft(results[0]);
    _root->SetRight(results[1]);
    pxr::GfRange3d range = pxr::GfRange3d::GetUnion(*results[0], *results[1]);
    _root->SetMin(range.GetMin());
    _root->SetMax(range.GetMax());
  }
  std::cout << "BVH BBOX : " << *(pxr::GfRange3d*)_root << std::endl;
  trees.clear();
}

void BVH::Init(Geometry* geometry, Cell* parent)
{
  _root = new BVH::Root(geometry, parent);
  if (geometry->GetType() == Geometry::MESH) {
    _elemType = BVH::TRIPAIR;
    std::vector<Cell*> leaves;
    _SortTrianglesByPair(leaves, geometry);

    std::vector<BVH::Cell*> cells = leaves;
    std::vector<BVH::Cell*> results;
    _SortCellsByPair(leaves, results);
    while (results.size() > 2) {
      cells = results;
      results.clear();
      _SortCellsByPair(cells, results);
    }
    if (results.size() == 1) {
      _root->SetLeft(results[0]);
      _root->SetMin(results[0]->GetMin());
      _root->SetMax(results[0]->GetMax());
    }
    else {
      _root->SetLeft(results[0]);
      _root->SetRight(results[1]);
      pxr::GfRange3d range = pxr::GfRange3d::GetUnion(*results[0], *results[1]);
      _root->SetMin(range.GetMin());
      _root->SetMax(range.GetMax());
    }

    std::cout << "GEOMETRY BBOX : " << *(pxr::GfRange3d*)_root << std::endl;
    leaves.clear();
  }
}

bool BVH::Raycast(const pxr::GfRay& ray, Hit* hit, 
  double maxDistance, double* minDistance) const
{
  return _root->Raycast(ray, hit, maxDistance, minDistance);
}

bool BVH::Closest(const pxr::GfVec3f& point, Hit* hit, 
  double maxDistance, double* minDistance) const
{
  return _root->Closest(point, hit, maxDistance, minDistance);
}


void BVH::Update(const std::vector<Geometry*>& geometries)
{

}

static void _RecurseCells(BVH::Cell* cell, size_t& count, short elemType)
{
  if (cell->IsLeaf()) {
    if (elemType == BVH::TRIPAIR) {
      count++;
    } else if (elemType == BVH::GEOMETRY) {
      BVH::Leaf* leaf = (BVH::Leaf*)cell;
      BVH* bvh = (BVH*)leaf->Get();
      count += bvh->GetNumCells();
    }
  } else {
    BVH::Branch* branch = (BVH::Branch*)cell;
    if (branch->GetLeft()) {
      _RecurseCells(branch->GetLeft(), count, elemType);
    }
    if (branch->GetRight()) {
      _RecurseCells(branch->GetRight(), count, elemType);
    }
  }
}

const pxr::GfRange3d&
BVH::GetBoundingBox() const 
{
  return *_root;
}

pxr::GfRange3d&
BVH::GetBoundingBox()
{
  return *_root;
}

size_t BVH::GetNumCells()
{
  size_t numCells = 0;
  _RecurseCells(_root, numCells, _elemType);
  return numCells;
}

JVR_NAMESPACE_CLOSE_SCOPE