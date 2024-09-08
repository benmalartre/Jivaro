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
  } else if (right) {
    SetMin(right->GetMin());
    SetMax(right->GetMax());
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

inline double _ContraintValueOnSilhouette(double value, double minimum, double maximum)
{
  return (value >= minimum && value <= maximum) ? 
    minimum + (maximum - value) : 
    (value > (minimum + maximum) /2.0 ? minimum : maximum);
}

pxr::GfVec3f _ConstraintPointOnSilhouette(const pxr::GfVec3f& point, const pxr::GfRange3d &range)
{
  const pxr::GfVec3f middle(range.GetMidpoint());
  const pxr::GfVec3f minimum(range.GetMin());
  const pxr::GfVec3f maximum(range.GetMax());
  return pxr::GfVec3f(
    _ContraintValueOnSilhouette(point[0], minimum[0], maximum[0]),
    _ContraintValueOnSilhouette(point[1], minimum[1], maximum[1]),
    _ContraintValueOnSilhouette(point[2], minimum[2], maximum[2])
  );
}

const size_t BVH::INVALID_INDEX = std::numeric_limits<size_t>::max();
const double BVH::EPSILON = 1e-6;


pxr::GfVec3f 
BVH::Constraint(const BVH::Cell* cell, const pxr::GfVec3f &point) const
{
  return _ConstraintPointOnSilhouette(point, *cell);
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
  const double distance = pxr::GfSqrt(cell->GetDistanceSquared(point));
  if (distance > hit->GetT() || distance > maxDistance)return false;

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

    const BVH::Cell* left = _GetCell(cell->GetLeft());
    const BVH::Cell* right = GetCell(cell->GetRight());

    switch(_LeftOrRight(cell, point, 0, hit->GetT()))
    {
      case 0:
        return _Closest(left, point, hit, maxDistance);
      
      case 1:
        return _Closest(right, point, hit, maxDistance);

      case 2:
      {

        bool leftFound = _Closest(left, point, hit, maxDistance);
        bool rightFound = _Closest(right, point, hit, maxDistance);

        return leftFound || rightFound;
      }
      case 3:
        return false;
    }
    
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
    p[0] / (float)MORTON_MAX_L, 
    p[1] / (float)MORTON_MAX_L, 
    p[2] / (float)MORTON_MAX_L
  );
}

size_t 
BVH::_FindSplit(size_t first, size_t last) const
{
  uint64_t firstCode = _leaves[first].code;
  uint64_t lastCode = _leaves[last].code;

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
      uint64_t splitCode = _leaves[newSplit].code;
      size_t splitPrefix = MortonLeadingZeros(firstCode ^ splitCode);
      if (splitPrefix > commonPrefix) {
        split = newSplit;
      }
    }
  } while (step > 1);

  return split;
}


double 
BVH:: _GetCellChildrenMinimumSilhouetteDistanceSq(const BVH::Cell* cell, const pxr::GfVec3f& point) const
{
  if(!cell) return DBL_MAX;
  if(cell->GetType() == BVH::Cell::LEAF)
    return (point - _ConstraintPointOnSilhouette(point, *cell)).GetLengthSq();

  double leftDistanceSq = (point - _ConstraintPointOnSilhouette(point, *_GetCell(cell->GetLeft()))).GetLengthSq();
  double rightDistanceSq = (point - _ConstraintPointOnSilhouette(point, *_GetCell(cell->GetRight()))).GetLengthSq();

  return leftDistanceSq < rightDistanceSq ? leftDistanceSq : rightDistanceSq;
}

pxr::GfVec2f _GetCellDistancesSq(const BVH::Cell* cell, const pxr::GfVec3f& point)
{
  return pxr::GfVec2f(
    cell->GetDistanceSquared(point),
    (point - _ConstraintPointOnSilhouette(point, *cell)).GetLengthSq()
  );
}

const Morton&
BVH::_CellToMorton(size_t cellIdx) const
{
  short cellType = _GetCell(cellIdx)->GetType();
  if(cellType == BVH::Cell::BRANCH || cellType == BVH::Cell::ROOT)
    return _branches[_cellToMorton[cellIdx]];
  else
    return _leaves[_cellToMorton[cellIdx]];
}

size_t 
BVH::_LeftOrRight(const BVH::Cell* cell, const pxr::GfVec3f &point, uint64_t code, double maxDistance) const
{
  double maxDistanceSq = maxDistance * maxDistance;

  const double leftDistanceSq = _GetCell(cell->GetLeft())->GetDistanceSquared(point);
  const double rightDistanceSq = _GetCell(cell->GetRight())->GetDistanceSquared(point);

  if(leftDistanceSq > maxDistanceSq && rightDistanceSq > maxDistanceSq)
    return 3;
  if( rightDistanceSq > maxDistanceSq)
    return 0;
  else if(leftDistanceSq > maxDistanceSq)
    return 1;

  return 2;

  return 2;
  const Morton &cellMorton = _CellToMorton(_GetIndex(cell));
  const Morton &leftMorton = _CellToMorton(cell->GetLeft());
  const Morton &rightMorton = _CellToMorton(cell->GetRight());

  size_t msb = 63 - MortonLeadingZeros(code ^ cellMorton.code);
  size_t leftMsb = 63 - MortonLeadingZeros(code ^ leftMorton.code);
  size_t rightMsb = 63 - MortonLeadingZeros(code ^ rightMorton.code);

  if(leftMsb == rightMsb) {
    return 2;
  }
  
  else if ((leftMsb + 1) / 3 == (rightMsb + 1) / 3) {
    size_t level = (msb + 1) / 3;
    size_t cellBits = MortonAtLevel(cellMorton.code, level);
    size_t leftBits = MortonAtLevel(leftMorton.code, level);
    size_t rightBits = MortonAtLevel(rightMorton.code, level);

    if(cellBits ^ leftBits < cellBits ^ rightBits)return 0;
    else if(cellBits ^ leftBits > cellBits ^ rightBits)return 1;
    else return 2;
  }
  
  return leftMsb > rightMsb ? 0 : 1;

  /*
  std::cout << "==================" << std::endl;
  std::cout << "cell msb : " << ( 63 - MortonLeadingZeros(code ^ cellMorton.code)) << std::endl;
  std::cout << "left msb : " << ( 63 - MortonLeadingZeros(code ^ leftMorton.code)) << std::endl;
  std::cout << "right msb : " << ( 63 - MortonLeadingZeros(code ^ rightMorton.code)) << std::endl;

  


  size_t leftCellPrefix = MortonLeadingZeros(leftMorton.code ^ code);
  size_t leftMinPrefix = MortonLeadingZeros(leftMorton.minimum ^ code);
  size_t leftMaxPrefix = MortonLeadingZeros(leftMorton.maximum ^ code);
  size_t leftPrefix = pxr::GfMax(leftCellPrefix, leftMinPrefix, leftMaxPrefix);

  
  size_t rightCellPrefix = MortonLeadingZeros(rightMorton.code ^ code);
  size_t rightMinPrefix = MortonLeadingZeros(rightMorton.minimum ^ code);
  size_t rightMaxPrefix = MortonLeadingZeros(rightMorton.maximum ^ code);
  size_t rightPrefix = pxr::GfMax(rightCellPrefix, rightMinPrefix, rightMaxPrefix);

  if(leftPrefix == rightPrefix){};
  else return leftPrefix > rightPrefix ? 0 : 1;
  
  
  const pxr::GfVec2f leftDistances = _GetCellDistancesSq(_GetCell(cell->GetLeft()), point);
  const pxr::GfVec2f rightDistances = _GetCellDistancesSq(_GetCell(cell->GetRight()), point);

  if(leftDistances[0] > maxDistance && rightDistances[0] > maxDistance)
    return 3;
  if( rightDistances[0] > maxDistance)
    return 0;
  else if(leftDistances[0] > maxDistance)
    return 1;

  return 2;
  
}

size_t 
BVH::_FindClosestBranch(const BVH::Cell* cell, const pxr::GfVec3f& point, uint64_t code) const
{

  size_t cellIdx = _GetIndex(cell);
  const BVH::Cell* left = _GetCell(cell->GetLeft());
  const BVH::Cell* right = _GetCell(cell->GetRight());

  if(left->GetType() == BVH::Cell::LEAF || right->GetType() == BVH::Cell::LEAF)
    return cellIdx;

  size_t leftPrefix = MortonLeadingZeros(_CellToMorton(cell->GetLeft()).code ^ code);
  size_t rightPrefix = MortonLeadingZeros(_CellToMorton(cell->GetRight()).code ^ code);

  if(leftPrefix == rightPrefix)return cellIdx;
  
  return leftPrefix > rightPrefix ? _FindClosestBranch(left, point, code) : _FindClosestBranch(right, point, code);


}

size_t 
BVH::_FindClosestCell(const BVH::Cell* cell, const pxr::GfVec3f& point, uint64_t code, double maxDistanceSq) const
{
  if(cell->GetType() == BVH::Cell::LEAF)return _GetIndex(cell);
  
  const BVH::Cell* left = _GetCell(cell->GetLeft());
  const BVH::Cell* right = _GetCell(cell->GetRight());

  switch(_LeftOrRight(cell, point, code, maxDistanceSq)) {

    case 0:
      return _FindClosestCell(left, point, code, maxDistanceSq);

    case 1:
      return _FindClosestCell(right, point, code, maxDistanceSq);

    case 2:
    {
      size_t leftIdx = _FindClosestCell(left, point, code, maxDistanceSq);
      size_t rightIdx = _FindClosestCell(right, point, code, maxDistanceSq);

      if(_GetCell(leftIdx)->GetDistanceSquared(point) < _GetCell(rightIdx)->GetDistanceSquared(point))
        return leftIdx;
      else 
        return rightIdx;

    }
     case 3:
        return BVH::INVALID_INDEX;
  }

  /*
  if(cell->GetDistanceSquared(point) > maxDistanceSq)return BVH::INVALID_INDEX;

  if(cell->GetType() == BVH::Cell::LEAF)return _GetIndex(cell);
  
  const BVH::Cell* left = _GetCell(cell->GetLeft());
  const BVH::Cell* right = _GetCell(cell->GetRight());

  //code = _ComputeCode(_ConstraintPointInRange(point, *cell));

  double leftDistanceSq = pxr::GfMin(
    _GetCellChildrenMinimumSilhouetteDistanceSq(_GetCell(left->GetLeft()), point),
    _GetCellChildrenMinimumSilhouetteDistanceSq(_GetCell(left->GetRight()), point)
  );
  double rightDistanceSq = pxr::GfMin(
    _GetCellChildrenMinimumSilhouetteDistanceSq(_GetCell(right->GetLeft()), point),
    _GetCellChildrenMinimumSilhouetteDistanceSq(_GetCell(right->GetRight()), point)
  );
  maxDistanceSq = pxr::GfMin((leftDistanceSq < rightDistanceSq ? leftDistanceSq : rightDistanceSq), maxDistanceSq);
  
  switch(_LeftOrRight(cell, point, code, maxDistanceSq)) {

    case 0:
      return _FindClosestCell(left, point, code, maxDistanceSq);

    case 1:
      return _FindClosestCell(right, point, code, maxDistanceSq);

    case 2:
    {
      if(leftDistanceSq < rightDistanceSq)
        return _FindClosestCell(left, point, code, maxDistanceSq);
      else 
        return _FindClosestCell(right, point, code, maxDistanceSq);

    }
     case 3:
        return BVH::INVALID_INDEX;
  }
  */
}

size_t 
BVH::_FindClosestCell(const pxr::GfVec3f &point, size_t first, size_t last, Location* hit, size_t closest, size_t num)const
{
  if(first >= last)return closest;
  size_t middle = (first + last) >> 1;
  double ratio = MortonRatio(_leaves[middle].code, _leaves[first].code, _leaves[last].code);
  
  size_t range = last - first; 
  size_t step = range /num;

  std::vector<size_t> samples(num);
  if(samples.size() < range)
    for(size_t n = 1; n < samples.size() - 1; ++n) {
      double blend = (double) n * 1.0 / (double) num;
      samples[n] = first + (double)range * blend;
    }
  
  else {
    step = 0;
    samples.resize(range);
    for(size_t s = 0; s < samples.size(); ++s) {
      samples[s] = first + s;
    }
  }
  
  samples.front() = first;
  samples.back() = last;
  
  for(size_t i = 0; i < samples.size(); ++i)
    if(_Closest(_GetCell(_leaves[samples[i]].data), point, hit, hit->GetT()))
      closest = samples[i];

  first = closest > (range >> 2) ? closest - (range >> 2) : 0 ;
  last = closest + (range >> 2) < _leaves.size() - 1 ? closest + (range >> 2) : _leaves.size() - 1;

  return _FindClosestCell(point, first, last, hit, closest, num > 4 ? num >> 1 : 4);

}

size_t 
BVH::_FindClosestCell(const pxr::GfVec3f &point, uint64_t code)const
{
  size_t numSamples = 64;

  Location hit;
  size_t closestIdx = 0;

  uint64_t centerCode = MortonEncode3D(pxr::GfVec3i(MORTON_MAX_L >> 1, MORTON_MAX_L >> 1, MORTON_MAX_L >> 1));
  uint32_t centerIdx = MortonLowerBound(&_leaves[0], 0, _leaves.size()-1, centerCode);

  std::vector<int> sortedByDistanceIndex(8);
  std::iota(sortedByDistanceIndex.begin(), sortedByDistanceIndex.end(), 0);
  std::sort(
    sortedByDistanceIndex.begin(), sortedByDistanceIndex.end(),
      [&](int a, int b) -> bool {
        return MortonLeadingZeros(code ^ MortonGetCorner(a)) > MortonLeadingZeros(code ^ MortonGetCorner(b));
      });


  uint64_t startCode, endCode;
  for(size_t c = 0; c < 8; ++c) {
    uint64_t cornerCode = MortonGetCorner(sortedByDistanceIndex[c]);
    if(cornerCode > centerCode) {
      startCode = centerCode;
      endCode = cornerCode;
    } else {
      startCode = cornerCode;
      endCode = centerCode;
    }

    if(c > 0 && hit.GetT() < DBL_MAX) {
      uint64_t minCode = _ComputeCode(point - pxr::GfVec3f(hit.GetT()));
      uint64_t maxCode = _ComputeCode(point + pxr::GfVec3f(hit.GetT()));
      if(!MortonCheckBoxIntersects(minCode, maxCode, startCode, endCode))continue;
    }

    size_t startIdx = MortonLowerBound(&_leaves[0], 0, _leaves.size()-1, startCode);
    size_t endIdx = MortonUpperBound(&_leaves[0], 0, _leaves.size()-1, endCode);

    closestIdx = _FindClosestCell(point, startIdx, endIdx, &hit, closestIdx, numSamples);

  }

  return closestIdx;

  /*



  
  size_t lFirst = 0;
  size_t lLast = _leaves.size() - 1;
  size_t rFirst = 0;
  size_t rLast = _leaves.size() - 1;

  while(lFirst != lLast && rFirst != rLast) {
    
    size_t lMiddle = (lFirst + lLast) >> 1;
    size_t rMiddle = (rFirst + rLast) >> 1;

    if(code < _leaves[lMiddle].code)
      lLast = lMiddle;

    else if(code > _leaves[lMiddle].code)
      lFirst = lMiddle + 1;

    if(code > _leaves[rMiddle].code)
      rLast = rMiddle;

    else if(code < _leaves[rMiddle].code)
      rFirst = rMiddle + 1;
  }


  uint64_t bigmin = MortonBigMin(code, _leaves[lFirst].code, _leaves[rFirst].code);
  uint64_t litmax = MortonLitMax(code, _leaves[lFirst].code, _leaves[rFirst].code);

  size_t bigminIdx = _FindClosestCell(bigmin);
  size_t litmaxIdx = _FindClosestCell(litmax);

  double minDistSq = DBL_MAX;
  size_t closest = lFirst;
  for(size_t index = lFirst; index <= rFirst; ++index) {
    double distSq = _GetCell(_leaves[index].data)->GetDistanceSquared(point);
    if(distSq < minDistSq) {
      closest = index;
      minDistSq = distSq;
    }
  }

  for(size_t index = litmaxIdx; index <= rFirst; ++index) {
    double distSq = _GetCell(_leaves[index].data)->GetDistanceSquared(point);
    if(distSq < minDistSq) {
      closest = index;
      minDistSq = distSq;
    }
  }

  return _leaves[closest].data;

  */
}

size_t 
BVH::_FindClosestCell(uint64_t code)const
{
  size_t first = 0;
  size_t last = _leaves.size() - 1;
  size_t closest = 0;

  while(first != last) {
    size_t middle = (first + last)>>1;
    closest = middle;
    if(code < _leaves[middle].code)
      last = middle;

    else if(code > _leaves[middle].code) 
      first = middle + 1;

    else break;
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

  _leaves.clear();
  _leaves.reserve(numComponents);

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
          _leaves.push_back({ code, minimum, maximum, leafIdx});
        }  
        
      }
    }
    SetGeometryCellIndices(g, start, _cells.size());
  }

  size_t numLeaves = _cells.size();
  Morton morton = SortCells();
  size_t numBranches = _cells.size() - numLeaves;

  _root = _GetCell(morton.data);
  _root->SetType(BVH::Cell::ROOT);

  
  _branches.reserve(numBranches);

  for(size_t c = numLeaves; c < _cells.size(); ++c) { 
    _branches.push_back({
      _ComputeCode(_cells[c].GetMidpoint()),
      _ComputeCode(_cells[c].GetMin()),
      _ComputeCode(_cells[c].GetMax()), c});
  }
  std::sort(_branches.begin(), _branches.end());
  
  _cellToMorton.resize(_cells.size(), BVH::INVALID_INDEX);
  for(size_t m = 0; m < _leaves.size(); ++m)
    _cellToMorton[_leaves[m].data] = m;

  
  for(size_t m = 0; m < _branches.size(); ++m)
    _cellToMorton[_branches[m].data] = m;


  size_t quarter = MORTON_MAX_L >> 2;

  uint64_t splits[8];
  for(size_t i = 0; i < 8; ++i)splits[i] = MortonGetSplit(i);

  std::cout << MortonGetCorner(3) << " " << MortonGetSplit(3) << " " << MortonGetCorner(4) << std::endl;

  for(size_t i = 0; i < 8; ++i) {
    std::cout << std::hex << splits[i] << " " << std::bitset<64>(splits[i]) << std::endl;

    std::cout << MortonToWorld(*this, MortonDecode3D(splits[i])) << std::endl;
  
  }

  

/*
  std::cout << "bounding box " << GetMin() << "," << GetMax() << std::endl;
  uint64_t code = _ComputeCode(pxr::GfVec3f(4, 8, 7));

  uint64_t minCode = _ComputeCode(GetMin());
  uint64_t maxCode = _ComputeCode(GetMax());

  uint64_t bigmin = MortonBigMin(code, minCode, maxCode);
  uint64_t litmax = MortonLitMax(code, minCode, maxCode);

  std::cout << "code : " << MortonDecode3D(code) << std::endl;
  std::cout << "minimum : " << MortonDecode3D(minCode) << std::endl;
  std::cout << "maximum : " << MortonDecode3D(maxCode) << std::endl;
  std::cout << "bigmin : " << MortonDecode3D(bigmin) << std::endl;
  std::cout << "litmax : " << MortonDecode3D(litmax) << std::endl;
  */

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
    return _leaves[first].data;
  }

  size_t split = _FindSplit(first, last);

  size_t leftIdx = _RecurseSortCells(first, split);
  size_t rightIdx = _RecurseSortCells(split + 1, last);
  return AddCell(_GetCell(leftIdx), _GetCell(rightIdx));
}

Morton 
BVH::SortCells()
{
  size_t numLeaves = _leaves.size();
  
  std::sort(_leaves.begin(), _leaves.end());

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


bool 
BVH::_ShouldCheckCell(const BVH::Cell* cell, 
  const pxr::GfVec3f& point, double maxDistance) const
{
  if(cell->GetType() == BVH::Cell::LEAF)return true;

  const pxr::GfVec3f minPoint(cell->GetMin() - pxr::GfVec3f(maxDistance));
  const pxr::GfVec3f maxPoint(cell->GetMax() + pxr::GfVec3f(maxDistance));

  return pxr::GfRange3f(minPoint, maxPoint).Contains(point);


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


bool 
BVH::_CheckCloserCells(const BVH::Cell* cell, const pxr::GfVec3f& point, Location* hit, uint64_t code) const
{
  return _Closest(cell, point, hit, hit->GetT());


  if(!cell) return false;

  if(cell->GetDistanceSquared(point) > pxr::GfPow(hit->GetT(), 2)) 
    return false;
  
  if(cell->IsLeaf()) {
    return _Closest(cell, point, hit, hit->GetT());
  } 
  else {
    return _Closest(cell, point, hit, hit->GetT());

    /*
    const BVH::Cell* left = _GetCell(cell->GetLeft());
    const BVH::Cell* right = _GetCell(cell->GetRight());
    bool leftFound(false), rightFound(false);
    
    const double leftDistance = (point - _ConstraintPointInRange(point, *left)).GetLength();
    const double rightDistance = (point - _ConstraintPointInRange(point, *right)).GetLength();

    const double leftOutDistance = (point - _ConstraintPointOnSilhouette(point, *left)).GetLength();
    const double rightOutDistance = (point - _ConstraintPointOnSilhouette(point, *right)).GetLength();

    if(leftOutDistance < rightDistance)
      leftFound = _CheckCloserCells(left, point, hit, code);

    else if(rightOutDistance < leftDistance)
      rightFound = _CheckCloserCells(right, point, hit, code);

    else {
  
      size_t choice = _LeftOrRight(cell, point, code, hit->GetT() * hit->GetT());
      switch(choice) {
        case 0:
          leftFound = _CheckCloserCells(left, point, hit, code);
          break;

        case 1:
          rightFound = _CheckCloserCells(right, point, hit, code);
          break;

        default: 
          return _Closest(cell, point, hit, hit->GetT());
      }
    //}

    return leftFound || rightFound;
    */
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
  Morton morton = {pntCode, pntCode, pntCode, NULL};

  /*
  size_t numSamples = 64;
  size_t step = _leaves.size() / numSamples;
  for(size_t i = 0; i < numSamples; ++i)
    _Closest(_GetCell(_leaves[i*step].data), point, hit, maxDistance);
  */

  //size_t closestBranchIdx = _FindClosestBranch(_root, point, pntCode);
  //const BVH::Cell* closestBranch = &_cells[closestBranchIdx];

  //size_t closestCellIdx = _FindClosestCell(closestBranch, point, pntCode, maxDistance);
  //const BVH::Cell* closestCell = &_cells[closestCellIdx];

  
  size_t closestCellIdx = _FindClosestCell(point, pntCode);
  const BVH::Cell* closestCell = &_cells[_leaves[closestCellIdx].data];

  _Closest(closestCell, point, hit, hit->GetT());
  
  
  //_Closest(closest, point, hit, maxDistance);
  //_Closest(_root, point, hit, hit->GetT());
  //_CheckCloserCells(_root, point, hit, pntCode);
  return true;

  if(_Closest(NULL, point, hit, maxDistance)) {
    Morton morton = {
      pntCode, 
      _ComputeCode(point - pxr::GfVec3f(hit->GetT())),
      _ComputeCode(point + pxr::GfVec3f(hit->GetT())), 
      INVALID_INDEX
    };

    //_CheckCloserCells(_root, point, hit, pntCode);

/*
    size_t start, end;

    start = _FindClosestCell(point - pxr::GfVec3f(hit->GetT()));
    end = _FindClosestCell(point + pxr::GfVec3f(hit->GetT()));
    for(size_t m = start; m < end; ++m)
      //if(_mortons[m].maximum > morton.minimum && _mortons[m].minimum < morton.maximum)
        if(_ShouldCheckCell(_GetCell(_mortons[m].data), point, hit->GetT()))
          _Closest(_GetCell(_mortons[m].data), point, hit, hit->GetT());
    */
    //_RecurseClosestCell(_root, point, hit);

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