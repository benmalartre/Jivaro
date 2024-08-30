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

BVH::Cell::Cell(size_t lhs, const BVH::Cell* left, size_t rhs, const BVH::Cell* right)
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
BVH::_Raycast(const BVH::Cell* cell, const pxr::GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{
  double enterDistance, exitDistance;
  if (!ray.Intersect(pxr::GfBBox3d(*this), &enterDistance, &exitDistance))
    return false;

  if(enterDistance > maxDistance) 
    return false;
  std::cout << "-Raycast " << _GetIndex(cell) << std::endl;
  size_t geomIdx = GetGeometryIndexFromCell(cell);
  const Geometry* geometry = GetGeometry(geomIdx);

  if (cell->IsLeaf()) {
    
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
        hit->Set(leftHit); hit->SetGeometryIndex(geomIdx); return true;
      } else {
        hit->Set(rightHit); hit->SetGeometryIndex(geomIdx); return true;
      }
    } else if (leftFound) {
      { hit->Set(leftHit); hit->SetGeometryIndex(geomIdx); return true;}
    } else if (rightFound) {
      { hit->Set(rightHit); hit->SetGeometryIndex(geomIdx); return true;}
    } return false;
  }
  
}

bool 
BVH::_Closest(const BVH::Cell* cell, const pxr::GfVec3f& point, Location* hit, 
  double maxDistance) const
{  
  double distance = GetDistance(*this, point) - GetSize().GetLength() * 0.5f;
  if (distance > hit->GetT() || distance > maxDistance)return false;

  size_t geomIdx = GetGeometryIndexFromCell(cell);

  if (cell->IsLeaf()) {
    const Geometry* geometry = GetGeometry(geomIdx);

    const pxr::GfVec3f* points = ((const Deformable*)geometry)->GetPositionsCPtr();
    Component* component = (Component*)cell->GetData();
    pxr::GfVec3f localPoint = geometry->GetInverseMatrix().Transform(point);
    Location localHit(*hit);
    localHit.TransformT(geometry->GetInverseMatrix());

    component->Closest(points, localPoint, &localHit);

    Triangle* triangle = ((Mesh*)geometry)->GetTriangle(localHit.GetComponentIndex());
    const pxr::GfVec3d localHitPoint =
      localHit.ComputePosition(points, &triangle->vertices[0], 3);

    const double distance = (point - geometry->GetMatrix().Transform(localHitPoint)).GetLength();
    if (distance < hit->GetT()) {
      hit->Set(localHit);
      hit->SetT(distance);
      hit->SetGeometryIndex(geomIdx);
      return true;
    }
    return false;
  }
  else {

    const BVH::Cell* left = _GetCell(cell->GetLeft());
    const BVH::Cell* right = GetCell(cell->GetRight());
    if(left && right) 
    {
      const BVH::Cell* root = GetRoot();
      uint64_t code = _ComputeCode( point);
      uint64_t leftCode = _ComputeCode( left->GetMidpoint());
      uint64_t rightCode = _ComputeCode( right->GetMidpoint());

      int leftPrefix = MortonLeadingZeros(leftCode ^ code);
      int rightPrefix = MortonLeadingZeros(rightCode ^ code);
      if(rightPrefix > leftPrefix)
        return _Closest(right, point, hit, maxDistance);
      else if(leftPrefix > rightPrefix)
        return _Closest(left, point, hit, maxDistance);
      else {
        

        bool leftFound = false, rightFound = false;
        leftFound = _Closest(left, point, hit, maxDistance);
        rightFound = _Closest(right, point, hit, maxDistance);

        return leftFound || rightFound;
      }
    } 

    else if(left)
      return _Closest(left, point, hit, maxDistance);

    else if(right)
      return _Closest(right, point, hit, maxDistance);

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
  uint64_t morton = _ComputeCode( point);
  pxr::GfVec3i p = MortonDecode3D(morton);
  return pxr::GfVec3d(
    p[0] / (float)MORTOM_MAX_L, 
    p[1] / (float)MORTOM_MAX_L, 
    p[2] / (float)MORTOM_MAX_L
  );
}

int 
BVH::_FindSplit(int first, int last)
{
  uint64_t firstCode = _mortons[first].code;
  uint64_t lastCode = _mortons[last].code;

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
      uint64_t splitCode = _mortons[newSplit].code;
      int splitPrefix = MortonLeadingZeros(firstCode ^ splitCode);
      if (splitPrefix > commonPrefix) {
        split = newSplit;
      }
    }
  } while (step > 1);

  return split;
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
  size_t cellIdx = _GetIndex(cell);
  for (size_t g = 0; g < GetNumGeometries(); ++g) {
    if (cellIdx >= GetGeometryCellsStartIndex(g) && cellIdx < GetGeometryCellsEndIndex(g))
      return GetGeometry(g);
  }
  return NULL;
}

size_t
BVH::GetGeometryIndexFromCell(const BVH::Cell* cell) const
{
  size_t cellIdx = _GetIndex(cell);

  for (size_t g = 0; g < GetNumGeometries(); ++g) {
    std::cout << "Get geometry index from cell : " << cellIdx << std::endl;
    std::cout << " - start index : " << GetGeometryCellsStartIndex(g) << std::endl;
    std::cout << " - end index : " << GetGeometryCellsEndIndex(g) << std::endl;

    if (cellIdx >= GetGeometryCellsStartIndex(g) && cellIdx < GetGeometryCellsEndIndex(g))
      return g;
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
        _mortons.resize(numTrianglePairs);
        const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
        const pxr::GfMatrix4d& matrix = mesh->GetMatrix();
        for (size_t t = 0; t < numTrianglePairs; ++t) {
          size_t leafIdx = AddCell(&trianglePairs[t],
            trianglePairs[t].GetBoundingBox(positions, matrix));
    
          uint64_t code = _ComputeCode(_GetCell(leafIdx)->GetMidpoint());
          _mortons[t] = { 
            code, 
            code,
            code,
            leafIdx
          };
        }  
        SetGeometryCellIndices(g, start, _cells.size());
      }
    }
  }

  Morton morton = SortCells();
  BVH::Cell* top = _GetCell(morton.data);

  _SwapCells(&_root, top);



  std::cout << "bvh initialized" << morton.code << std::endl;
  
}

void
BVH::Update()
{
  pxr::GfRange3f newRange = _RecurseUpdateCells(&_root);
  SetMin(newRange.GetMin());
  SetMax(newRange.GetMax());
}

bool BVH::Raycast(const pxr::GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{
  for (size_t g = 0; g < GetNumGeometries(); ++g) {
    size_t index = GetGeometryCellIndex(g);
    std::cout << "raycast geom index " << index << std::endl;
    const BVH::Cell* cell = _GetCell(index);
    std::cout << "cekk : " << cell << std::endl;
    // find the first
    if (_Raycast(cell, ray, hit, maxDistance, minDistance))
      hit->SetGeometryIndex(g);
  }
  return hit->IsValid();
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
    const BVH::Cell* root = GetRoot();
    uint64_t code = _ComputeCode( _GetCell(leafIdx)->GetMidpoint());
    _mortons[t] = { 
      code, 
      code,
      code,
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
    const BVH::Cell* root = GetRoot();
    uint64_t code = _ComputeCode(_GetCell(leafIdx)->GetMidpoint());
    _mortons[t] = { 
      code,
      code,
      code,
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
  int           first,
  int           last)
{
  if (first == last) {
    return _mortons[first].data;
  }

  int split = _FindSplit(first, last);

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

void BVH::_SwapCells(BVH::Cell* lhs, BVH::Cell* rhs)
{
  lhs->SetRight(rhs->GetRight());
  lhs->SetLeft(rhs->GetLeft());
  lhs->SetRight(rhs->GetRight());
  lhs->SetMin(rhs->GetMin());
  lhs->SetMax(rhs->GetMax());
  rhs->SetLeft(BVH::INVALID_INDEX);
  rhs->SetRight(BVH::INVALID_INDEX);
  //_cells.pop_back();
}

pxr::GfVec3f
BVH::GetMortonColor(const pxr::GfVec3f& point)
{
  return pxr::GfVec3f(_ComputeCodeAsColor(point));
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

*/

bool BVH::Closest(const pxr::GfVec3f& point, 
  Location* hit, double maxDistance) const
{

  for (size_t g = 0; g < GetNumGeometries(); ++g) {
    size_t index = GetGeometryCellIndex(g);
    const BVH::Cell* cell = _GetCell(index);
    
    // find the first
    if (_Closest(_GetCell(cell->GetLeft()), point, hit, maxDistance))
      hit->SetGeometryIndex(g);
  }
  return hit->IsValid();
}

void BVH::GetCells(pxr::VtArray<pxr::GfVec3f>& positions,
  pxr::VtArray<pxr::GfVec3f>& sizes, pxr::VtArray<pxr::GfVec3f>& colors, bool branchOrLeaf)
{
  std::vector<const BVH::Cell*> cells;
  if(branchOrLeaf)GetLeaves(&_root, cells);
  else GetBranches(&_root, cells);
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