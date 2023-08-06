#include <algorithm>
#include <iostream>
#include <iterator>

#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/ray.h>
#include "../acceleration/bvh.h"
#include "../acceleration/morton.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"

JVR_NAMESPACE_OPEN_SCOPE

static void _SwapCells(BVH::Cell* lhs, BVH::Cell* rhs)
{
  lhs->SetLeft(rhs->GetLeft());
  lhs->SetRight(rhs->GetRight());
  lhs->SetMin(rhs->GetMin());
  lhs->SetMax(rhs->GetMax());
  rhs->SetLeft(NULL);
  rhs->SetRight(NULL);
  delete rhs;
}


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
  , _type(BVH::Cell::GEOM)
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

static double 
GetDistance(const pxr::GfRange3d& range, const pxr::GfVec3d& point)
{
  const pxr::GfVec3d& minimum = range.GetMin();
  const pxr::GfVec3d& maximum = range.GetMax();
  float dx = _GetDistance1D(point[0], minimum[0], maximum[0]);
  float dy = _GetDistance1D(point[1], minimum[1], maximum[1]);
  float dz = _GetDistance1D(point[2], minimum[2], maximum[2]);
  return pxr::GfSqrt(dx * dx + dy * dy + dz * dz);
}

bool
BVH::Cell::IsRoot() const
{
  return _type == BVH::Cell::ROOT;
}

bool
BVH::Cell::IsGeom() const
{
  return _type == BVH::Cell::GEOM;
}


bool
BVH::Cell::IsLeaf() const
{
  return _type == BVH::Cell::LEAF;
}

const BVH::Cell*
BVH::Cell::GetRoot() const
{
  if (IsRoot())return this;
  BVH::Cell* parent = _parent;
  while (parent) {
    if (parent->IsRoot()) return parent;
    parent = parent->_parent;
  }
  return this;
}

const BVH::Cell*
BVH::Cell::GetGeom() const
{
  if (IsGeom())return this;
  BVH::Cell* parent = _parent;
  while (parent) {
    if (parent->IsGeom()) return parent;
    parent = parent->_parent;
  }
  return this;
}

const BVH* 
BVH::Cell::GetIntersector() const
{
  return (BVH*) GetRoot()->GetData();
}

int
BVH::GetGeometryIndex(Geometry* geom) const 
{
  for (size_t index = 0; index < _geometries.size(); ++index) {
    if(_geometries[index] == geom)return static_cast<int>(index);
  }
  return -1;
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
BVH::Cell::GetLeaves(std::vector<BVH::Cell*>& leaves) const
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
BVH::Cell::GetCells(std::vector<BVH::Cell*>& cells) const
{
  cells.clear();
  if (_left)_RecurseGetCells(_left, cells);
  if (_right)_RecurseGetCells(_right, cells);
}

Geometry*
BVH::Cell::GetGeometry()
{
  const BVH::Cell* geom = GetGeom();
  if (geom) {
    return (Geometry*)geom->_data;
  }
  else return NULL;
}

bool
BVH::Cell::Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
  double maxDistance, double* minDistance) const
{
  double enterDistance, exitDistance;
  if (!ray.Intersect(*this, &enterDistance, &exitDistance))
    return false;

  if(enterDistance > maxDistance) 
    return false;

  if (IsLeaf()) {
    Component* component = (Component*)_data;
    return component->Raycast(points, ray, hit, maxDistance, minDistance);
  } else {
    if(IsGeom()) {        
      const BVH* intersector = GetIntersector();
      hit->SetGeometryIndex(intersector->GetGeometryIndex((Geometry*)_data));
    }
    Hit leftHit(*hit), rightHit(*hit);
    if (_left)_left->Raycast(points, ray, &leftHit, maxDistance, minDistance);
    if (_right)_right->Raycast(points, ray, &rightHit, maxDistance, minDistance);

    if (leftHit.HasHit() && rightHit.HasHit()) {
      if (leftHit.GetT() < rightHit.GetT()) {
        hit->Set(leftHit); 
        return true;
      }
      else {
        hit->Set(rightHit); 
        return true;
      }
    }
    else if (leftHit.HasHit()) {
      hit->Set(leftHit); 
      return true;
    }
    else if (rightHit.HasHit()) {
      hit->Set(rightHit); 
      return true;
    }
  }
  return false;
}

bool 
BVH::Cell::Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, 
  Hit* hit, double maxDistance, double* minDistance) const
{  
  if(IsLeaf()) {
    std::cout << "this is fuckin leaf " << std::endl;
    Component* component = (Component*)_data;
    return component->Closest(points, point, hit, maxDistance, minDistance);
  } else {
    std::cout << "this is fuckin branch " << std::endl;
    const pxr::GfVec3d offset(maxDistance);
    const pxr::GfRange3d range(point - offset, point + offset);
    bool leftHit = false;
    bool rightHit = false;
    if(_left && (_left->Contains(point) || !_left->IntersectWith(range).IsEmpty())) {
      leftHit = _left->Closest(points, point, hit, maxDistance, minDistance);
    }
    if(_right && (_right->Contains(point) || !_right->IntersectWith(range).IsEmpty())) {
      rightHit = _right->Closest(points, point, hit, maxDistance, minDistance);
    }
    return leftHit || rightHit;
  }
}

void BVH::Cell::_SortTrianglesByPair(std::vector<Morton>& leaves, Geometry* geometry)
{
  if (geometry->GetType() != Geometry::MESH)return;
  Mesh* mesh = (Mesh*)geometry;
  const pxr::GfVec3f* points = geometry->GetPositionsCPtr();

  pxr::VtArray<TrianglePair>& trianglePairs = mesh->GetTrianglePairs();
  leaves.reserve(trianglePairs.size());
  for (auto& trianglePair : trianglePairs) {
    BVH::Cell* leaf =
      new BVH::Cell(this, &trianglePair, trianglePair.GetBoundingBox(points));

    const BVH::Cell* root = GetRoot();
    BVH::ComputeCode(root, leaf->GetMidpoint());
    leaves.push_back({ BVH::ComputeCode(root, leaf->GetMidpoint()), leaf });
  }
}

int 
_FindSplit(const std::vector<Morton>& mortons,  int first, int last)
{
  uint64_t firstCode = mortons[first].code;
  uint64_t lastCode = mortons[last].code;

  if (firstCode == lastCode)
    return (first + last) >> 1;

  int commonPrefix = MortonLeadingZeros(firstCode ^ lastCode);
  int split = first;
  int step = last - first;

  do
  {
    step = (step + 1) >> 1;
    int newSplit = split + step;

    if (newSplit < last)
    {
      uint64_t splitCode = mortons[newSplit].code;
      int splitPrefix = MortonLeadingZeros(firstCode ^ splitCode);
      if (splitPrefix > commonPrefix) {
        split = newSplit;
      }
    }
  } while (step > 1);

  return split;
}

BVH::Cell*
BVH::Cell::_RecurseSortCellsByPair(
  std::vector<Morton>& mortons,
  int           first,
  int           last)
{
  if (first == last)
    return (BVH::Cell*)mortons[first].data;

  int split = _FindSplit(mortons, first, last);

  BVH::Cell* left = _RecurseSortCellsByPair(mortons, first, split);
  BVH::Cell* right = _RecurseSortCellsByPair(mortons, split + 1, last);
  return new BVH::Cell(this, left, right);
}

void 
BVH::_BuildCellMortons()
{
  std::vector<BVH::Cell*> cells;
  _root.GetCells(cells);
  size_t numCells = cells.size();
  _mortons.resize(numCells);
  for (size_t cellIdx = 0; cellIdx < numCells; ++cellIdx) {
    BVH::Cell* cell = cells[cellIdx];
    const pxr::GfVec3i p = WorldToMorton(*this, cell->GetMidpoint());
    _mortons[cellIdx].code = MortonEncode3D(p);
    _mortons[cellIdx].data = (void*)cells[cellIdx];
  }

  std::sort(_mortons.begin(), _mortons.end());
}

Morton BVH::Cell::SortCellsByPair(
  std::vector<Morton>& cells)
{
  size_t numCells = cells.size();
  std::vector<Morton> mortons(numCells);
  for (size_t cellIdx = 0; cellIdx < numCells; ++cellIdx) {
    BVH::Cell* cell = (BVH::Cell*)cells[cellIdx].data;
    const pxr::GfVec3i p = WorldToMorton(*this, cell->GetMidpoint());
    mortons[cellIdx].code = MortonEncode3D(p);
    mortons[cellIdx].data = cells[cellIdx].data;
  }

  std::sort(mortons.begin(), mortons.end());
  return { 0, _RecurseSortCellsByPair(mortons, 0, static_cast<int>(numCells) - 1) };
} 

void BVH::Cell::_FinishSort(std::vector<Morton>& cells)
{
  Morton morton = SortCellsByPair(cells);
  _SwapCells(this, (BVH::Cell*)morton.data);
  cells.clear();
}

void BVH::Cell::Init(Geometry* geometry)
{
  _type = BVH::Cell::GEOM;
  if (geometry->GetType() == Geometry::MESH) {
    std::vector<Morton> leaves;
    _SortTrianglesByPair(leaves, geometry);
    _FinishSort(leaves);
  }
}

void
BVH::Init(const std::vector<Geometry*>& geometries)
{
  _geometries = geometries;
  size_t numColliders = _geometries.size();
  pxr::GfRange3d accum = _geometries[0]->GetBoundingBox().GetRange();
  for (size_t i = 1; i < numColliders; ++i) {
    accum.UnionWith(_geometries[i]->GetBoundingBox().GetRange());
  }
  SetMin(accum.GetMin());
  SetMax(accum.GetMax());

  std::vector<Morton> cells;
  cells.reserve(numColliders);

  for (Geometry* geom : _geometries) {
    BVH::Cell* bvh = new BVH::Cell(&_root, geom);
    cells.push_back({ BVH::ComputeCode(&_root, bvh->GetMidpoint()), bvh });
  }

  Morton morton = _root.SortCellsByPair(cells);
  BVH::Cell* cell = (BVH::Cell*)morton.data;
  _root.SetLeft(cell);
  _root.SetMin(cell->GetMin());
  _root.SetMax(cell->GetMax());
  _root.SetData((void*)this);
  cells.clear();
  _BuildCellMortons();
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

bool BVH::Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, 
  Hit* hit, double maxDistance) const
{
  double minDistance = FLT_MAX;
  return _root.Closest(points, point, hit, maxDistance, &minDistance);

  /*
  size_t index = 0;
  for(auto& leaf: leaves) {
    const pxr::GfVec3i p = WorldToMorton(_root, leaf->GetMidpoint());
    uint64_t code = MortonEncode3D(p2);
    if(code >= code1 && code <= code2) {
      //leaf->Closest(points, point, hit, maxDistance, &minDistance);
    }
     //if(leaf->IsInside(point) || GetDistance(&range, leaf) < hit->GetT()) {
    //if(leaf->Closest(points, point, hit, maxDistance, &minDistance))return true;
    //std::cout << "check cell id " << (index++) << std::endl;
    //}
  }
  return hit->HasHit();
  */
}




JVR_NAMESPACE_CLOSE_SCOPE