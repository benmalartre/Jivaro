#include <algorithm>
#include <iostream>
#include <iterator>

#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/ray.h>
#include "../acceleration/bvh.h"
#include "../acceleration/mortom.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"


#ifdef WIN32
    #include <intrin.h>
    static uint32_t __inline __builtin_clz(uint32_t x) {
        unsigned long r = 0;
        _BitScanReverse(&r, x);
        return (31-r);
    }
#endif

JVR_NAMESPACE_OPEN_SCOPE

BVH::Cell::Cell()
  : _parent(NULL)
  , _left(NULL)
  , _right(NULL)
  , _data(NULL)
  , _type(BVH::Cell::ROOT)
{
}

BVH::Cell::Cell(BVH::Cell* parent, Geometry* geometry)
  : _parent(parent)
  , _left(NULL)
  , _right(NULL)
  , _data(NULL)
  , _type(BVH::Cell::ROOT)
{
  if (geometry) {
    const pxr::GfRange3d& range = geometry->GetBoundingBox().GetRange();
    SetMin(range.GetMin());
    SetMax(range.GetMax());

    Init(geometry);
    _data = (void*)geometry;
  }
}

BVH::Cell::Cell(BVH::Cell* parent, BVH::Cell* lhs, BVH::Cell* rhs)
  : _parent(parent)
  , _left(lhs)
  , _right(rhs)
  , _data(NULL)
  , _type(BVH::Cell::BRANCH)
{
  _type = BVH::Cell::BRANCH;
  if (_left && _right) {
    const pxr::GfRange3d range = pxr::GfRange3d::GetUnion(*_left, *_right);
    SetMin(range.GetMin());
    SetMax(range.GetMax());
  } else if (_left) {
    SetMin(_left->GetMin());
    SetMax(_left->GetMax());
  }
}


BVH::Cell::Cell(BVH::Cell* parent, Component* component, const pxr::GfRange3d& range)
  : _parent(parent)
  , _left(NULL)
  , _right(NULL)
  , _data((void*)component)
  , _type(BVH::Cell::LEAF)
{
  SetMin(range.GetMin());
  SetMax(range.GetMax());
}

uint32_t _CountLeadingZeros(const uint64_t x)
{
  uint32_t u32 = (x >> 32);
  uint32_t result = u32 ? __builtin_clz(u32) : 32;
  if (result == 32) {
    u32 = x & 0xFFFFFFFFUL;
    result += (u32 ? __builtin_clz(u32) : 32);
  }
  return result;
}

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
GetDistance(const pxr::GfRange3d* lhs, const pxr::GfRange3d* rhs)
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

bool
BVH::Cell::IsRoot() const
{
  return _type == BVH::Cell::ROOT;
}

bool
BVH::Cell::IsLeaf() const
{
  return _type == BVH::Cell::LEAF;
}

BVH::Cell*
BVH::Cell::GetRoot()
{
  if (IsRoot())return this;
  BVH::Cell* parent = _parent;
  while (parent) {
    if (parent->IsRoot()) return parent;
    parent = parent->_parent;
  }
  return this;
}

static void 
_RecurseGetLeaves(BVH::Cell* cell, std::vector<BVH::Cell*>& leaves)
{
  if (cell->IsLeaf()) {
    leaves.push_back(cell);
  }
  else {
    if (cell->GetLeft()) {
      _RecurseGetLeaves(cell->GetLeft(), leaves);
    }
    if (cell->GetRight()) {
      _RecurseGetLeaves(cell->GetRight(), leaves);
    }
  }
}

void
BVH::Cell::GetLeaves(std::vector<BVH::Cell*>& leaves)
{
  leaves.clear();
  if (_left)_RecurseGetLeaves(_left, leaves);
  if(_right)_RecurseGetLeaves(_right, leaves);
}

static void
_RecurseGetCells(BVH::Cell* cell, std::vector<BVH::Cell*>& leaves)
{
  leaves.push_back(cell);
  if (cell->GetLeft()) {
    _RecurseGetCells(cell->GetLeft(), leaves);
  }
  if (cell->GetRight()) {
    _RecurseGetCells(cell->GetRight(), leaves);
  }
}

void
BVH::Cell::GetCells(std::vector<BVH::Cell*>& cells)
{
  cells.clear();
  if (_left)_RecurseGetCells(_left, cells);
  if (_right)_RecurseGetCells(_right, cells);
}

Geometry*
BVH::Cell::GetGeometry()
{
  BVH::Cell* root = GetRoot();
  if (root) {
    return (Geometry*)root->_data;
  }
  else return NULL;
}

bool
BVH::Cell::Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
  double maxDistance, double* minDistance) const
{
  double enterDistance, exitDistance;
  if (ray.Intersect(*this, &enterDistance, &exitDistance)) {
    if (IsLeaf()) {
      Component* component = (Component*)_data;
      return component->Raycast(points, ray, hit, maxDistance, minDistance);
    }
    else {
      Hit leftHit(*hit), rightHit(*hit);
      if (_left)_left->Raycast(points, ray, &leftHit);
      if (_right)_right->Raycast(points, ray, &rightHit);

      if (leftHit.HasHit() && rightHit.HasHit()) {
        if (leftHit.GetT() < rightHit.GetT()) {
          hit->Set(leftHit); return true;
        }
        else {
          hit->Set(rightHit);; return true;
        }
      }
      else if (leftHit.HasHit()) {
        hit->Set(leftHit); return true;
      }
      else if (rightHit.HasHit()) {
        hit->Set(rightHit); return true;
      }
      else {
        return false;
      }
    }
  }
  return false;
  
}

bool 
BVH::Cell::Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, 
  Hit* hit, double maxDistance) const
{
  double leftMinDistance, rightMinDistance;
  pxr::GfRange3d range(point, point);
  if (maxDistance < 0 || GetDistance(this, &range) < maxDistance) {
    if (IsLeaf()) {
      Component* component = (Component*)_data;
      return component->Closest(points, point, hit, maxDistance);
    }
    else {
      Hit leftHit, rightHit;
      if (_left)_left->Closest(points, point, &leftHit, maxDistance);
      if (_right)_right->Closest(points, point, &rightHit, maxDistance);
      if (leftHit.HasHit() && rightHit.HasHit()) {
        if (leftHit.GetT() < rightHit.GetT()) {
          hit->Set(leftHit); return true;
        }
        else {
          hit->Set(rightHit); return true;
        }
      }
      else if (leftHit.HasHit()) {
        hit->Set(leftHit); return true;
      }
      else if (rightHit.HasHit()) {
        hit->Set(rightHit); return true;
      }
      else {
        return false;
      }
    }
  }
  return false;
}

void BVH::Cell::_SortTrianglesByPair(std::vector<Mortom>& leaves, Geometry* geometry)
{
  if (geometry->GetType() != Geometry::MESH)return;
  Mesh* mesh = (Mesh*)geometry;
  const pxr::GfVec3f* points = geometry->GetPositionsCPtr();

  pxr::VtArray<TrianglePair>& trianglePairs = mesh->GetTrianglePairs();
  leaves.reserve(trianglePairs.size());
  for (auto& trianglePair : trianglePairs) {
    BVH::Cell* leaf =
      new BVH::Cell(this, &trianglePair, trianglePair.GetBoundingBox(points));
    GetRoot();
    BVH::ComputeCode(GetRoot(), leaf->GetMidpoint());
    leaves.push_back({ BVH::ComputeCode(GetRoot(), leaf->GetMidpoint()), leaf });
  }
}

int _FindSplit(std::vector<Mortom>& mortoms,  int first, int last)
{
  uint64_t firstCode = mortoms[first].code;
  uint64_t lastCode = mortoms[last].code;

  if (firstCode == lastCode)
    return (first + last) >> 1;

  int commonPrefix = _CountLeadingZeros(firstCode ^ lastCode);
  int split = first;
  int step = last - first;

  do
  {
    step = (step + 1) >> 1;
    int newSplit = split + step;

    if (newSplit < last)
    {
      uint64_t splitCode = mortoms[newSplit].code;
      int splitPrefix = _CountLeadingZeros(firstCode ^ splitCode);
      if (splitPrefix > commonPrefix) {
        split = newSplit;
      }
    }
  } while (step > 1);

  return split;
}

void
BVH::SetRoot(BVH::Cell* cell)
{
  _root.SetLeft(cell->GetLeft());
  _root.SetRight(cell->GetRight());
  _root.SetMin(cell->GetMin());
  _root.SetMax(cell->GetMax());
  cell->SetLeft(NULL);
  cell->SetRight(NULL);
  delete cell;
}

BVH::Cell*
BVH::Cell::_RecurseSortCellsByPair(
  std::vector<Mortom>& mortoms,
  int           first,
  int           last)
{
  if (first == last)
    return (BVH::Cell*)mortoms[first].cell;

  int split = _FindSplit(mortoms, first, last);

  BVH::Cell* left = _RecurseSortCellsByPair(mortoms, first, split);
  BVH::Cell* right = _RecurseSortCellsByPair(mortoms, split + 1, last);
  return new BVH::Cell(this, left, right);
}

Mortom BVH::Cell::SortCellsByPair(
  std::vector<Mortom>& cells)
{

  size_t numCells = cells.size();

  std::vector<Mortom> mortoms(numCells);
  for (size_t cellIdx = 0; cellIdx < numCells; ++cellIdx) {
    BVH::Cell* cell = (BVH::Cell*)cells[cellIdx].cell;
    const pxr::GfVec3i p = WorldToMortom(*this, cell->GetMidpoint());
    mortoms[cellIdx].code = Encode3D(p);
    mortoms[cellIdx].cell = cells[cellIdx].cell;
  }

  std::sort(mortoms.begin(), mortoms.end());
  return { 0, _RecurseSortCellsByPair(mortoms, 0, numCells - 1) };
} 

static void _SwapCells(BVH::Cell* lhs, BVH::Cell* rhs)
{
  lhs->SetLeft(rhs->GetLeft());
  lhs->SetRight(rhs->GetRight());
  lhs->SetMin(rhs->GetMin());
  lhs->SetMax(rhs->GetMax());
  delete rhs;
}

void BVH::Cell::Init(const std::vector<Geometry*>& geometries)
{
  size_t numColliders = geometries.size();
  pxr::GfRange3d accum = geometries[0]->GetBoundingBox().GetRange();
  for (size_t i = 1; i < numColliders; ++i) {
    accum.UnionWith(geometries[i]->GetBoundingBox().GetRange());
  }
  SetMin(accum.GetMin());
  SetMax(accum.GetMax());

  std::vector<Mortom> cells;
  cells.reserve(numColliders);

  for (Geometry* geom : geometries) {
    BVH::Cell* bvh = new BVH::Cell(this, geom);
    cells.push_back({ BVH::ComputeCode(bvh, bvh->GetMidpoint()), bvh });
  }

  Mortom result = SortCellsByPair(cells);
  _SwapCells(this, (BVH::Cell*)result.cell);

  cells.clear();
}

void BVH::Cell::_FinishSort(std::vector<Mortom>& cells)
{
  Mortom result = SortCellsByPair(cells);
  _SwapCells(this, (BVH::Cell*)result.cell);
  cells.clear();
}

void BVH::Cell::Init(Geometry* geometry)
{
  if (geometry->GetType() == Geometry::MESH) {
    std::vector<Mortom> leaves;
    _SortTrianglesByPair(leaves, geometry);
    _FinishSort(leaves);
  }
}

void
BVH::Init(const std::vector<Geometry*>& geometries)
{
  size_t numColliders = geometries.size();
  pxr::GfRange3d accum = geometries[0]->GetBoundingBox().GetRange();
  for (size_t i = 1; i < numColliders; ++i) {
    accum.UnionWith(geometries[i]->GetBoundingBox().GetRange());
  }
  SetMin(accum.GetMin());
  SetMax(accum.GetMax());

  std::vector<Mortom> cells;
  cells.reserve(numColliders);

  for (Geometry* geom : geometries) {
    BVH::Cell* bvh = new BVH::Cell(&_root, geom);
    cells.push_back({ BVH::ComputeCode(&_root, bvh->GetMidpoint()), bvh });
  }

  Mortom mortom = _root.SortCellsByPair(cells);
  _SwapCells(&_root, (BVH::Cell*)mortom.cell);
  cells.clear();
}

void
BVH::Update(const std::vector<Geometry*>& geometries)
{

}

bool BVH::Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
  double maxDistance, double* minDistance) const
{
  return _root.Raycast(points, ray, hit, maxDistance, minDistance);
};

bool BVH::Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
  double maxDistance) const
{
  return _root.Closest(points, point, hit, maxDistance);
}




JVR_NAMESPACE_CLOSE_SCOPE