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
#include "../geometry/component.h"
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
    const pxr::GfRange3d range = geometry->GetBoundingBox(true).GetRange();
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
BVH::Cell::IsBranch() const
{
  return _type == BVH::Cell::BRANCH;
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

static void 
_RecurseGetBranches(BVH::Cell* cell, std::vector<BVH::Cell*>& branches)
{
  if(cell->GetType() == BVH::Cell::BRANCH)branches.push_back(cell);

  if (cell->GetLeft()) {
    _RecurseGetBranches(cell->GetLeft(), branches);
  }
  if (cell->GetRight()) {
    _RecurseGetBranches(cell->GetRight(), branches);
  }
}

void
BVH::Cell::GetBranches(std::vector<BVH::Cell*>& branches) const
{
  if (_left)_RecurseGetBranches(_left, branches);
  if(_right)_RecurseGetBranches(_right, branches);
}

static void
_RecurseGetLeaves(BVH::Cell* cell, std::vector<BVH::Cell*>& leaves)
{
  if(cell->GetType() == BVH::Cell::LEAF)leaves.push_back(cell);
  if (cell->GetLeft()) {
    _RecurseGetLeaves(cell->GetLeft(), leaves);
  }
  if (cell->GetRight()) {
    _RecurseGetLeaves(cell->GetRight(), leaves);
  }
}

void
BVH::Cell::GetLeaves(std::vector<BVH::Cell*>& leaves) const
{
  leaves.clear();
  if (_left)_RecurseGetLeaves(_left, leaves);
  if (_right)_RecurseGetLeaves(_right, leaves);
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

const Geometry*
BVH::Cell::GetGeometry() const
{
  const BVH::Cell* geom = GetGeom();
  if (geom) {
    return (Geometry*)geom->_data;
  }
  else return NULL;
}

bool
BVH::Cell::Raycast(const pxr::GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{
  double enterDistance, exitDistance;
  if (!ray.Intersect(pxr::GfBBox3d(*this), &enterDistance, &exitDistance))
    return false;

  if(enterDistance > maxDistance) 
    return false;

  if (IsLeaf()) {
    const Geometry* geometry = GetGeometry();
    pxr::GfRay localRay(ray);

    localRay.Transform(geometry->GetInverseMatrix());
    const pxr::GfVec3f* points = ((const Deformable*)geometry)->GetPositionsCPtr();
    
    Component* component = (Component*)_data;
    Location localHit(*hit);
    if (component->Raycast(points, localRay, &localHit)) {
      const pxr::GfVec3d localPoint(localRay.GetPoint(localHit.GetT()));
      const double distance = (ray.GetStartPoint() - geometry->GetMatrix().Transform(localPoint)).GetLength();
      if ((distance < *minDistance) && (distance < maxDistance)) {
        hit->Set(localHit);
        hit->SetT(distance);
        *minDistance = distance;
        return true;
      }
    }
    return false;
  } else {
    if(IsGeom()) {       
      const BVH* intersector = GetIntersector();
      hit->SetGeometryIndex(intersector->GetGeometryIndex((Geometry*)_data));
    }
    Location leftHit(*hit), rightHit(*hit);
    bool leftFound(false), rightFound(false);
    if (_left && _left->Raycast(ray, &leftHit, maxDistance, minDistance))leftFound=true;
    if (_right && _right->Raycast(ray, &rightHit, maxDistance, minDistance))rightFound=true;

    if (leftFound && rightFound) {
      if(leftHit.GetT()<rightHit.GetT()) 
        { hit->Set(leftHit);} else {hit->Set(rightHit);return true;}
    } else if (leftFound) {
      { hit->Set(leftHit); return true;}
    } else if (rightFound) {
      { hit->Set(rightHit); return true;}
    } return false;
  }
  
}

bool 
BVH::Cell::Closest(const pxr::GfVec3f& point, Location* hit, 
  double maxDistance) const
{  
  double distance = GetDistance(*this, point) - GetSize().GetLength() * 0.5f;
  if (distance > hit->GetT() || distance > maxDistance)return false;

  if(IsLeaf()) {
    const Geometry* geometry = GetGeometry();
    const pxr::GfVec3f* points = ((const Deformable*)geometry)->GetPositionsCPtr();
    Component* component = (Component*)_data;
    pxr::GfVec3f localPoint = geometry->GetInverseMatrix().Transform(point);
    Location localHit(*hit);
    localHit.TransformT(geometry->GetInverseMatrix());

    component->Closest(points, localPoint, &localHit);
      
    Triangle* triangle = ((Mesh*)geometry)->GetTriangle(localHit.GetComponentIndex());
    const pxr::GfVec3d localHitPoint = 
      localHit.ComputePosition(points, &triangle->vertices[0], 3);

    const double distance = (point - geometry->GetMatrix().Transform(localHitPoint)).GetLength();
    if(distance < hit->GetT()){
      hit->Set(localHit);
      hit->SetT(distance);
      return true;
    }
    return false;
  } else {
    
    if(IsGeom()) {       
      const BVH* intersector = GetIntersector();
      hit->SetGeometryIndex(intersector->GetGeometryIndex((Geometry*)_data));
    }

    if(_left && _right) 
    {
      double leftDistSq = (_left->GetMidpoint() - point).GetLengthSq() + 
        _left->GetSize().GetLengthSq() * 0.5f;
      double rightDistSq = (_right->GetMidpoint() - point).GetLengthSq() + 
        _right->GetSize().GetLengthSq() * 0.5f;
      bool leftFound, rightFound;
      if( leftDistSq < rightDistSq) {
        leftFound = _left->Closest(point, hit, maxDistance);
        rightFound = _right->Closest(point, hit, maxDistance);
      } else {
        rightFound = _right->Closest(point, hit, maxDistance);
        leftFound = _left->Closest(point, hit, maxDistance);
      }
      return leftFound || rightFound;
    } 

    else if(_left)
      return _left->Closest(point, hit, maxDistance);

    else if(_right)
      return _right->Closest(point, hit, maxDistance);

    else
      return false;


  }
}

void BVH::Cell::_MortonSortTrianglePairs(std::vector<Morton>& mortons, Geometry* geometry)
{
  if (geometry->GetType() != Geometry::MESH)return;
  Mesh* mesh = (Mesh*)geometry;


  pxr::VtArray<TrianglePair>& trianglePairs = mesh->GetTrianglePairs();
  size_t numTrianglePairs = trianglePairs.size();
  mortons.resize(numTrianglePairs);
  const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
  const pxr::GfMatrix4d& matrix = mesh->GetMatrix();
  for (size_t t = 0; t < numTrianglePairs; ++t) {
    BVH::Cell* leaf =
      new BVH::Cell(this, &trianglePairs[t], trianglePairs[t].GetBoundingBox(positions, matrix));
    const BVH::Cell* root = GetRoot();
    mortons[t] = { BVH::ComputeCode(root, leaf->GetMidpoint()), leaf };
  }
}


void BVH::Cell::_MortonSortTriangles(std::vector<Morton>& mortons, Geometry* geometry)
{
  if (geometry->GetType() != Geometry::MESH)return;
  Mesh* mesh = (Mesh*)geometry;

  pxr::VtArray<Triangle>& triangles = mesh->GetTriangles();
  size_t numTriangles = triangles.size();
  mortons.resize(numTriangles);
  const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
  const pxr::GfMatrix4d& matrix = mesh->GetMatrix();
  for (size_t t = 0; t < numTriangles; ++t) {
    BVH::Cell* leaf =
      new BVH::Cell(this, &triangles[t], triangles[t].GetBoundingBox(positions, matrix));
    const BVH::Cell* root = GetRoot();
    mortons[t] = { BVH::ComputeCode(root, leaf->GetMidpoint()), leaf };
  }
}

static pxr::GfRange3f 
_RecurseUpdateCells(BVH::Cell* cell, Geometry* geometry)
{
  if(cell->IsGeom()) {
    geometry = cell->GetGeometry();
  } 
  
  if (cell->IsLeaf()) {
    if (geometry->GetType() >= Geometry::POINT) {
      Component* component = (Component*)cell->GetData();
      const pxr::GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
      const pxr::GfMatrix4d& matrix = geometry->GetMatrix();
      const pxr::GfRange3f range = component->GetBoundingBox(positions, matrix);
      cell->SetMin(range.GetMin());
      cell->SetMax(range.GetMax());
      return range;
    } else {
      // todo handle implicit geometries
      return pxr::GfRange3f();
    }
   
  } else {
    pxr::GfRange3f range;
    if (cell->GetLeft() && cell->GetRight()) {
      range.UnionWith(_RecurseUpdateCells(cell->GetLeft(), geometry));
      range.UnionWith(_RecurseUpdateCells(cell->GetRight(), geometry));
    } 
    else if(cell->GetLeft()) {
      range.UnionWith(_RecurseUpdateCells(cell->GetLeft(), geometry));
    } 
    else if(cell->GetRight()) {
      range.UnionWith(_RecurseUpdateCells(cell->GetRight(), geometry));
    } 

    cell->SetMin(range.GetMin());
    cell->SetMax(range.GetMax());
    return range;
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

Morton BVH::Cell::SortCellsByPair(
  std::vector<Morton>& mortons)
{
  size_t numMortons = mortons.size();
  std::sort(mortons.begin(), mortons.end());
  return { 0, _RecurseSortCellsByPair(mortons, 0, static_cast<int>(numMortons) - 1) };
} 

void BVH::Cell::_FinishSort(std::vector<Morton>& mortons)
{
  Morton morton = SortCellsByPair(mortons);
  _SwapCells(this, (BVH::Cell*)morton.data);
}

const BVH::Cell* 
BVH::Cell::FindClosestBranch(const pxr::GfVec3f &point) const
{
  if(_type == BVH::Cell::LEAF)return _parent;
  bool checkLeft = false, checkRight = false;
  if(_left && _left->Contains(point))checkLeft=true;
  if(_right && _right->Contains(point))checkRight=true;

  if(checkLeft && checkRight)return this;
  if(checkLeft)return _left->FindClosestBranch(point);
  if(checkRight)return _right->FindClosestBranch(point);
  return this;
}

void BVH::Cell::Init(Geometry* geometry)
{
  _type = BVH::Cell::GEOM;
  if (geometry->GetType() == Geometry::MESH) {
    std::vector<Morton> leaves;
    _MortonSortTrianglePairs(leaves, geometry);
    _FinishSort(leaves);
  } else if (geometry->GetType() == Geometry::CURVE) {

  }
}

void
BVH::Init(const std::vector<Geometry*>& geometries)
{
  _geometries = geometries;
  _leaves.clear();
  size_t numColliders = _geometries.size();
  const pxr::GfBBox3d bbox = _geometries[0]->GetBoundingBox(true);
  pxr::GfRange3d accum = bbox.GetRange();
  for (size_t i = 1; i < numColliders; ++i) {
    const pxr::GfBBox3d bbox = _geometries[i]->GetBoundingBox(true);
    accum.UnionWith(bbox.GetRange());
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
  
  SortLeaves();
  cells.clear();
  
}

void
BVH::Update()
{
  pxr::GfRange3f newRange = _RecurseUpdateCells(&_root,NULL);
  SetMin(newRange.GetMin());
  SetMax(newRange.GetMax());
}

bool BVH::Raycast(const pxr::GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{
  return _root.Raycast(ray, hit, maxDistance, minDistance);
};

void 
BVH::SortLeaves()
{
  _root.GetLeaves(_leaves);

  size_t num = _leaves.size();
  std::vector<Morton> mortons(num);
  std::vector<BVH::Cell*> leaves = _leaves;

  for(size_t m = 0; m < num; ++m) {
    mortons[m] = {BVH::ComputeCode(&_root, leaves[m]->GetMidpoint()), leaves[m] };
  }

  std::vector<int> indices(num);
  std::iota(indices.begin(), indices.end(), 0);
  std::sort(indices.begin(), indices.end(),
            [&](int A, int B) -> bool {
                return mortons[A] < mortons[B];
            });
  
  _mortons.resize(num);
  for(size_t m = 0; m < num; ++m) {
    _leaves[m] = leaves[indices[m]];
    _mortons[m] = mortons[indices[m]];
  }
}


/*


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

Morton BVH::Cell::SortCellsByPair(
  std::vector<Morton>& mortons)
{
  size_t numMortons = mortons.size();
  std::sort(mortons.begin(), mortons.end());
  return { 0, _RecurseSortCellsByPair(mortons, 0, static_cast<int>(numMortons) - 1) };
} 

void BVH::Cell::_FinishSort(std::vector<Morton>& mortons)
{
  Morton morton = SortCellsByPair(mortons);
  _SwapCells(this, (BVH::Cell*)morton.data);
}
*/

bool BVH::Closest(const pxr::GfVec3f& point, 
  Location* hit, double maxDistance) const
{
  const BVH::Cell* branch = _root.FindClosestBranch(point);    
  return branch->Closest(point, hit, maxDistance);
}

void BVH::GetCells(pxr::VtArray<pxr::GfVec3f>& positions,
  pxr::VtArray<pxr::GfVec3f>& sizes, pxr::VtArray<pxr::GfVec3f>& colors, bool branchOrLeaf)
{
  std::vector<BVH::Cell*> cells;
  if(branchOrLeaf)_root.GetLeaves(cells);
  else _root.GetBranches(cells);
  size_t numCells = cells.size();
  std::cout << "DRAW " << numCells << " BVH CELLS !!" << std::endl;
  positions.resize(numCells);
  sizes.resize(numCells);
  colors.resize(numCells);
  for (size_t l = 0; l < cells.size(); ++l) {
    positions[l] = pxr::GfVec3f(cells[l]->GetMidpoint());
    sizes[l] = pxr::GfVec3f(cells[l]->GetSize());
    colors[l] = pxr::GfVec3f(ComputeCodeAsColor(GetRoot(), 
      pxr::GfVec3f(cells[l]->GetMidpoint())));
  }
}



JVR_NAMESPACE_CLOSE_SCOPE