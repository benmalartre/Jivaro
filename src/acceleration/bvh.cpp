#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>

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


BVH::Cell::Cell()
  : _left(Intersector::INVALID_INDEX)
  , _right(Intersector::INVALID_INDEX)
  , _data(NULL)
  , _type(BVH::Cell::ROOT)
{
}

BVH::Cell::Cell(size_t lhs, BVH::Cell* left, size_t rhs, BVH::Cell* right)
  : _left(lhs)
  , _right(rhs)
  , _data(NULL)
  , _type(BVH::Cell::BRANCH)
{
  if (left && right) {
    const pxr::GfRange3d range = pxr::GfRange3d::GetUnion(*left,*right);
    SetMin(range.GetMin());
    SetMax(range.GetMax());
  } else if (left) {
    SetMin(left->GetMin());
    SetMax(left->GetMax());
  } else if (right) {
    SetMin(right->GetMin());
    SetMax(right->GetMax());
  }
}

BVH::Cell::Cell(Component* component, const pxr::GfRange3d& range)
  : _left(Intersector::INVALID_INDEX)
  , _right(Intersector::INVALID_INDEX)
  , _data((void*)component)
  , _type(BVH::Cell::LEAF)
{
  SetMin(range.GetMin());
  SetMax(range.GetMax());
}

bool
BVH::_Raycast(const BVH::Cell* cell, const pxr::GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{
  double enterDistance, exitDistance;
  if (!ray.Intersect(pxr::GfBBox3d(*cell), &enterDistance, &exitDistance))
    return false;

  if(enterDistance > maxDistance) 
    return false;

  const pxr::GfVec3f enterPoint(ray.GetPoint(enterDistance));

  if (cell->IsLeaf()) {
    
    size_t geomIdx = GetGeometryIndexFromCell(cell);
    const Geometry* geometry = GetGeometry(geomIdx);

    pxr::GfRay localRay(ray);

    localRay.Transform(geometry->GetInverseMatrix());
    const pxr::GfVec3f* points = ((const Deformable*)geometry)->GetPositionsCPtr();
    
    Component* component = (Component*)cell->GetData();
    Location localHit(*hit);
    if (component->Raycast(points, localRay, &localHit)) {
      const pxr::GfVec3d localPoint(localRay.GetPoint(localHit.GetT()));
      const double distance = (ray.GetStartPoint() - geometry->GetMatrix().Transform(localPoint)).GetLength();
      if ((distance < *minDistance) && (distance < maxDistance)) {
        hit->Set(localHit);
        hit->SetT(distance);
        hit->SetGeometryIndex(geomIdx);
        *minDistance = distance;
        return true;
      }
    }
    return false;
  } else {
    Location leftHit(*hit), rightHit(*hit);
    bool leftFound(false), rightFound(false);
    const BVH::Cell* left = _GetCell(cell->GetLeft());
    if (left && _Raycast(left, ray, &leftHit, maxDistance, minDistance))leftFound=true;
    const BVH::Cell* right = _GetCell(cell->GetRight());
    if (right && _Raycast(right, ray, &rightHit, maxDistance, minDistance))rightFound=true;

    if (leftFound && rightFound) {
      if(leftHit.GetT()<rightHit.GetT()) { 
        hit->Set(leftHit); return true;
      } else {
        hit->Set(rightHit); return true;
      }
    } else if (leftFound) {
      { hit->Set(leftHit); return true;}
    } else if (rightFound) {
      { hit->Set(rightHit); return true;}
    } return false;
  }

}

bool 
BVH::_Closest(const BVH::Cell* cell, const pxr::GfVec3f& point, Location* hit, 
  double maxDistance) const
{  
  if(!cell->Contains(point)) {
    const double distance = pxr::GfSqrt(cell->GetDistanceSquared(point));
    if(distance > hit->GetT() || distance > maxDistance)return false;
  }

  if (cell->IsLeaf()) {

    size_t geomIdx = GetGeometryIndexFromCell(cell);
    const Geometry* geometry = GetGeometry(geomIdx);
    
    const pxr::GfVec3f* points = ((const Deformable*)geometry)->GetPositionsCPtr();
    Component* component = (Component*)cell->GetData();
    pxr::GfVec3f localPoint(geometry->GetInverseMatrix().Transform(point));
    
    Location localHit;

    if (component->Closest(points, localPoint, &localHit)) {
      const Triangle* triangle = ((const Mesh*)geometry)->GetTriangle(localHit.GetComponentIndex());
      localHit.SetT((localHit.ComputePosition(points, &triangle->vertices[0], 3,
        &geometry->GetMatrix()) - point).GetLength());
    }

    if (localHit.GetT() < hit->GetT()) {
      hit->Set(localHit);
      hit->SetGeometryIndex(geomIdx);
      return true;
    }
    
    return false;
  }
  else {
    const BVH::Cell* left = _GetCell(cell->GetLeft());
    const BVH::Cell* right = GetCell(cell->GetRight());

    double maxDistSq = pxr::GfPow(hit->GetT(), 2);
    double leftDistSq = left->GetDistanceSquared(point);
    double rightDistSq = right->GetDistanceSquared(point);

    if( leftDistSq < maxDistSq && rightDistSq < maxDistSq ) {
      bool leftFound(false), rightFound(false);
      if(leftDistSq < rightDistSq) {
        leftFound = _Closest(left, point, hit, hit->GetT());
        rightFound = _Closest(right, point, hit, hit->GetT());
      } else {
        rightFound = _Closest(right, point, hit, hit->GetT());
        leftFound = _Closest(left, point, hit, hit->GetT());
      }
      return leftFound || rightFound;
    }
    else if( leftDistSq < maxDistSq )
      return _Closest(left, point, hit, hit->GetT());
    else if( rightDistSq < maxDistSq )
      return _Closest(right, point, hit, hit->GetT());
    else 
      return false;
  }
}

pxr::GfRange3f 
BVH::_RecurseUpdateCells(BVH::Cell* cell)
{

  if (cell->IsLeaf()) {
    const Geometry* geometry = GetGeometryFromCell(cell);
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
      range.UnionWith(_RecurseUpdateCells(_GetCell(cell->GetLeft())));
      range.UnionWith(_RecurseUpdateCells(_GetCell(cell->GetRight())));
    } 
    else if(cell->GetLeft()) {
      range.UnionWith(_RecurseUpdateCells(_GetCell(cell->GetLeft())));
    } 
    else if(cell->GetRight()) {
      range.UnionWith(_RecurseUpdateCells(_GetCell(cell->GetRight())));
    } 

    cell->SetMin(range.GetMin());
    cell->SetMax(range.GetMax());
    return range;
  }
}

uint64_t 
BVH::_ComputeCode(const pxr::GfVec3d& point) const
{
  const pxr::GfVec3i p = WorldToMorton(*this, point);
  return MortonEncode3D(p);
}

pxr::GfVec3d 
BVH::_ComputeCodeAsColor(const pxr::GfVec3d& point) const
{
  return MortonColor(*this, point);
}

const Morton&
BVH::_CellToMorton(size_t cellIdx) const
{
  short cellType = _GetCell(cellIdx)->GetType();
  if(cellType == BVH::Cell::BRANCH || cellType == BVH::Cell::ROOT)
    return { 0,0,0, NULL };
  else
    return _mortons[_cellToMorton[cellIdx]];
}

size_t
BVH::AddCell( BVH::Cell* lhs, BVH::Cell* rhs)
{
  _cells.push_back(BVH::Cell(_GetIndex(lhs), lhs, _GetIndex(rhs), rhs));
  return _cells.size() - 1;
}

size_t
BVH::AddCell(Component* component, const pxr::GfRange3d& range)
{
  _cells.push_back(BVH::Cell(component, range));
  return _cells.size() - 1;
}


const Geometry* 
BVH::GetGeometryFromCell(const BVH::Cell* cell) const
{
  size_t geomIdx = GetGeometryIndexFromCell(cell);
  
  return geomIdx != INVALID_GEOMETRY ?
    GetGeometry(geomIdx) : NULL;

}

size_t
BVH::GetGeometryIndexFromCell(const BVH::Cell* cell) const
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

void
BVH::Init(const std::vector<Geometry*>& geometries)
{
  Intersector::_Init(geometries);
  
  const pxr::GfBBox3d bbox;
  pxr::GfRange3d accum = bbox.GetRange();
  _numComponents = 0;
  for (size_t g = 0; g < GetNumGeometries(); ++g) {
    const pxr::GfBBox3d bbox = GetGeometry(g)->GetBoundingBox(true);
    accum.UnionWith(bbox.GetRange());
    _numComponents += ((Mesh*)GetGeometry(g))->GetTrianglePairs().size();
  }

  SetMin(accum.GetMin());
  SetMax(accum.GetMax());

  _mortons.clear();
  _mortons.reserve(_numComponents);
  
  // first load all geometries
  for (size_t g = 0; g < GetNumGeometries(); ++g) {
    size_t start = _cells.size();
    Geometry* geometry = GetGeometry(g);
    switch(geometry->GetType()) {
      case Geometry::MESH:
      {
        Mesh* mesh = (Mesh*)geometry;

        pxr::VtArray<TrianglePair>& trianglePairs = mesh->GetTrianglePairs();
        size_t numTrianglePairs = trianglePairs.size();

        const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
        const pxr::GfMatrix4d& matrix = mesh->GetMatrix();
        for (size_t t = 0; t < numTrianglePairs; ++t) {
          size_t leafIdx = AddCell(&trianglePairs[t],
            trianglePairs[t].GetBoundingBox(positions, matrix));
    
          uint64_t code = _ComputeCode(_GetCell(leafIdx)->GetMidpoint());
          uint64_t minimum = _ComputeCode(_GetCell(leafIdx)->GetMin());
          uint64_t maximum = _ComputeCode(_GetCell(leafIdx)->GetMax());
          _mortons.push_back({ code, minimum, maximum, leafIdx});
        }
        break;
      }
    }
    SetGeometryCellIndices(g, start, _cells.size());
  }

  size_t numLeaves = _cells.size();
  Morton morton = SortCells();
  size_t numBranches = _cells.size() - numLeaves;

  _root = _GetCell(morton.data);
  _root->SetType(BVH::Cell::ROOT);
  
  _cellToMorton.resize(_cells.size(), Intersector::INVALID_INDEX);
  for(size_t m = 0; m < _mortons.size(); ++m)
    _cellToMorton[_mortons[m].data] = m;

}

void
BVH::Update()
{
  pxr::GfRange3f newRange = _RecurseUpdateCells(_root);
  SetMin(newRange.GetMin());
  SetMax(newRange.GetMax());
}

bool BVH::Raycast(const pxr::GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{
  return _Raycast(_root, ray, hit, maxDistance, minDistance);
};

size_t 
BVH::_GetIndex(const BVH::Cell* cell) const
{
  return  ((intptr_t)cell - (intptr_t)&_cells[0]) / sizeof(BVH::Cell);
}

BVH::Cell*
BVH::_GetCell(size_t index)
{
  return index < _cells.size() ? &_cells[index] : NULL;
}

const BVH::Cell*
BVH::_GetCell(size_t index) const
{
  return index < _cells.size() ? &_cells[index] : NULL;
}

size_t
BVH::_RecurseSortCells(
  size_t           first,
  size_t           last)
{
  if (first == last) {
    return _mortons[first].data;
  }

  size_t split = MortonFindSplit(&_mortons[0], first, last);

  size_t leftIdx = _RecurseSortCells(first, split);
  size_t rightIdx = _RecurseSortCells(split + 1, last);
  return AddCell(_GetCell(leftIdx), _GetCell(rightIdx));
}

Morton 
BVH::SortCells()
{
  size_t numLeaves = _mortons.size();
  
  std::sort(_mortons.begin(), _mortons.end());

  size_t topIdx = _RecurseSortCells(0, static_cast<int>(numLeaves) - 1);
  BVH::Cell* top = _GetCell(topIdx);
  return {
    _ComputeCode( top->GetMidpoint() ),
    _ComputeCode( top->GetMin() ),
    _ComputeCode( top->GetMax() ),
    topIdx
   };
} 

pxr::GfVec3f
BVH::GetMortonColor(const pxr::GfVec3f& point)
{
  return pxr::GfVec3f(_ComputeCodeAsColor(point));
}


pxr::GfVec3f 
BVH::_ComputeHitPoint(Location* hit) const
{
  const Geometry* geometry = GetGeometry(hit->GetGeometryIndex());
  switch(geometry->GetType()) {
    case Geometry::MESH:
    {
      const Mesh* mesh = (Mesh*)geometry;
      const Triangle* triangle = mesh->GetTriangle(hit->GetComponentIndex());
      const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();

      return hit->ComputePosition(positions, &triangle->vertices[0], 3, &geometry->GetMatrix());
    }
  }
  return pxr::GfVec3f(0.f);
}


bool BVH::Closest(const pxr::GfVec3f& point, 
  Location* hit, double maxDistance) const
{
  return _Closest(_root, point, hit, maxDistance);
}

void BVH::GetCells(pxr::VtArray<pxr::GfVec3f>& positions,
  pxr::VtArray<pxr::GfVec3f>& sizes, pxr::VtArray<pxr::GfVec3f>& colors, bool branchOrLeaf)
{
  std::vector<const BVH::Cell*> cells;
  if(branchOrLeaf)GetLeaves(_root, cells);
  else GetBranches(_root, cells);
  size_t numCells = cells.size();
  positions.resize(numCells);
  sizes.resize(numCells);
  colors.resize(numCells);
  for (size_t l = 0; l < cells.size(); ++l) {
    positions[l] = pxr::GfVec3f(cells[l]->GetMidpoint());
    sizes[l] = pxr::GfVec3f(cells[l]->GetSize());
    colors[l] = pxr::GfVec3f(_ComputeCodeAsColor( 
      pxr::GfVec3f(cells[l]->GetMidpoint())));
  }
}

void
BVH::GetBranches(const BVH::Cell* cell, std::vector<const BVH::Cell*>& branches) const
{
  const BVH::Cell* left = _GetCell(cell->GetLeft());
  const BVH::Cell* right = _GetCell(cell->GetRight());
  if (left)GetBranches(left, branches);
  if (right)GetBranches(right, branches);
  if (cell->GetType() == BVH::Cell::BRANCH)branches.push_back(cell);
}

void
BVH::GetLeaves(const BVH::Cell* cell, std::vector<const BVH::Cell*>& leaves) const
{
  const BVH::Cell* left = _GetCell(cell->GetLeft());
  const BVH::Cell* right = _GetCell(cell->GetRight());
  if (left)GetLeaves(left, leaves);
  if (right)GetLeaves(right, leaves);
  if (cell->GetType() == BVH::Cell::LEAF)leaves.push_back(cell);
}



JVR_NAMESPACE_CLOSE_SCOPE