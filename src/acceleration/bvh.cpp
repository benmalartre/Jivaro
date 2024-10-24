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
#include "../geometry/edge.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"

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
    const GfRange3d range = GfRange3d::GetUnion(*left,*right);
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

BVH::Cell::Cell(Component* component, const GfRange3d& range)
  : _left(Intersector::INVALID_INDEX)
  , _right(Intersector::INVALID_INDEX)
  , _data((void*)component)
  , _type(BVH::Cell::LEAF)
{
  SetMin(range.GetMin());
  SetMax(range.GetMax());
}

bool
BVH::_Raycast(const BVH::Cell* cell, const GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{
  maxDistance = GfMin(maxDistance, hit->GetDistance());
  if(cell->GetDistanceSquared(ray.GetPoint(0.f)) > maxDistance * maxDistance) return false;

  if (cell->IsLeaf()) {
    
    size_t geomIdx = GetGeometryIndexFromCell(cell);
    const Geometry* geometry = GetGeometry(geomIdx);

    GfRay localRay(ray);

    localRay.Transform(geometry->GetInverseMatrix());
    const GfVec3f* points = ((const Deformable*)geometry)->GetPositionsCPtr();
    
    Component* component = (Component*)cell->GetData();
    Location localHit(*hit);
    if (component->Raycast(points, localRay, &localHit)) {

      const GfVec3d localPoint(localRay.GetPoint(localHit.GetDistance()));
      const double distance = (ray.GetStartPoint() - geometry->GetMatrix().Transform(localPoint)).GetLength();
      
      if ((distance < maxDistance)) {
        hit->Set(localHit);
        hit->SetDistance(distance);
        hit->SetGeometryIndex(geomIdx);
        if(minDistance)
          *minDistance = distance;
        return true;
      }
    }
    return false;
  } else {
    bool leftFound(false), rightFound(false);
    const BVH::Cell* left = _GetCell(cell->GetLeft());
    const BVH::Cell* right = _GetCell(cell->GetRight());
  
    double leftDist=maxDistance, rightDist=maxDistance;
    ray.Intersect(GfBBox3d(*left), &leftDist);
    bool leftCheck = leftDist < maxDistance;
    ray.Intersect(GfBBox3d(*right), &rightDist);
    bool rightCheck = rightDist < maxDistance;
  
    if(leftCheck && rightCheck) {
      if(leftDist < rightDist) {
        if (_Raycast(left, ray, hit, maxDistance, minDistance))leftFound=true;
        if (_Raycast(right, ray, hit, maxDistance, minDistance))rightFound=true;
      } else {
        if (_Raycast(right, ray, hit, maxDistance, minDistance))rightFound=true;
        if (_Raycast(left, ray, hit, maxDistance, minDistance))leftFound=true;
      }
    } else if (leftCheck) {
      if (_Raycast(left, ray, hit, maxDistance, minDistance))leftFound=true;
    } else if (rightCheck) {
      if (_Raycast(right, ray, hit, maxDistance, minDistance))rightFound=true;
    }

    return leftFound || rightFound;

  }

}

bool 
BVH::_Closest(const BVH::Cell* cell, const GfVec3f& point, Location* hit, 
  double maxDistanceSq) const
{  
  if(!cell->Contains(point)) {
    const double distanceSq = cell->GetDistanceSquared(point);
    if(distanceSq > (point - hit->GetPoint()).GetLengthSq() || distanceSq > maxDistanceSq)return false;
  }

  if (cell->IsLeaf()) {

    size_t geomIdx = GetGeometryIndexFromCell(cell);
    const Geometry* geometry = GetGeometry(geomIdx);
    const GfMatrix4d& invMatrix = geometry->GetInverseMatrix();
    
    const GfVec3f* points = ((const Deformable*)geometry)->GetPositionsCPtr();
    Component* component = (Component*)cell->GetData();
    GfVec3f localPoint(invMatrix.Transform(point));
    
    Location localHit(*hit);
    if(hit->IsValid())
      localHit.ConvertToLocal(invMatrix);

    if (component->Closest(points, localPoint, &localHit)) {
      localHit.ConvertToWorld(geometry->GetMatrix());
      hit->Set(localHit);
      hit->SetGeometryIndex(geomIdx);
      return true;
    }

    return false;
  }
  else {
    const BVH::Cell* left = _GetCell(cell->GetLeft());
    const BVH::Cell* right = GetCell(cell->GetRight());

    double hitDistSq = hit->IsValid() ? (point - hit->GetPoint()).GetLengthSq() : DBL_MAX;
    double leftDistSq = left->GetDistanceSquared(point);
    double rightDistSq = right->GetDistanceSquared(point);

    if( leftDistSq < hitDistSq && rightDistSq < hitDistSq) {
      bool leftFound(false), rightFound(false);
      if(leftDistSq < rightDistSq) {
        leftFound = _Closest(left, point, hit, hitDistSq);
        rightFound = _Closest(right, point, hit, hitDistSq);
      } else {
        rightFound = _Closest(right, point, hit, hitDistSq);
        leftFound = _Closest(left, point, hit, hitDistSq);
      }
      return leftFound || rightFound;
    }
    else if( leftDistSq < hitDistSq )
      return _Closest(left, point, hit, hitDistSq);
    else if( rightDistSq < hitDistSq )
      return _Closest(right, point, hit, hitDistSq);
    else 
      return false;
  }
}

GfRange3f 
BVH::_RecurseUpdateCells(BVH::Cell* cell)
{

  if (cell->IsLeaf()) {
    const Geometry* geometry = GetGeometryFromCell(cell);
    if (geometry->GetType() >= Geometry::POINT) {
      Component* component = (Component*)cell->GetData();
      const GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
      const GfMatrix4d& matrix = geometry->GetMatrix();
      const GfRange3f range = component->GetBoundingBox(positions, matrix);
      cell->SetMin(range.GetMin());
      cell->SetMax(range.GetMax());
      return range;
    } else {
      // todo handle implicit geometries
      return GfRange3f();
    }
   
  } else {
    GfRange3f range;
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
BVH::_ComputeCode(const GfVec3d& point) const
{
  const GfVec3i p = WorldToMorton(*this, point);
  return MortonEncode3D(p);
}

GfVec3d 
BVH::_ComputeCodeAsColor(const GfVec3d& point) const
{
  return MortonColor(*this, point);
}

const Morton&
BVH::_CellToMorton(size_t cellIdx) const
{
  short cellType = _GetCell(cellIdx)->GetType();
  if(cellType == BVH::Cell::BRANCH || cellType == BVH::Cell::ROOT)
    return { 0,NULL };
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
BVH::AddCell(Component* component, const GfRange3d& range)
{
  _cells.push_back(BVH::Cell(component, range));
  return _cells.size() - 1;
}


const Geometry* 
BVH::GetGeometryFromCell(const BVH::Cell* cell) const
{
  size_t geomIdx = GetGeometryIndexFromCell(cell);
  
  return geomIdx != INVALID_INDEX ?
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
  return INVALID_INDEX;
}

void
BVH::Init(const std::vector<Geometry*>& geometries)
{
  Intersector::_Init(geometries);
  
  const GfBBox3d bbox;
  GfRange3d accum = bbox.GetRange();
  _numComponents = 0;
  for (size_t g = 0; g < GetNumGeometries(); ++g) {
    const GfBBox3d bbox = GetGeometry(g)->GetBoundingBox(true);
    accum.UnionWith(bbox.GetRange());
    _numComponents += ((Mesh*)GetGeometry(g))->GetTrianglePairs().size();
  }

  SetMin(accum.GetMin());
  SetMax(accum.GetMax());

  if(!_accelerated)return;

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

        VtArray<TrianglePair>& trianglePairs = mesh->GetTrianglePairs();
        size_t numTrianglePairs = trianglePairs.size();

        const GfVec3f* positions = mesh->GetPositionsCPtr();
        const GfMatrix4d& matrix = mesh->GetMatrix();
        for (size_t t = 0; t < numTrianglePairs; ++t) {
          size_t leafIdx = AddCell(&trianglePairs[t],
            trianglePairs[t].GetBoundingBox(positions, matrix));
          uint64_t code = _ComputeCode(_GetCell(leafIdx)->GetMidpoint());
          _mortons.push_back({ code, leafIdx});
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
  if(_accelerated) {
    
    GfRange3f newRange = _RecurseUpdateCells(_root);
    SetMin(newRange.GetMin());
    SetMax(newRange.GetMax());
    
    /*
   _mortons.clear();

    // first update morton codes
    for (size_t g = 0; g < GetNumGeometries(); ++g) {
      size_t start = GetGeometryCellsStartIndex(g);
      size_t end = GetGeometryCellsStartIndex(g);

      Geometry* geometry = GetGeometry(g);
      switch(geometry->GetType()) {
        case Geometry::MESH:
        {
          Mesh* mesh = (Mesh*)geometry;

          VtArray<TrianglePair>& trianglePairs = mesh->GetTrianglePairs();
          size_t numTrianglePairs = trianglePairs.size();

          for(size_t i = 0; i < numTrianglePairs; ++i) {
            uint64_t code = _ComputeCode(_GetCell(start + i)->GetMidpoint());
            _mortons.push_back({ code, start + i});
          }
          break;
        }
      }
    }

    size_t numLeaves = _cells.size();
    Morton morton = SortCells();
    size_t numBranches = _cells.size() - numLeaves;

    _root = _GetCell(morton.data);
    _root->SetType(BVH::Cell::ROOT);
    
    _cellToMorton.resize(_cells.size(), Intersector::INVALID_INDEX);
    for(size_t m = 0; m < _mortons.size(); ++m)
      _cellToMorton[_mortons[m].data] = m;
    */
  }
}

bool BVH::Raycast(const GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{
  if(_accelerated) 
    return _Raycast(_root, ray, hit, maxDistance, minDistance);

  else {
    bool found = false;
    for(size_t g = 0; g < GetNumGeometries(); ++g) {
      const Geometry* geom = GetGeometry(g);
      if(geom->GetType() == Geometry::MESH) {
        const Mesh* mesh = (Mesh*)geom;
        if(mesh->Raycast(ray, hit, maxDistance, minDistance)) {
          hit->SetGeometryIndex(g);
          found = true;
        }
      }
    }
    return found;
  }
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
    topIdx
   };
} 

GfVec3f
BVH::GetMortonColor(const GfVec3f& point)
{
  return GfVec3f(_ComputeCodeAsColor(point));
}


GfVec3f 
BVH::_ComputeHitPoint(Location* hit) const
{
  const Geometry* geometry = GetGeometry(hit->GetGeometryIndex());
  switch(geometry->GetType()) {
    case Geometry::MESH:
    {
      const Mesh* mesh = (Mesh*)geometry;
      const Triangle* triangle = mesh->GetTriangle(hit->GetComponentIndex());
      const GfVec3f* positions = mesh->GetPositionsCPtr();

      return hit->ComputePosition(positions, &triangle->vertices[0], 3, &geometry->GetMatrix());
    }

    case Geometry::CURVE:
    {
      const Curve* curve = (Curve*)geometry;
      const Edge* edge = curve->GetEdge(hit->GetComponentIndex());
      const GfVec3f* positions = curve->GetPositionsCPtr();

      return hit->ComputePosition(positions, &edge->vertices[0], 2, &geometry->GetMatrix());
    }
  }
  return GfVec3f(0.f);
}


bool BVH::Closest(const GfVec3f& point, 
  Location* hit, double maxDistance) const
{
  if(_accelerated)
    return _Closest(_root, point, hit, 
      maxDistance < DBL_MAX ? maxDistance * maxDistance : DBL_MAX);
  return false;
}

void BVH::GetCells(VtArray<GfVec3f>& positions,
  VtArray<GfVec3f>& sizes, VtArray<GfVec3f>& colors, bool branchOrLeaf)
{
  std::vector<const BVH::Cell*> cells;
  if(branchOrLeaf)GetLeaves(_root, cells);
  else GetBranches(_root, cells);
  size_t numCells = cells.size();
  positions.resize(numCells);
  sizes.resize(numCells);
  colors.resize(numCells);
  for (size_t l = 0; l < cells.size(); ++l) {
    positions[l] = GfVec3f(cells[l]->GetMidpoint());
    sizes[l] = GfVec3f(cells[l]->GetSize());
    colors[l] = GfVec3f(_ComputeCodeAsColor( 
      GfVec3f(cells[l]->GetMidpoint())));
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