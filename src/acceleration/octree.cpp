//======================================================
// OCTREE IMPLEMENTATION
//======================================================
#include "../acceleration/octree.h"
#include "../geometry/point.h"
#include "../geometry/edge.h"
#include "../geometry/triangle.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"

JVR_NAMESPACE_OPEN_SCOPE


const int Octree::MAX_ELEMENTS_NUMBER = 32;

// destructor
Octree::Cell::~Cell()
{
  for (int i=0; i<8; i++)
  {
    if (_child[i])delete _child[i];
    _child[i] = NULL;
  }
  if(_elements.size())_elements.clear();
}

// clear tree
void Octree::Cell::ClearTree()
{
  for (int i=0; i<8; i++)
  {
    if (_child[i])delete _child[i];
    _child[i] = NULL;
  }
  if(_elements.size())_elements.clear();
}

// get distance
float Octree::Cell::GetDistance(const pxr::GfVec3f &point) const
{
  float dx = _GetDistance1D(point[0], _min[0], _max[0]);
  float dy = _GetDistance1D(point[1], _min[1], _max[1]);
  float dz = _GetDistance1D(point[2], _min[2], _max[2]);
  return sqrt(dx * dx + dy * dy + dz * dz);
}

// intersect sphere
bool Octree::Cell::IntersectSphere(const pxr::GfVec3f& center, const float radius) const
{
  float r2 = radius * radius;
  float dmin = 0;

  for(int ii = 0; ii < 3; ii++ )
  {
    if( center[ii] < _min[ii] ) dmin += _Squared( center[ii] - _min[ii] );
    else if( center[ii] > _max[ii] ) dmin += _Squared( center[ii] - _max[ii] );
  }
  return dmin <= r2;
}

// get bounding box
void Octree::Cell::GetBoundingBox(const pxr::GfVec3f* points, pxr::VtArray<int>& vertices)
{
  // reset bounding box
  _min[0] = FLT_MAX;
  _min[1] = FLT_MAX;
  _min[2] = FLT_MAX;

  _max[0] = -FLT_MAX;
  _max[1] = -FLT_MAX;
  _max[2] = -FLT_MAX;
    
  pxr::GfVec3f tmp;
  unsigned int numVertices = vertices.size();
  for (int i = 0; i < numVertices; i++)
  {
    tmp = points[vertices[i]];

    if (tmp[0] < _min[0])_min[0] = tmp[0];
    if (tmp[1] < _min[1])_min[1] = tmp[1];
    if (tmp[2] < _min[2])_min[2] = tmp[2];

    if (tmp[0] > _max[0])_max[0] = tmp[0];
    if (tmp[1] > _max[1])_max[1] = tmp[1];
    if (tmp[2] > _max[2])_max[2] = tmp[2];
  }
    
  // extend bounding box
  _min[0] -= 0.1f;
  _min[1] -= 0.1f;
  _min[2] -= 0.1f;
  _max[0] += 0.1f;
  _max[1] += 0.1f;
  _max[2] += 0.1f;
}


// get furthest corner
void Octree::Cell::GetFurthestCorner(const pxr::GfVec3f& point, pxr::GfVec3f& corner)
{
  pxr::GfVec3f delta;
  float dist;
  float furthestDist=-1.0f;
    
  float P[6] = {
    (float)_min[0],
    (float)_min[1],
    (float)_min[2],
    (float)_max[0],
    (float)_max[1],
    (float)_max[2]
  };
    
  static const int permutation[24] = {0,1,2,3,1,2,0,1,5,3,1,5,
      0,4,2,3,4,2,0,4,5,3,4,5};
    
  for(unsigned int z=0;z<8;z++)
  {
    pxr::GfVec3f currentCorner(P[permutation[z*3]],P[permutation[z*3+1]],P[permutation[z*3+2]]);
    delta = point - currentCorner;
    dist = (float)delta.GetLength();
    if(dist>furthestDist)
    {
      furthestDist = dist;
      corner = currentCorner;
    }
  }
}

// build tree
void Octree::Cell::BuildTree(Component* components, size_t num, Geometry* geometry)
{
  ClearTree();
    
  if(geometry->GetType() < Geometry::POINT) {
    // TODO add warning message here
    return;
  }
  // loop over all elements, insert all leaves to the tree
  for(size_t i = 0; i < num; ++i)
  {
    Insert(&components[i]);
  }
    
  Split(((Deformable*)geometry)->GetPositionsCPtr());
}

// split tree
void Octree::Cell::Split(const pxr::GfVec3f* points)
{
  int esz = _elements.size();

  if (esz <= MAX_ELEMENTS_NUMBER || (esz <= 2*MAX_ELEMENTS_NUMBER && _depth > 3) ||(esz <= 3*MAX_ELEMENTS_NUMBER && _depth > 4) ||_depth > 6 )
  {
      _isLeaf = true;
      return;
  }
    
  _isLeaf = false;
    
  double xx[] = {_min[0], 0.5*(_min[0]+_max[0]), _max[0]};
  double yy[] = {_min[1], 0.5*(_min[1]+_max[1]), _max[1]};
  double zz[] = {_min[2], 0.5*(_min[2]+_max[2]), _max[2]};
    
  pxr::GfVec3f center, halfSize;

  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 2; ++j) {
      for (int k = 0; k < 2; ++k) {
        int m = 4 * i + 2 * j + k;
        _child[m] = new Cell(pxr::GfVec3f(xx[i], yy[j], zz[k]),
          pxr::GfVec3f(xx[i + 1], yy[j + 1], zz[k + 1]), _depth + 1);

        center = _child[m]->GetCenter();
        halfSize = _child[m]->GetHalfSize();
        int esz = _elements.size();

        for (int e = 0; e < esz; ++e) {
          if (_elements[e]->Touch(points, center, halfSize)) {
            _child[m]->Insert(_elements[e]);
          }
        }

        if (_child[m]->NumElements() == 0) {
          delete _child[m];
          _child[m] = NULL;
        } else {
          _child[m]->Split(points);
        }
      }
    }
  }
  _elements.clear();
}

void 
Octree::Init(const std::vector<Geometry*>& geometries)
{

}

void 
Octree::Update()
{

}

bool 
Octree::Raycast(const pxr::GfRay& ray, Location* hit, 
  double maxDistance, double* minDistance) const
{
  return false;
}

bool 
Octree::Closest(const pxr::GfVec3f& point, 
  Location* hit, double maxDistance) const
{
  Cell* closestCell = NULL;
  _GetClosestCell(point, closestCell);

  const pxr::GfVec3f* positions = NULL;

  // brute force neighbor cell
  //std::vector<Triangle*>::iterator tri = closestCell->getTriangles().begin();
  Component* comp;
  float closestDistance = FLT_MAX;

  for(unsigned e = 0; e < closestCell->NumElements(); ++e)
  {
    comp = closestCell->GetElement(e);
    comp->Closest( positions, point , hit);
  }
    
  std::vector<Cell*> nearbyCells;
  if(!_octree.IsLeaf())
  {
    _GetNearbyCells(point, closestCell, nearbyCells, closestDistance);
  }
    
  // loop nearby cells
  std::vector<Cell*>::iterator nearby = nearbyCells.begin();
  for(; nearby < nearbyCells.end(); ++nearby)
  {
    for(unsigned e = 0; e <(*nearby)->NumElements(); ++e)
    {
      comp = (*nearby)->GetElement(e);
      comp->Closest( positions, point , hit);
    }
  }
  return hit->GetGeometryIndex() != -1 && hit->GetComponentIndex() != -1;
}

void 
Octree::_GetClosestCell(const pxr::GfVec3f& point, Cell*& closestCell) const
{
  float closestDistance = FLT_MAX;
    
  // the case of low polygon count
  if(_octree.IsLeaf())
  {
    closestCell = (Cell*)&_octree;
  }
    
  // normal case
  else
  {
    for(unsigned int j = 0; j < 8; ++j)
    {
      if(_octree.GetCell(j) != NULL)
        _RecurseGetClosestCell(point, _octree.GetCell(j), closestDistance, closestCell);
    }
  }
}

void 
Octree::_RecurseGetClosestCell(const pxr::GfVec3f& point, const Cell* cell, 
  float& closestDistance, Cell*& closestCell) const
{
  if(cell==NULL)return;
  float distance;
  if(cell->IsLeaf())
  {
    distance = cell->GetDistance(point);
    if(distance < closestDistance)
    {
      closestDistance = distance;
      closestCell = (Cell*)cell;
    }
  }
  else
  {
    int cid = -1;
    float cdist = FLT_MAX;
    for(unsigned int k = 0; k < 8; ++k)
    {
      const Cell* current = cell->GetCell(k);
            
      if(current!=NULL)
      {
        distance = current->GetDistance(point);
        if(distance<=cdist)
        {
          cdist = distance;
          cid = k;
        }
      }
    }
    if(cid >= 0)
      _RecurseGetClosestCell(point, cell->GetCell(cid), closestDistance, closestCell);
  }
}

void 
Octree::_GetNearbyCells(const pxr::GfVec3f& point, const Cell* cell, 
    std::vector<Cell*>& cells, float closestDistance) const
{
  // the case of low polygon count
  if(!_octree.IsLeaf())
  {
    for(unsigned int j = 0; j < 8; ++j)
    {
      _RecurseGetNearbyCells(_octree.GetCell(j), point, closestDistance, cells);
    }
  }
}

void 
Octree::_RecurseGetNearbyCells(const Cell* cell, const pxr::GfVec3f& center, 
    const float radius, std::vector<Cell*>& cells) const
{
  if(cell == NULL) return;
  Cell* child = NULL;
  if(!cell->IsLeaf()) {
    for(unsigned int k = 0; k < 8; ++k) {
      child = (Cell*)cell->GetCell(k);
      if (child != NULL) {
        if (child->IntersectSphere(center, radius)) {
          _RecurseGetNearbyCells(child, center, radius, cells);
        }
      }
    }
  } else {
    if(cell->IntersectSphere(center, radius)) {
      cells.push_back((Cell*)cell);
    }
  }
}

JVR_NAMESPACE_CLOSE_SCOPE