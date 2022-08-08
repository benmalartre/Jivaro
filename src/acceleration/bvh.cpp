#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/ray.h>
#include "../acceleration/bvh.h"
#include "../acceleration/mortom.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"

JVR_NAMESPACE_OPEN_SCOPE

static size_t NUM_HITS = 0;

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
  return parent;
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
  return parent;
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

BVH::BVH()
  : _parent(NULL)
  , _left(NULL)
  , _right(NULL)
  , _data(NULL)
  , _type(BVH::ROOT)
{
}

BVH::BVH(BVH* parent)
  : _parent(parent)
  , _left(NULL)
  , _right(NULL)
  , _data(NULL)
  , _type(BVH::ROOT)
{
}

BVH::BVH(BVH* parent, Geometry* geometry)
  : _parent(parent)
  , _left(NULL)
  , _right(NULL)
  , _data(NULL)
  , _type(BVH::ROOT)
{
  std::cout << "BVH LEAF CONSTRUCTOR" << std::endl;
  if (geometry) {
    pxr::GfRange3d range = geometry->GetBoundingBox().GetRange();
    std::cout << "BBOX : " << range << std::endl;
    SetMin(range.GetMin());
    SetMax(range.GetMax());

    Init(geometry, parent);

    SetMin(GetBoundingBox().GetMin());
    SetMax(GetBoundingBox().GetMax());
    _data = (void*)new BVH::Data({
      geometry, 
      _GetElementTypeFromGeometry(geometry)
      });
  }
}

BVH::BVH(BVH* parent, BVH* lhs)
  : _parent(parent)
  , _left(lhs)
  , _right(NULL)
  , _data(NULL)
  , _type(BVH::BRANCH)
{
  SetMin(lhs->GetMin());
  SetMax(lhs->GetMax());
  lhs->SetParent(this);
}

BVH::BVH(BVH* parent, BVH* lhs, BVH* rhs)
  : _parent(parent)
  , _left(lhs)
  , _right(rhs)
  , _data(NULL)
  , _type(BVH::BRANCH)
{
  _type = BVH::BRANCH;
  SetMin(_GetBoundingMinimum(lhs, rhs));
  SetMax(_GetBoundingMaximum(lhs, rhs));
  lhs->SetParent(this);
  rhs->SetParent(this);
}


BVH::BVH(BVH* parent, TrianglePair* pair, const pxr::GfRange3d& range)
  : _parent(parent)
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
    BVH::Data* data = (BVH::Data*)root->_data;
    return data->geometry;
  }
  else return NULL;
}

const Geometry*
BVH::GetGeometry() const
{
  const BVH* root = GetRoot();
  if (root) {
    BVH::Data* data = (BVH::Data*)root->_data;
    return data->geometry;
  }
  else return NULL;
}

short
BVH::GetElementType()
{
  BVH* root = GetRoot();
  if (root) {
    BVH::Data* data = (BVH::Data*)root->_data;
    return data->elemType;
  }
  return BVH::INVALID;
}

bool
BVH::_RaycastGeometry(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
  double maxDistance, double* minDistance) const
{
  //BVH* bvh = (BVH*)_data;
  //return bvh->Raycast(ray, hit, maxDistance, minDistance);
  return false;
}

bool
BVH::_RaycastTrianglePair(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
  double maxDistance, double* minDistance) const
{
  TrianglePair* pair = (TrianglePair*)_data;
  return pair->Raycast(points, ray, hit, maxDistance, minDistance);
}

bool 
BVH::_ClosestGeometry(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
  double maxDistance, double* minDistance) const
{
  BVH* bvh = (BVH*)_data;
  return bvh->Closest(point, hit, maxDistance, minDistance);
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
      BVH::Data* data = (BVH::Data*)GetRoot()->_data;
      const pxr::GfVec3f* points = data->geometry->GetPositionsCPtr();
      switch (data->elemType) {
      case BVH::TRIPAIR:
        if (_RaycastTrianglePair(points, ray, hit, maxDistance, minDistance)) {
          hit->SetGeometry(data->geometry);
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
  if (maxDistance < 0 || _GetDistance(this, &pxr::GfRange3d(point, point)) < maxDistance) {
    if (IsLeaf()) {
      BVH::Data* data = (BVH::Data*)GetRoot()->_data;
      const pxr::GfVec3f* points = data->geometry->GetPositionsCPtr();
      switch (data->elemType) {
      case BVH::TRIPAIR:
        if (_ClosestTrianglePair(points, point, hit, maxDistance, minDistance)) {
          hit->SetGeometry(data->geometry);
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

void BVH::_SortCellsByPair(std::vector<BVH*>& cells,
  std::vector<BVH*>& results)
{
  size_t numCells = cells.size();
  std::vector<short> used;
  used.resize(numCells);
  memset(&used[0], 0x0, numCells * sizeof(bool));
  /*
  std::vector<uint64_t> mortom(numCells);
  for (size_t i = 0; i < numCells; ++i) {
    const MortomPoint3d p = WorldToMortom(*this, cells[i]->GetMidpoint());
    mortom[i] = Encode3D(p);
  }
  */
  for (size_t i = 0; i < numCells; ++i) {
    if (used[i]) continue;
    BVH* cell = cells[i];
    float min_distance = std::numeric_limits<float>::max();
    int closest_idx = -1;
    for (size_t j = 0; j < numCells; ++j) {
      BVH* other = cells[j];
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
      results.push_back(new BVH(this, cell, cells[closest_idx]));
    }
    else {
      used[i] = true;
      results.push_back(new BVH(this, cell));
    }
  }
}

void BVH::_SortTrianglesByPair(std::vector<BVH*>& leaves,  Geometry* geometry)
{
  if (geometry->GetType() != Geometry::MESH)return;
  Mesh* mesh = (Mesh*)geometry;
  const pxr::GfVec3f* points = geometry->GetPositionsCPtr();

  pxr::VtArray<TrianglePair>& trianglePairs = mesh->GetTrianglePairs();
  leaves.reserve(trianglePairs.size());
  for (auto& trianglePair : trianglePairs) {
    BVH* leaf = 
      new BVH(this, &trianglePair, trianglePair.GetBoundingBox(points));
    leaves.push_back(leaf);
  }
}

void BVH::Init(const std::vector<Geometry*>& geometries)
{
  size_t numColliders = geometries.size();
  std::cout << "INIT BVH ..." << std::endl;
  pxr::GfRange3d accum = geometries[0]->GetBoundingBox().GetRange();
  for (size_t i = 1; i < numColliders; ++i) {
    accum.UnionWith(geometries[i]->GetBoundingBox().GetRange());
  }
  SetMin(accum.GetMin());
  SetMax(accum.GetMax());
  std::cout << "BBOX : " << pxr::GfBBox3d(accum) << std::endl;

  std::vector<BVH*> trees;
  trees.reserve(numColliders);

  for (Geometry* geom : geometries) {
    std::cout << "CREATE BVH LEAVES..." << std::endl;
    trees.push_back(new BVH(this, geom));
  }
  std::cout << "BVH LEAVES CREATED..." << std::endl;
  std::vector<BVH*> cells = trees;
  std::vector<BVH*> results;
  _SortCellsByPair(trees, results);
  while (results.size() > 2) {
    cells = results;
    results.clear();
    _SortCellsByPair(cells, results);
  }
  if (results.size() == 1) {
    SetLeft(results[0]);
    SetMin(results[0]->GetMin());
    SetMax(results[0]->GetMax());
  }
  else {
    SetLeft(results[0]);
    SetRight(results[1]);
    pxr::GfRange3d range = pxr::GfRange3d::GetUnion(*results[0], *results[1]);
    SetMin(range.GetMin());
    SetMax(range.GetMax());
  }
  trees.clear();
}

void BVH::Init(Geometry* geometry, BVH* parent)
{
  if (geometry->GetType() == Geometry::MESH) {
    std::vector<BVH*> leaves;
    _SortTrianglesByPair(leaves, geometry);

    std::vector<BVH*> cells = leaves;
    std::vector<BVH*> results;
    _SortCellsByPair(leaves, results);
    while (results.size() > 2) {
      cells = results;
      results.clear();
      _SortCellsByPair(cells, results);
    }
    if (results.size() == 1) {
      SetLeft(results[0]);
      SetMin(results[0]->GetMin());
      SetMax(results[0]->GetMax());
    }
    else {
      SetLeft(results[0]);
      SetRight(results[1]);
      pxr::GfRange3d range = pxr::GfRange3d::GetUnion(*results[0], *results[1]);
      SetMin(range.GetMin());
      SetMax(range.GetMax());
    }
    leaves.clear();
  }
}

void BVH::Update(const std::vector<Geometry*>& geometries)
{

}

const pxr::GfRange3d&
BVH::GetBoundingBox() const
{
  return *this;
}

pxr::GfRange3d&
BVH::GetBoundingBox()
{
  return *this;
}

static void _RecurseGetNumCells(BVH* cell, size_t& count, short elemType)
{
  if (cell->IsLeaf()) {
    if (elemType == BVH::TRIPAIR) {
      count++;
    } else if (elemType == BVH::GEOMETRY) {
      BVH* leaf = (BVH*)cell;
      //BVH* bvh = (BVH*)leaf->Get();
      //count += bvh->GetNumCells();
      
    }
  } else {
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

void
BVH::EchoNumHits()
{
  std::cout << "NUM HITS : " << NUM_HITS << std::endl;
}


JVR_NAMESPACE_CLOSE_SCOPE