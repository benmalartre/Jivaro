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

/*
static void _RearangeChildrenCells(BVH::Cell* lhs, BVH::Cell* rhs)
{
  if(lhs->IsLeaf() || rhs->IsLeaf())return;

  if(lhs->GetLeft() && lhs->GetRight() && rhs->GetLeft() && rhs->GetRight()) {
    pxr::GfRange3d alt1 = pxr::GfRange3d().UnionWith(*lhs->GetLeft()).UnionWith(*rhs->GetLeft());
    pxr::GfRange3d alt2 = pxr::GfRange3d().UnionWith(*lhs->GetLeft()).UnionWith(*rhs->GetRight());

    float alt1LengthSq = alt1.GetSize().GetLengthSq();
    float alt2LengthSq = alt2.GetSize().GetLengthSq();
    if( alt1 < alt2 && alt1 < lhs->GetSize().GetLengthSq()) {
      std::swap(lhs->GetLeft(), rhs->GetLeft());
      lhs->Set(alt1);
       
    } else if(alt2 < alt1 && alt2 < lhs->GetSize().GetLengthSq()) {

    }
  }
}
*/


BVH::Cell::Cell()
  : _left(BVH::INVALID_INDEX)
  , _right(BVH::INVALID_INDEX)
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
  }
}

BVH::Cell::Cell(Component* component, const pxr::GfRange3d& range)
  : _left(BVH::INVALID_INDEX)
  , _right(BVH::INVALID_INDEX)
  , _data((void*)component)
  , _type(BVH::Cell::LEAF)
{
  SetMin(range.GetMin());
  SetMax(range.GetMax());
}

pxr::GfVec3f _ConstraintPointInRange(const pxr::GfVec3f& point, const pxr::GfRange3d &range)
{
  return pxr::GfVec3f(
    CLAMP(point[0], range.GetMin()[0], range.GetMax()[0]),
    CLAMP(point[1], range.GetMin()[1], range.GetMax()[1]),
    CLAMP(point[2], range.GetMin()[2], range.GetMax()[2])
  );
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

static double 
GetDistanceSq(const pxr::GfRange3d& range, const pxr::GfVec3d& point)
{
  const pxr::GfVec3d& minimum = range.GetMin();
  const pxr::GfVec3d& maximum = range.GetMax();
  float dx = _GetDistance1D(point[0], minimum[0], maximum[0]);
  float dy = _GetDistance1D(point[1], minimum[1], maximum[1]);
  float dz = _GetDistance1D(point[2], minimum[2], maximum[2]);
  return dx * dx + dy * dy + dz * dz;
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
  if (cell->IsLeaf()) {

    size_t geomIdx = GetGeometryIndexFromCell(cell);
    const Geometry* geometry = GetGeometry(geomIdx);
    
    const pxr::GfVec3f* points = ((const Deformable*)geometry)->GetPositionsCPtr();
    Component* component = (Component*)cell->GetData();
    pxr::GfVec3f localPoint = geometry->GetInverseMatrix().Transform(point);
    Location localHit(*hit);
    localHit.TransformT(geometry->GetInverseMatrix());

    component->Closest(points, localPoint, &localHit);

    localHit.TransformT(geometry->GetMatrix());

    if (localHit.GetT() < hit->GetT()) {
      hit->Set(localHit);
      hit->SetGeometryIndex(geomIdx);
      return true;
    }
    
    return false;
  }
  else {
    
    const double distance = GetDistance(*cell, point);
    if (distance > hit->GetT() || distance > maxDistance)return false;

    const BVH::Cell* left = _GetCell(cell->GetLeft());
    const BVH::Cell* right = GetCell(cell->GetRight());

    if(left && right) 
    {
      uint64_t morton = _ComputeCode(point);
      uint64_t leftMorton = _ComputeCode(_ConstraintPointInRange(point, *left));
      uint64_t rightMorton = _ComputeCode(_ConstraintPointInRange(point, *right));

      int leftPrefix = MortonLeadingZeros(morton ^ leftMorton);
      int rightPrefix = MortonLeadingZeros(morton ^ rightMorton);
      if (leftPrefix > rightPrefix)
        return _Closest(left, point, hit, maxDistance);

      else if (rightPrefix > leftPrefix)
        return _Closest(right, point, hit, maxDistance);

      else 
      {
        Location leftHit(*hit), rightHit(*hit);
        bool leftFound = _Closest(left, point, &leftHit, maxDistance);
        bool rightFound = _Closest(right, point, &rightHit, maxDistance);

        if(leftFound && rightFound) {
          if(leftHit.GetT() < rightHit.GetT())
            hit->Set(leftHit);
          else
            hit->Set(rightHit);
          return true;
        } 

        else if(leftFound) 
          {hit->Set(leftHit);return true;}

        else if(rightFound) 
          {hit->Set(rightHit);return true;}

        else return false;
      }
    }
    else if(left)
      return _Closest(left, point, hit, maxDistance);

    else if(right)
      return _Closest(right, point, hit, maxDistance);

    else
      return false;
    

    /*
    if(left && right) {
      Location leftHit(*hit), rightHit(*hit);
      bool leftFound = _Closest(left, point, &leftHit, maxDistance);
      bool rightFound = _Closest(right, point, &rightHit, maxDistance);

      if(leftFound && rightFound) {
        if(leftHit.GetT() < rightHit.GetT())
          hit->Set(leftHit);
        else
          hit->Set(rightHit);
        return true;
      } 

    }
    else if(left)
      return _Closest(left, point, hit, maxDistance);

    else if(right)
      return _Closest(right, point, hit, maxDistance);

    else
      return false;
    */
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
  uint64_t morton = _ComputeCode( point);
  pxr::GfVec3i p = MortonDecode3D(morton);
  return pxr::GfVec3d(
    p[0] / (float)MORTOM_MAX_L, 
    p[1] / (float)MORTOM_MAX_L, 
    p[2] / (float)MORTOM_MAX_L
  );
}

size_t 
BVH::_FindSplit(size_t first, size_t last) const
{
  uint64_t firstCode = _mortons[first].code;
  uint64_t lastCode = _mortons[last].code;

  if (firstCode == lastCode)
    return (first + last) >> 1;

  size_t commonPrefix = MortonLeadingZeros(firstCode ^ lastCode);
  size_t split = first;
  size_t step = last - first;

  do
  {
    step = (step + 1) >> 1;
    size_t newSplit = split + step;

    if (newSplit < last)
    {
      uint64_t splitCode = _mortons[newSplit].code;
      size_t splitPrefix = MortonLeadingZeros(firstCode ^ splitCode);
      if (splitPrefix > commonPrefix) {
        split = newSplit;
      }
    }
  } while (step > 1);

  return split;
}



size_t 
BVH::_FindClosestCell(uint64_t code) const
{
  size_t first = 0;
  size_t last = _mortons.size();
  size_t closest = BVH::INVALID_INDEX;
  size_t maxCommonPrefix = 0;

  while(first != last) {
    size_t middle = (first + last) >> 1;

    if(_mortons[middle].code > code) last = middle;
    else if(_mortons[middle].code < code) first = middle + 1;
    else {
      size_t firstPrefix = MortonLeadingZeros(code ^ _mortons[first].code);
      size_t lastPrefix = MortonLeadingZeros(code ^ _mortons[last].code);

      if(firstPrefix > lastPrefix)return first;
      else if(firstPrefix < lastPrefix)return last;
      else return middle;
    }

    size_t commonPrefix = MortonLeadingZeros(code ^ _mortons[middle].code);

    if(commonPrefix > maxCommonPrefix) {
      closest = middle;
      maxCommonPrefix = commonPrefix;
    } 
  }

  return closest;
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
  
  return geomIdx != BVH::INVALID_GEOMETRY ?
    GetGeometry(geomIdx) : NULL;

}

size_t
BVH::GetGeometryIndexFromCell(const BVH::Cell* cell) const
{
  /*
   size_t cellIdx = _GetIndex(cell);

   for(size_t geomIdx = 0; geomIdx < GetNumGeometries(); ++geomIdx) {
      size_t startIdx = GetGeometryCellsStartIndex(geomIdx);
      size_t endIdx = GetGeometryCellsEndIndex(geomIdx);
      if(cellIdx >= startIdx && cellIdx < endIdx )
        return geomIdx;
   }
   return INVALID_GEOMETRY;
  */
  size_t cellIdx = _GetIndex(cell);
  size_t startIdx, endIdx;
  size_t start = 0;
  size_t end = GetNumGeometries();
  size_t split = end >> 1;

  while(start != end)
  {
    startIdx = GetGeometryCellsStartIndex(split);
    endIdx = GetGeometryCellsEndIndex(split);
    if (cellIdx < startIdx)
      end = split;

    else if(cellIdx >= endIdx)
      start = split;

    else if(startIdx <= cellIdx && cellIdx < endIdx )
      return split;
      
    split = (start + end) >> 1;
  } 
  return INVALID_GEOMETRY;
  
}


void
BVH::Init(const std::vector<Geometry*>& geometries)
{
  Intersector::_Init(geometries);
  
  const pxr::GfBBox3d bbox = GetGeometry(0)->GetBoundingBox(true);
  pxr::GfRange3d accum = bbox.GetRange();
  size_t numComponents = ((Mesh*)GetGeometry(0))->GetTrianglePairs().size();
  for (size_t i = 1; i < GetNumGeometries(); ++i) {
    const pxr::GfBBox3d bbox = GetGeometry(i)->GetBoundingBox(true);
    accum.UnionWith(bbox.GetRange());
    numComponents += ((Mesh*)GetGeometry(1))->GetTrianglePairs().size();
  }
  SetMin(accum.GetMin());
  SetMax(accum.GetMax());

  _mortons.clear();
  _mortons.reserve(numComponents);

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
        
      }
    }
    SetGeometryCellIndices(g, start, _cells.size());
  }

  Morton morton = SortCells();

  _root = _GetCell(morton.data);
  _root->SetType(BVH::Cell::ROOT);

  _mortons.clear();
  for(size_t c = 0; c < _cells.size(); ++c) { 
    if (_cells[c].GetType() == BVH::Cell::LEAF) {
      _mortons.push_back({
        _ComputeCode(_cells[c].GetMidpoint()),
        _ComputeCode(_cells[c].GetMin()),
        _ComputeCode(_cells[c].GetMax()),
        _GetIndex(&_cells[c])});
    }
  }
  std::sort(_mortons.begin(), _mortons.end());
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

void BVH::_AddTrianglePairs(Geometry* geometry)
{
  if (geometry->GetType() != Geometry::MESH)return;
  Mesh* mesh = (Mesh*)geometry;

  pxr::VtArray<TrianglePair>& trianglePairs = mesh->GetTrianglePairs();
  size_t numTrianglePairs = trianglePairs.size();
  _mortons.resize(numTrianglePairs);
  const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
  const pxr::GfMatrix4d& matrix = mesh->GetMatrix();
  for (size_t t = 0; t < numTrianglePairs; ++t) {
    size_t leafIdx = AddCell(&trianglePairs[t],
      trianglePairs[t].GetBoundingBox(positions, matrix));
    _mortons[t] = { 
      _ComputeCode( _GetCell(leafIdx)->GetMidpoint()), 
      _ComputeCode( _GetCell(leafIdx)->GetMin()),
      _ComputeCode( _GetCell(leafIdx)->GetMax()),
      leafIdx
    };
  }
}


void BVH::_AddTriangles( Geometry* geometry)
{
  if (geometry->GetType() != Geometry::MESH)return;
  Mesh* mesh = (Mesh*)geometry;

  pxr::VtArray<Triangle>& triangles = mesh->GetTriangles();
  size_t numTriangles = triangles.size();
  _mortons.resize(numTriangles);
  const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
  const pxr::GfMatrix4d& matrix = mesh->GetMatrix();
  for (size_t t = 0; t < numTriangles; ++t) {
    size_t leafIdx = AddCell(&triangles[t],
      triangles[t].GetBoundingBox(positions, matrix));
    _mortons[t] = { 
      _ComputeCode(_GetCell(leafIdx)->GetMidpoint()),
      _ComputeCode(_GetCell(leafIdx)->GetMin()),
      _ComputeCode(_GetCell(leafIdx)->GetMax()),
      leafIdx
    };
  }
}

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

  size_t split = _FindSplit(first, last);

  size_t leftIdx = _RecurseSortCells(first, split);
  size_t rightIdx = _RecurseSortCells(split + 1, last);
  return AddCell(_GetCell(leftIdx), _GetCell(rightIdx));
}

Morton 
BVH::SortCells()
{
  size_t numMortons = _mortons.size();
  
  std::sort(_mortons.begin(), _mortons.end());

  size_t topIdx = _RecurseSortCells(0, static_cast<int>(numMortons) - 1);
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


bool 
BVH::_ShouldCheckCell(const BVH::Cell* parent, const BVH::Cell* cell, 
  const Morton &morton, const pxr::GfVec3f& point, double maxDistance) const
{
  if(!cell)return false;

  const float maxDistanceSq = pxr::GfPow(maxDistance, 2);
  if(cell->GetDistanceSquared(point) < maxDistanceSq)
    return false;

  uint64_t cellMinCode = _ComputeCode(cell->GetMin());
  uint64_t cellMaxCode = _ComputeCode(cell->GetMax());

  return morton.minimum  < cellMaxCode && morton.maximum > cellMinCode;
}

bool 
BVH::_RecurseClosestCell(const BVH::Cell* cell, const Morton &morton, 
  const pxr::GfVec3f& point, Location* hit, double maxDistance) const
{
  if(!cell) return false;

  if(cell->GetDistanceSquared(point) > pxr::GfPow(maxDistance, 2)) 
    return false;
  
  if(cell->IsLeaf()) {
    return _Closest(cell, point, hit, maxDistance);
  } 
  else {
    const BVH::Cell* left = _GetCell(cell->GetLeft());
    const BVH::Cell* right = _GetCell(cell->GetRight());

    Location leftHit(*hit), rightHit(*hit);
    bool leftFound(false), rightFound(false);
    if(_ShouldCheckCell(cell, left, morton, point, maxDistance)) 
      leftFound = _RecurseClosestCell(left, morton, point, &leftHit, hit->GetT());
    
    if (_ShouldCheckCell(cell, right, morton, point, maxDistance))
      rightFound = _RecurseClosestCell(right, morton, point, &rightHit, hit->GetT());

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

bool BVH::Closest(const pxr::GfVec3f& point, 
  Location* hit, double maxDistance) const
{
  /*
  uint64_t morton = _ComputeCode(point);

  const BVH::Cell *cell=_root;
  while(false) {
    const BVH::Cell* left = _GetCell(cell->GetLeft());
    const BVH::Cell* right = _GetCell(cell->GetRight());
    
    if(!left &&  !right) break;

    uint64_t leftMorton = _ComputeCode(left->GetMidpoint());
    uint64_t rightMorton = _ComputeCode(right->GetMidpoint());

    int leftPrefix = MortonLeadingZeros(morton ^ leftMorton);
    int rightPrefix = MortonLeadingZeros(morton ^ rightMorton);
    if (leftPrefix > rightPrefix)
      cell = _GetCell(cell->GetLeft());
    else if (rightPrefix > leftPrefix)
      cell = _GetCell(cell->GetRight());
    else break;

  }
  */

  uint64_t pntCode = _ComputeCode(point);
  size_t closestIdx = _FindClosestCell(pntCode);
  const BVH::Cell* closest = _GetCell(_mortons[closestIdx].data);

  if(_Closest(closest, point, hit, maxDistance)) {
    Morton morton = {
      pntCode, 
      _ComputeCode(point - pxr::GfVec3f(hit->GetT())),
      _ComputeCode(point + pxr::GfVec3f(hit->GetT())), 
      INVALID_INDEX
    };

    _RecurseClosestCell(_root, morton, point, hit, hit->GetT());

    return true;
  }
  return false;

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