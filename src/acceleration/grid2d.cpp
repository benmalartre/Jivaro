#include "../acceleration/grid2d.h"
#include "../geometry/triangle.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"

AMN_NAMESPACE_OPEN_SCOPE

// delete all cells
void Grid2DIntersector::DeleteCells()
{
  if(_cells)
  {
    for (uint32_t i = 0; i < _numCells; ++i)
      if (_cells[i] != NULL) delete _cells[i];
    delete [] _cells;
    _cells = NULL;
  }
}

// reset all cells
void Grid2DIntersector::ResetCells()
{
  if(_cells)
  {
    for (uint32_t i = 0; i < _numCells; ++i)
      if (_cells[i] != NULL) _cells[i]->_hit = false;
  }
}

void Grid2DIntersector::InsertMesh(Mesh* mesh)
{

}

void Grid2DIntersector::InsertCurve(Curve* curve)
{

}

void Grid2DIntersector::InsertPoints(Points* points)
{

}

/*
void Grid3D::PlaceIntoGrid(Mesh* mesh, std::vector<Vertex*>& points, 
  const MMatrix& M, float cellSize)
{
  // delete old cells
  deleteCells();

  // compute bound of the scene
  uint32_t totalNumPoints = points.size();

  _bbox.compute(mesh);

  // create the grid
  pxr::GfVec3f size = _bbox.max() - _bbox.min();

 cellSize = fmax(cellSize, 0.01f);
  //float desiredCellSize = area*12;
  for (uint8_t i = 0; i < 3; ++i) {
      _resolution[i] = ceil(size[i] * 1.0f/cellSize)+1;
      _resolution[i] = MAX(uint32_t(1), MIN(_resolution[i], uint32_t(128)));
  }

  _cellDimension.x = size.x /(float)(_resolution[0]-1);
  _cellDimension.y = size.y /(float)(_resolution[1]-1);
  _cellDimension.z = size.z /(float)(_resolution[2]-1);

  // allocate memory
  _nc = _resolution[0] * _resolution[1] * _resolution[2];
  if(_nc > 0)_cells = new Grid3D::Cell* [_nc];
  else{
    _cells = NULL;
    return;
  }

  // set all pointers to NULL
  memset(_cells, 0x0, sizeof(Grid3D::Cell*) * _nc);

  Vertex* V;
  pxr::GfVec3f P, R;
  pxr::GfVec3f invDimensions(
    1/_cellDimension[0],
    1/_cellDimension[1],
    1/_cellDimension[2]);

  // insert all the points in the cells
  for(uint32_t i=0;i<totalNumPoints;i++)
  {
      V = points[i];
      P = MPoint(V->_position) * M;

      // convert to cell coordinates
      R.x = (P.x - _bbox.min().x + 0.5f * _cellDimension[0]) * invDimensions.x;
      R.y = (P.y - _bbox.min().y + 0.5f * _cellDimension[1]) * invDimensions.y;
      R.z = (P.z - _bbox.min().z + 0.5f * _cellDimension[2]) * invDimensions.z;

      uint32_t idz = CLAMP(floor(R[2]), 0, _resolution[2] - 1);
      uint32_t idy = CLAMP(floor(R[1]), 0, _resolution[1] - 1);
      uint32_t idx = CLAMP(floor(R[0]), 0, _resolution[0] - 1);

      // insert vertex in cell
      uint32_t o = 
        idz * _resolution[0] * _resolution[1] + idy * _resolution[0] + idx;
      if (_cells[o] == NULL) 
        _cells[o] = new Cell(o);
      //_cells[o]->_color = MVector(0.5,0.5,0.5);
      _cells[o]->Insert(V);
      _cells[o]->_grid = this;
    }
}
*/

// place a triangle mesh in the grid
void Grid2DIntersector::Init(const std::vector<Geometry*>& geometries)
{
  // delete old cells
  DeleteCells();
  _geometries.clear();
  if (!geometries.size())return;
  _geometries = geometries;

  // compute bound of the scene
  uint32_t totalNumElements = 0;
  _range.SetEmpty();
  for(Geometry* geom:_geometries) {
    const pxr::GfBBox3d& bbox = geom->GetBoundingBox();
    const pxr::GfRange3d& range = bbox.GetRange();
    _range.UnionWith(pxr::GfRange3f(
      pxr::GfVec3f(range.GetMin()), pxr::GfVec3f(range.GetMax())));
    switch (geom->GetType()) {
      case Geometry::MESH:
      {
        Mesh* mesh = (Mesh*)geom;
        totalNumElements += mesh->GetNumTriangles();
        break;
      }
      case Geometry::CURVE:
      {
        Curve* curve = (Curve*)geom;
        totalNumElements += curve->GetTotalNumSegments();
        break;
      }
      case Geometry::POINT:
      {
        Points* points = (Points*)geom;
        totalNumElements += points->GetNumPoints();
        break;
      }
    }
  }

  // create the grid
  pxr::GfVec3f size = _range.GetMax() - _range.GetMin();

  //float desiredCellSize = area*12;
  float squareRoot = 
    powf(1 * totalNumElements / (size[0] * size[1]), 1 / 2.f);
  for (uint8_t i = 0; i < 2; ++i) {
    _resolution[i] = floor(size[i] * squareRoot);
    _resolution[i] = 
      MAX(uint32_t(1), MIN(_resolution[i], uint32_t(128)));
  }

  _cellDimension[0] = size[0] /(float)_resolution[0];
  _cellDimension[1] = size[1] /(float)_resolution[1];

  // allocate memory
  _numCells = _resolution[0] * _resolution[1];
  _cells = new Grid2DIntersector::Cell* [_numCells];

  // set all pointers to NULL
  memset(_cells, 0x0, sizeof(Grid2DIntersector::Cell*) * _numCells);

  pxr::GfVec2f A, B, C;
  Triangle* T;
  unsigned offset = 0;

  pxr::GfVec2f invDimensions(1/_cellDimension[0], 1/_cellDimension[1]);

  const pxr::GfVec3f& bboxMin = _range.GetMin();
  const pxr::GfVec3f& bboxMax = _range.GetMax();
  // insert all the triangles in the cells
  for (const auto& geom : _geometries) {
    switch (geom->GetType()) {
      case Geometry::MESH:
        InsertMesh((Mesh*)&geom);
        break;
    }
  }
  
}

void Grid2DIntersector::Update(const std::vector<Geometry*>& geometries)
{

}

bool Grid2DIntersector::Raycast(const pxr::GfRay& ray, Hit* hit, 
  double maxDistance, double* minDistance) const
{
    double bmin, bmax;
    // if the ray doesn't intersect the grid return
    if(!ray.Intersect(pxr::GfBBox3d(
      pxr::GfRange3d(_range.GetMin(), _range.GetMax())), &bmin, &bmax))
    {
        return false;
    }

    // initialization step
    int32_t exit[3], step[3], cell[3];
    pxr::GfVec3d deltaT, nextCrossingT;
    pxr::GfVec3d invdir = -1.0 * ray.GetDirection();

    for (uint8_t i = 0; i < 2; ++i) {
        // convert ray starting point to cell coordinates
        double rayOrigCell = 
          ((ray.GetPoint(0.f)[i] + ray.GetDirection()[i] * bmin) - 
            _range.GetMin()[i]);
        cell[i] = CLAMP(floor(rayOrigCell / _cellDimension[i]), 0, _resolution[i] - 1);
        if(fabs(ray.GetDirection()[i]) < 0.0000001)
        {
            deltaT[i] = 0;
            nextCrossingT[i] = maxDistance;
            exit[i] = cell[i];
            step[i] = 0;
        }
        else if (ray.GetDirection()[i] < 0.0) {
            deltaT[i] = -_cellDimension[i] * invdir[i];
            nextCrossingT[i] = bmin + (cell[i] * _cellDimension[i] - rayOrigCell) * invdir[i];
            exit[i] = -1;
            step[i] = -1;
        }
        else {
            deltaT[i] = _cellDimension[i] * invdir[i];
            nextCrossingT[i] = bmin + ((cell[i] + 1)  * _cellDimension[i] - rayOrigCell) * invdir[i];
            exit[i] = _resolution[i];
            step[i] = 1;
        }
    }

    // start cell
    uint32_t o = cell[1] * _resolution[0] + cell[0];
    if(_cells[o] != NULL) _cells[o]->_hit = true;

    // walk through each cell of the grid and test for an intersection if
    // current cell contains geometry
    while(1) {
        uint32_t o = cell[1] * _resolution[0] + cell[0];
        if (_cells[o] != NULL)
        {
          /*
            if (_cells[o]->Raycast(_mesh, ray, hit, maxDistance, minDistance))
            {
                _cells[o]->_hit = true;
                hit = true;
            }
            */
        }
        uint8_t k =
          ((nextCrossingT[0] < nextCrossingT[1]) << 2) +
          ((nextCrossingT[0] < nextCrossingT[2]) << 1) +
          ((nextCrossingT[1] < nextCrossingT[2]));
        static const uint8_t map[8] = {2, 1, 2, 1, 2, 2, 0, 0};
        uint8_t axis = map[k];
        if (maxDistance <= nextCrossingT[axis]) break;
        cell[axis] += step[axis];
        if (cell[axis] == exit[axis]) break;
        nextCrossingT[axis] += deltaT[axis];
    }

    return hit;
}

bool Grid2DIntersector::Closest(const pxr::GfVec3f& point, Hit* hit,
  double maxDistance, double* minDistance) const
{
  return false;
}

pxr::GfVec3f Grid2DIntersector::GetCellPosition(uint32_t index) {
  pxr::GfVec3f position;
  const pxr::GfVec3f& bboxMin = _range.GetMin();
  position[0] = bboxMin[0] + 
    _cellDimension[0] * (index % _resolution[0]);
  position[1] = bboxMin[1] + 
    _cellDimension[1] * ((index / _resolution[0]) % _resolution[1]);
  return position;
}

pxr::GfVec3f Grid2DIntersector::GetCellMin(uint32_t index){
  return GetCellPosition(index) - _cellDimension * 0.5f;
}

pxr::GfVec3f Grid2DIntersector::GetCellMax(uint32_t index){
  return GetCellPosition(index) + _cellDimension * 0.5f;
}

bool Grid2DIntersector::Cell::Raycast(Geometry* geom, const pxr::GfRay& ray,
  Hit* hit, double maxDistance, double* minDistance) const
{
  /*
  pxr::GfVec3f p0, p1, p2;
  pxr::GfVec3d baryCoords;
  double distance;

  bool frontFacing;
  Triangle* tri;
  for(unsigned t = 0;t<_elements.size();t++) {
    switch(_elements[t].)
    tri = (Triangle*)_elements[t].ptr;
    p0 = mesh->GetPosition(tri, 0);
    p1 = mesh->GetPosition(tri, 1);
    p2 = mesh->GetPosition(tri, 2);
    frontFacing = false;

    if(ray.Intersect(p0, p1, p2, &distance, &baryCoords, &frontFacing, 
      maxDistance)) {
      if(distance < *minDistance) {
        *minDistance = distance;
        hit->SetGeometry(geom);
        hit->SetElementIndex(tri->id);
        
        hit->id = tri->id;
        hit-> = pxr::GfVec3f(baryCoords);
        hit = true;

      }
    }
  }
  return hit;
  */
    return false;
}

/*
    switch(_elementType) {
    case POINT:
      break;
    case EDGE:
      break;
    case TRIANGLE:
    {
      break;
    }
    case POLYGON:
      break;
*/


uint32_t Grid2DIntersector::SLICE_INDICES[27*3] = {    
  // X_SLICE
  0,3,6,9,12,15,18,21,24,
  1,4,7,10,13,16,19,22,25,
  2,5,8,11,14,17,20,23,26,
  
  // Y_SLICE
  0,1,2,9,10,11,18,19,20,
  3,4,5,12,13,14,21,22,23,
  6,7,8,15,16,17,24,25,26,
  
  // Z_SLICE
  0,1,2,3,4,5,6,7,8,
  9,10,11,12,13,14,15,16,17,
  18,19,20,21,22,23,24,25,26 
};

void Grid2DIntersector::Cell::InitNeighborBits(uint32_t& neighborBits)
{
    neighborBits = 0;
    for(int i=0;i<32;++i)neighborBits |= (1<<(i));
}

void Grid2DIntersector::Cell::ClearNeighborBitsSlice(uint32_t& neighborBits,
  short axis, short slice)
{
  uint32_t* indices;
  indices = &Grid2DIntersector::SLICE_INDICES[axis * 27 + slice * 9];
  for(int i=0;i<9;++i){
    neighborBits &= ~(1<<(indices[i]));
  }
}

bool Grid2DIntersector::Cell::CheckNeighborBit(const uint32_t neighborBits,
  uint32_t index)
{
    return neighborBits & (1<<(index));
}

void Grid2DIntersector::GetIndices(uint32_t index, uint32_t& X, uint32_t& Y)
{
    X = index % _resolution[0];
    Y = (index / _resolution[0]);
}

Grid2DIntersector::Cell* Grid2DIntersector::GetCell(uint32_t index)
{
    return _cells[MIN(MAX(index, 0), _numCells-1)];
}

Grid2DIntersector::Cell* Grid2DIntersector::GetCell(uint32_t x, uint32_t y)
{
    uint32_t cx = MIN(MAX(x, 0), _resolution[0]);
    uint32_t cy = MIN(MAX(y, 0), _resolution[1]);
    return _cells[_resolution[0] * cy + cx];
}

Grid2DIntersector::Cell* Grid2DIntersector::GetCell(const pxr::GfVec3f& pos){
  pxr::GfVec3f rescale;
  pxr::GfVec3f invDimensions(1.f/_cellDimension[0],
                             1.f/_cellDimension[1],
                             1.f/_cellDimension[2]);

  // convert to cell coordinates
  const pxr::GfVec3f& bboxMin = _range.GetMin();
  rescale[0] = (pos[0] - bboxMin[0]) * invDimensions[0];
  rescale[0] = (pos[1] - bboxMin[1]) * invDimensions[1];
  rescale[0] = (pos[2] - bboxMin[2]) * invDimensions[2];

  uint32_t idz = CLAMP(floor(rescale[2]), 0, _resolution[2] - 1);
  uint32_t idy = CLAMP(floor(rescale[1]), 0, _resolution[1] - 1);
  uint32_t idx = CLAMP(floor(rescale[0]), 0, _resolution[0] - 1);

  return 
    _cells[_resolution[0] * _resolution[1] * idz + _resolution[0] * idy + idx];
}


void Grid2DIntersector::GetCellIndexAndWeights(const pxr::GfVec3f& pos,
                                    uint32_t& index,
                                    pxr::GfVec3f& weights){
  pxr::GfVec3f rescale;
  pxr::GfVec3f invDimensions(1.f/(_cellDimension[0]),
                             1.f/(_cellDimension[1]),
                             1.f/(_cellDimension[2]));

  // convert to cell coordinates
  const pxr::GfVec3f& bboxMin = _range.GetMin();
  rescale[0] = (pos[0] - bboxMin[0] - 0.5f * _cellDimension[0]) * invDimensions[0];
  rescale[1] = (pos[1] - bboxMin[1] - 0.5f * _cellDimension[1]) * invDimensions[1];
  rescale[2] = (pos[2] - bboxMin[2] - 0.5f * _cellDimension[2]) * invDimensions[2];


  uint32_t idz = CLAMP(floor(rescale[2]), 0, _resolution[2] - 2);
  uint32_t idy = CLAMP(floor(rescale[1]), 0, _resolution[1] - 2);
  uint32_t idx = CLAMP(floor(rescale[0]), 0, _resolution[0] - 2);

  weights[0] = MIN(MAX((((rescale[0] - idx) * 
    _cellDimension[0]) / _cellDimension[0]), 0.f), 1.f);
  weights[1] = MIN(MAX((((rescale[1] - idy) * 
    _cellDimension[1]) / _cellDimension[1]), 0.f), 1.f);
  weights[2] = MIN(MAX((((rescale[2] - idz) * 
    _cellDimension[2]) / _cellDimension[2]), 0.f), 1.f);

  index = _resolution[0]*_resolution[1]*idz + _resolution[0]*idy + idx;
}

void Grid2DIntersector::IndexToXY(const uint32_t index, uint32_t& x, uint32_t& y)
{
    x = index % (GetResolutionX());
    y = (index  / (GetResolutionY()));

    x = MAX(MIN(x, GetResolutionX()), 0);
    y = MAX(MIN(y, GetResolutionY()), 0);
}

void Grid2DIntersector::XYToIndex(const uint32_t x, const uint32_t y, uint32_t& index){
    index = y * GetResolutionX() + x;
}

void Grid2DIntersector::GetNeighbors(uint32_t index, std::vector<uint32_t>& neighbors){
  Grid2DIntersector::Cell* cell = _cells[index];
    uint32_t neighborBits = 0;
    uint32_t x, y, c, n;
    Grid2DIntersector::Cell::InitNeighborBits(neighborBits);
    IndexToXY(index, x, y);
    if(x==0)Grid2DIntersector::Cell::ClearNeighborBitsSlice(neighborBits, 0,0);
    else if(x==_resolution[0]-1)Grid2DIntersector::Cell::ClearNeighborBitsSlice(neighborBits, 0,2);
    if(y==0)Grid2DIntersector::Cell::ClearNeighborBitsSlice(neighborBits, 1,0);
    else if(y==_resolution[1]-1)Grid2DIntersector::Cell::ClearNeighborBitsSlice(neighborBits, 1,2);

    for(y=0;y<3;++y){
        for(x=0;x<3;++x){
            c = y * 3 + x;
            if(c==13)continue;
            if(Grid2DIntersector::Cell::CheckNeighborBit(neighborBits, c)){
                n = index;

                if(y==0){
                    n -= _resolution[0];
                }
                else if(y == 2){
                    n += _resolution[0];
                }

                if(x == 0){
                    n -= 1;
                }
                else if(x == 2){
                    n += 1;    
                }
                    
                neighbors.push_back(n);
            }
        }
    }            
}

void 
Grid2DIntersector::GetNeighbors(uint32_t index, std::vector<Grid2DIntersector::Cell*>& neighbors)
{
  std::vector<uint32_t> indices;
  GetNeighbors(index, indices);
  for(int i=0;i<indices.size();++i){
    if(_cells[indices[i]]){
      neighbors.push_back(_cells[indices[i]]);
    }
  }
}


AMN_NAMESPACE_CLOSE_SCOPE
