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

static size_t NUM_HITS = 0;

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
BVH::IsRoot() const
{
  return _type == BVH::ROOT;
}

bool
BVH::IsLeaf() const
{
  return _type == BVH::LEAF;
}

BVH* 
BVH::GetRoot()
{
  if (IsRoot())return this;
  BVH* parent = _parent;
  while (parent) {
    if (parent->IsRoot()) return parent;
    parent = parent->_parent;
  }
  return this;
}

const BVH*
BVH::GetRoot() const
{
  if (IsRoot())return this;
  BVH* parent = _parent;
  while (parent) {
    if (parent->IsRoot()) return parent;
    parent = parent->_parent;
  }
  return this;
}

static void 
_RecurseGetLeaves(BVH* cell, std::vector<BVH*>& leaves)
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
BVH::GetLeaves(std::vector<BVH*>& leaves)
{
  leaves.clear();
  if (_left)_RecurseGetLeaves(_left, leaves);
  if(_right)_RecurseGetLeaves(_right, leaves);
}

static void
_RecurseGetCells(BVH* cell, std::vector<BVH*>& leaves)
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
BVH::GetCells(std::vector<BVH*>& cells)
{
  cells.clear();
  if (_left)_RecurseGetCells(_left, cells);
  if (_right)_RecurseGetCells(_right, cells);
}

static void _RecurseGetNumCells(BVH* cell, size_t& count, short elemType)
{
  if (cell->IsLeaf()) {
    if (elemType == BVH::TRIPAIR) {
      count++;
    }
    else if (elemType == BVH::GEOMETRY) {
      BVH* leaf = (BVH*)cell;
    }
  }
  else {
    if (cell->GetLeft()) {
      _RecurseGetNumCells(cell->GetLeft(), count, elemType);
    }
    if (cell->GetRight()) {
      _RecurseGetNumCells(cell->GetRight(), count, elemType);
    }
  }
}

size_t
BVH::GetNumCells()
{
  size_t numCells = 0;
  _RecurseGetNumCells(this, numCells, BVH::GEOMETRY);
  return numCells;
}


static short 
_GetElementTypeFromGeometry(Geometry* geom)
{
  if (geom) {
    switch (geom->GetType()) {
    case Geometry::MESH:
      return BVH::TRIPAIR;
    case Geometry::CURVE:
      return BVH::SEGMENT;
    case Geometry::POINT:
      return BVH::POINT;
    }
  }
  return BVH::INVALID;
}


BVH::BVH(BVH* parent, Geometry* geometry)
  : _parent(parent)
  , _left(NULL)
  , _right(NULL)
  , _data(NULL)
  , _type(BVH::ROOT)
{
  if (geometry) {
    const pxr::GfRange3d& range = geometry->GetBoundingBox().GetRange();
    SetMin(range.GetMin());
    SetMax(range.GetMax());

    Init(geometry);

    _data = (void*)geometry;
  }
}

BVH::BVH(BVH* parent, BVH* lhs, BVH* rhs)
  : _parent(parent)
  , _left(lhs)
  , _right(rhs)
  , _data(NULL)
  , _type(BVH::BRANCH)
{
  _type = BVH::BRANCH;
  if (_left && _right) {
    const pxr::GfRange3d range = pxr::GfRange3d::GetUnion(*_left, *_right);
    SetMin(range.GetMin());
    SetMax(range.GetMax());
  }
  else if (_left) {
    SetMin(_left->GetMin());
    SetMax(_left->GetMax());
  }
  ComputeCode(GetMidpoint());
}


BVH::BVH(BVH* parent, TrianglePair* pair, const pxr::GfRange3d& range)
  :  _parent(parent)
  , _left(NULL)
  , _right(NULL)
  , _data((void*)pair)
  , _type(BVH::LEAF)
{
  SetMin(range.GetMin());
  SetMax(range.GetMax());
}

BVH::BVH(BVH* parent, Triangle* tri, const pxr::GfRange3d& range)
  : _parent(parent)
  , _left(NULL)
  , _right(NULL)
  , _data((void*)tri)
  , _type(BVH::LEAF)
{
  SetMin(range.GetMin());
  SetMax(range.GetMax());
}

Geometry*
BVH::GetGeometry()
{
  BVH* root = GetRoot();
  if (root) {
    return (Geometry*)root->_data;
  }
  else return NULL;
}

Geometry*
BVH::GetGeometry() const
{
  const BVH* root = GetRoot();
  if (root) {
    return (Geometry*)root->_data;
  }
  else return NULL;
}

bool
BVH::_RaycastTrianglePair(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
  double maxDistance, double* minDistance) const
{
  TrianglePair* pair = (TrianglePair*)_data;
  return pair->Raycast(points, ray, hit, maxDistance, minDistance);
}

bool 
BVH::_ClosestTrianglePair(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
  double maxDistance, double* minDistance) const
{
  TrianglePair* pair = (TrianglePair*)_data;
  return pair->Closest(points, point, hit, maxDistance, minDistance);
}

bool BVH::Raycast(const pxr::GfRay& ray, Hit* hit, 
  double maxDistance, double* minDistance) const
{
  NUM_HITS++;
  double enterDistance, exitDistance;
  if (ray.Intersect(*this, &enterDistance, &exitDistance)) {
    if (IsLeaf()) {
      Geometry* geometry = GetGeometry();
      const pxr::GfVec3f* points = geometry->GetPositionsCPtr();
      switch (geometry->GetType()) {
      case Geometry::MESH:
        if (_RaycastTrianglePair(points, ray, hit, maxDistance, minDistance)) {
          hit->SetGeometry(geometry);
          return true;
        }
        break;
      default:
        return false;
        break;
      }
    }
    else {
      Hit leftHit(*hit), rightHit(*hit);
      if (_left)_left->Raycast(ray, &leftHit);
      if (_right)_right->Raycast(ray, &rightHit);

      if (leftHit.GetGeometry() && rightHit.GetGeometry()) {
        if (leftHit.GetT() < rightHit.GetT()) {
          hit->Set(leftHit); return true;
        }
        else {
          hit->Set(rightHit);; return true;
        }
      }
      else if (leftHit.GetGeometry()) {
        hit->Set(leftHit); return true;
      }
      else if (rightHit.GetGeometry()) {
        hit->Set(rightHit); return true;
      }
      else {
        return false;
      }
    }
  }
  return false;
}

bool BVH::Closest(const pxr::GfVec3f& point, Hit* hit,
  double maxDistance, double* minDistance) const
{
  double leftMinDistance, rightMinDistance;
  pxr::GfRange3d range(point, point);
  if (maxDistance < 0 || GetDistance(this, &range) < maxDistance) {
    if (IsLeaf()) {
      Geometry* geometry = GetGeometry();
      const pxr::GfVec3f* points = geometry->GetPositionsCPtr();
      switch (geometry->GetType()) {
      case Geometry::MESH:
        if (_ClosestTrianglePair(points, point, hit, maxDistance, minDistance)) {
          hit->SetGeometry(geometry);
          return true;
        }
        break;
      default:
        return false;
        break;
      }
    }
    else {
      Hit leftHit(*hit), rightHit(*hit);
      if (_left)_left->Closest(point, &leftHit, maxDistance);
      if (_right)_right->Closest(point, &rightHit, maxDistance);
      if (leftHit.GetGeometry() && rightHit.GetGeometry()) {
        if (leftHit.GetT() < rightHit.GetT()) {
          hit->Set(leftHit); return true;
        } else {
          hit->Set(rightHit); return true;
        }
      } else if (leftHit.GetGeometry()) {
        hit->Set(leftHit); return true;
      } else if (rightHit.GetGeometry()) {
        hit->Set(rightHit); return true;
      } else {
        return false;
      }
    }
  }
  return false;
}

void BVH::_SortTrianglesByPair(std::vector<Mortom>& leaves, Geometry* geometry)
{
  if (geometry->GetType() != Geometry::MESH)return;
  Mesh* mesh = (Mesh*)geometry;
  const pxr::GfVec3f* points = geometry->GetPositionsCPtr();

  pxr::VtArray<TrianglePair>& trianglePairs = mesh->GetTrianglePairs();
  leaves.reserve(trianglePairs.size());
  for (auto& trianglePair : trianglePairs) {
    BVH* leaf =
      new BVH(this, &trianglePair, trianglePair.GetBoundingBox(points));
    leaves.push_back({ this->ComputeCode(leaf->GetMidpoint()), leaf });
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

BVH*
BVH::_RecurseSortCellsByPair(
  std::vector<Mortom>& mortoms,
  int           first,
  int           last)
{
  if (first == last)
    return mortoms[first].cell;

  int split = _FindSplit(mortoms, first, last);

  BVH* left = _RecurseSortCellsByPair(mortoms, first, split);
  BVH* right = _RecurseSortCellsByPair(mortoms, split + 1, last);
  return new BVH(this, left, right);
}

Mortom BVH::_SortCellsByPair(
  std::vector<Mortom>& cells)
{

  size_t numCells = cells.size();

  std::vector<Mortom> mortoms(numCells);
  for (size_t cellIdx = 0; cellIdx < numCells; ++cellIdx) {
    const pxr::GfVec3i p = WorldToMortom(*this, cells[cellIdx].cell->GetMidpoint());
    mortoms[cellIdx].code = Encode3D(p);
    mortoms[cellIdx].cell = cells[cellIdx].cell;
  }

  std::sort(mortoms.begin(), mortoms.end());
  return { 0, _RecurseSortCellsByPair(mortoms, 0, numCells - 1) };
} 

void BVH::Init(const std::vector<Geometry*>& geometries)
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
    BVH* bvh = new BVH(this, geom);
    cells.push_back({ bvh->ComputeCode(bvh->GetMidpoint()), bvh });
  }

  Mortom result = _SortCellsByPair(cells);

  SetLeft(result.cell);
  SetRight(NULL);

  SetMin(result.cell->GetMin());
  SetMax(result.cell->GetMax());
  
  cells.clear();
}

void BVH::Init(Geometry* geometry)
{
  if (geometry->GetType() == Geometry::MESH) {
    std::vector<Mortom> leaves;
    _SortTrianglesByPair(leaves, geometry);

    Mortom result = _SortCellsByPair(leaves);

    SetLeft(result.cell);
    SetMin(result.cell->GetMin());
    SetMax(result.cell->GetMax());
  
    leaves.clear();
  }
}

void BVH::Update(const std::vector<Geometry*>& geometries)
{

}

uint64_t 
BVH::ComputeCode(const pxr::GfVec3d& point)
{
  const pxr::GfVec3i p = WorldToMortom(*GetRoot(), point);
  return Encode3D(p);
}

pxr::GfVec3d
BVH::ComputeCodeAsColor(const pxr::GfVec3d& point)
{
  uint64_t mortom = ComputeCode(point);
  pxr::GfVec3i p = Decode3D(mortom);
  return pxr::GfVec3d(p[0]/(float)MORTOM_MAX_L, p[1] / (float)MORTOM_MAX_L, p[2] / (float)MORTOM_MAX_L);
}

JVR_NAMESPACE_CLOSE_SCOPE