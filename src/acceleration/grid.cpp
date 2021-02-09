#include "grid.h"
#include "utils.h"

// delete all cells
void Grid3D::DeleteCells()
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
void Grid3D::ResetCells()
{
  if(_cells)
  {
    for (uint32_t i = 0; i < _numCells; ++i)
      if (_cells[i] != NULL) _cells[i]->_hit = false;
  }
}

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

// place a triangle mesh in the grid
void Grid3D::PlaceIntoGrid(Mesh* mesh)
{
  // delete old cells
  DeleteCells();
  if(!mesh) {
    _mesh = NULL;
    return;
  }

  _mesh = mesh;

  // compute bound of the scene
  uint32_t totalNumTriangles = _mesh->_numTriangles;
  //AABB& bbox = _mesh->_bbox;
  _bbox.compute(_mesh);
  // create the grid
  pxr::GfVec3f size = _bbox.max() - _bbox.min();

  //float desiredCellSize = area*12;
  float cubeRoot = 
    powf(1 * totalNumTriangles / (size[0] * size[1] * size[2]), 1 / 3.f);
  for (uint8_t i = 0; i < 3; ++i) {
    _resolution[i] = floor(size[i] * cubeRoot);
    _resolution[i] = 
      MAX(uint32_t(1), MIN(_resolution[i], uint32_t(128)));
  }

  _cellDimension.x = size.x /(float)_resolution[0];
  _cellDimension.y = size.y /(float)_resolution[1];
  _cellDimension.z = size.z /(float)_resolution[2];

  // allocate memory
  _numCells = _resolution[0] * _resolution[1] * _resolution[2];
  _cells = new Grid3D::Cell* [_numCells];

  // set all pointers to NULL
  memset(_cells, 0x0, sizeof(Grid3D::Cell*) * _numCells);

  pxr::GfVec3f A, B, C;
  Triangle* T;
  Mesh* mesh = _mesh->_mesh;
  unsigned offset = 0;

  pxr::GfVec3f invDimensions(1/_cellDimension[0],
    1/_cellDimension[1], 1/_cellDimension[2]);

  // insert all the triangles in the cells
  for(uint32_t i=0;i<totalNumTriangles;i++)
  {
    pxr::GfVec3f tmin(FLT_MAX,FLT_MAX,FLT_MAX);
    pxr::GfVec3f tmax(-FLT_MAX,-FLT_MAX,-FLT_MAX);
    T = mesh->GetTriangle(i+offset);
    A = mesh->GetPosition(T,0);
    B = mesh->GetPosition(T,1);
    C = mesh->GetPosition(T,2);

    for (uint8_t k = 0; k < 3; ++k) {
      if (A[k] < tmin[k]) tmin[k] = A[k];
      if (B[k] < tmin[k]) tmin[k] = B[k];
      if (C[k] < tmin[k]) tmin[k] = C[k];
      if (A[k] > tmax[k]) tmax[k] = A[k];
      if (B[k] > tmax[k]) tmax[k] = B[k];
      if (C[k] > tmax[k]) tmax[k] = C[k];
    }

    // convert to cell coordinates
    tmin.x = (tmin.x - _bbox.min().x) * invDimensions.x;
    tmin.y = (tmin.y - _bbox.min().y) * invDimensions.y;
    tmin.z = (tmin.z - _bbox.min().z) * invDimensions.z;

    tmax.x = (tmax.x - _bbox.min().x) * invDimensions.x;
    tmax.y = (tmax.y - _bbox.min().y) * invDimensions.y;
    tmax.z = (tmax.z - _bbox.min().z) * invDimensions.z;

    uint32_t zmin = CLAMP(floor(tmin[2]), 0, _resolution[2] - 1);
    uint32_t zmax = CLAMP(floor(tmax[2]), 0, _resolution[2] - 1);
    uint32_t ymin = CLAMP(floor(tmin[1]), 0, _resolution[1] - 1);
    uint32_t ymax = CLAMP(floor(tmax[1]), 0, _resolution[1] - 1);
    uint32_t xmin = CLAMP(floor(tmin[0]), 0, _resolution[0] - 1);
    uint32_t xmax = CLAMP(floor(tmax[0]), 0, _resolution[0] - 1);

    // loop over all the cells the triangle overlaps and insert
    for (uint32_t z = zmin; z <= zmax; ++z)
    {
      for (uint32_t y = ymin; y <= ymax; ++y)
      {
        for (uint32_t x = xmin; x <= xmax; ++x)
        {
            uint32_t o = 
              z * _resolution[0] * _resolution[1] + y * _resolution[0] + x;
            if (_cells[o] == NULL) _cells[o] = new Cell(o);
            _cells[o]->insert(T);
            _cells[o]->_color = pxr::GfVec3f(0.5f);
        }
      }
    }
  }
}

bool Grid3D::intersect(const pxr::GfRay& ray, double maxDistance, 
  PointOnMesh* hitPoint) const
{
    double bmin, bmax;
    // if the ray doesn't intersect the grid return
    if(!ray.intersectBox(_mesh->_bbox, &bmin, &bmax))
    {
        return false;
    }

    // initialization step
    int32_t exit[3], step[3], cell[3];
    MVector deltaT, nextCrossingT;
    MVector invdir = ray.getInvDirection();

    for (uint8_t i = 0; i < 3; ++i) {
        // convert ray starting point to cell coordinates
        double rayOrigCell = ((ray.getOrigin()[i] + ray.getDirection()[i] * bmin) -  _mesh->_bbox.min()[i]);
        cell[i] = CLAMP(floor(rayOrigCell / _cellDimension[i]), 0, _resolution[i] - 1);
        if(fabs(ray.getDirection()[i]) < 0.0000001)
        {
            deltaT[i] = 0;
            nextCrossingT[i] = maxDistance;
            exit[i] = cell[i];
            step[i] = 0;
        }
        else if (ray.getDirection()[i] < 0.0) {
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
    uint32_t o = cell[2] * _resolution[0] * _resolution[1] + cell[1] * _resolution[0] + cell[0];
    if(_cells[o] != NULL) _cells[o]->_hit = true;

    // walk through each cell of the grid and test for an intersection if
    // current cell contains geometry
    Mesh* mesh = _mesh->_mesh;
    double minDistance = DBL_MAX;
    bool hit = false;
    while(1) {
        uint32_t o = cell[2] * _resolution[0] * _resolution[1] + cell[1] * _resolution[0] + cell[0];
        if (_cells[o] != NULL)
        {
            if (_cells[o]->intersect(mesh, ray, MP, maxDistance, &minDistance))
            {
                _cells[o]->_hit = true;
                hit = true;
            }
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

MVector Grid3D::GetCellPosition(uint32_t index){
  pxr::GfVec3f position;
  position.x = _bbox.min().x + 
    _cellDimension.x * (index % _resolution[0]);
  position.y = _bbox.min().y + 
    _cellDimension.y * ((index / _resolution[0]) % _resolution[1]);
  position.z = _bbox.min().z + 
    _cellDimension.z * (index / (_resolution[0] * _resolution[1]));
  return position;
}

MVector Grid3D::GetCellMin(uint32_t index){
  return GetCellPosition(index) - _cellDimension * 0.5f;
}

MVector Grid3D::GetCellMax(uint32_t index){
  return GetCellPosition(index) + _cellDimension * 0.5f;
}

const bool Grid3D::Cell::Intersect(Mesh* mesh, const pxr::GfRay& ray, 
  PointOnMesh* hitPoint, double maxDistance, double* minDistance) const
{
  pxr::GfVec3f A, B, C;
  pxr::GfVec3f baryCoords;
  double distance;

  bool frontFacing;
  bool hit = false;
  Triangle* T;
  for(unsigned t = 0;t<_triangles.size();t++)
  {
    T = _triangles[t];
    A = mesh->getPosition(T, 0);
    B = mesh->getPosition(T, 1);
    C = mesh->getPosition(T, 2);
    frontFacing = false;
    if(ray.intersectTriangle(A, B, C, &distance, &baryCoords, &frontFacing, maxDistance))
    {
      if(distance < *minDistance)
      {
        *minDistance = distance;
        hitPoint->_triangleID = T->_ID;
        hitPoint->_baryCoords = baryCoords;
        hit = true;
      }
    }
  }
  return hit;
}


uint32_t Grid3D::SLICE_INDICES[27*3] = {    
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

void Grid3D::Cell::InitNeighborBits(uint32_t& neighborBits)
{
    neighborBits = 0;
    for(int i=0;i<32;++i)neighborBits |= (1<<(i));
}

void Grid3D::Cell::ClearNeighborBitsSlice(uint32_t& neighborBits, 
  short axis, short slice)
{
  uint32_t* indices;
  indices = &Grid3D::SLICE_INDICES[axis * 27 + slice * 9];
  for(int i=0;i<9;++i){
    neighborBits &= ~(1<<(indices[i]));
  }
}

bool Grid3D::Cell::CheckNeighborBit(const uint32_t neighborBits, 
  uint32_t index)
{
    return neighborBits & (1<<(index));
}

void Grid3D::GetIndices(uint32_t index, uint32_t& X, uint32_t& Y, uint32_t& Z)
{
    X = index % _resolution[0];
    Y = (index / _resolution[0]) % _resolution[1];
    Z = index / (_resolution[0] * _resolution[1]);
}

Grid3D::Cell* Grid3D::GetCell(uint32_t index)
{
    return _cells[pxr::MIN(pxr::MAX(index, 0), _numCells-1)];
}

Grid3D::Cell* Grid3D::GetCell(uint32_t x, uint32_t y, uint32_t z) 
{
    uint32_t cx = MIN(MAX(x, 0), _resolution[0]);
    uint32_t cy = MIN(MAX(y, 0), _resolution[1]);
    uint32_t cz = MIN(MAX(z, 0), _resolution[2]);
    return 
      _cells[_resolution[0] * _resolution[1] * cz + _resolution[0] * cy + cx];
}

Grid3D::Cell* Grid3D::GetCell(const pxr::GfVec3f& P){
  pxr::GfVec3f R;
  pxr::GfVec3f invDimensions(1.f/_cellDimension[0],
                             1.f/_cellDimension[1],
                             1.f/_cellDimension[2]);

  // convert to cell coordinates
  R.x = (P.x - _bbox.min().x) * invDimensions.x;
  R.y = (P.y - _bbox.min().y) * invDimensions.y;
  R.z = (P.z - _bbox.min().z) * invDimensions.z;

  uint32_t idz = CLAMP(floor(R[2]), 0, _resolution[2] - 1);
  uint32_t idy = CLAMP(floor(R[1]), 0, _resolution[1] - 1);
  uint32_t idx = CLAMP(floor(R[0]), 0, _resolution[0] - 1);

  return 
    _cells[_resolution[0] * _resolution[1] * idz + _resolution[0] * idy + idx];
}


void Grid3D::GetCellIndexAndWeights(const pxr::GfVec3f& P,
                                    uint32_t& index,
                                    pxr::GfVec3f& weights){
  pxr::GfVec3f R;
  pxr::GfVec3f invDimensions(1.f/(_cellDimension[0]),
                             1.f/(_cellDimension[1]),
                             1.f/(_cellDimension[2]));

  // convert to cell coordinates
  R.x = (P.x - _bbox.min().x - 0.5f * _cellDimension[0]) * invDimensions.x;
  R.y = (P.y - _bbox.min().y - 0.5f * _cellDimension[1]) * invDimensions.y;
  R.z = (P.z - _bbox.min().z - 0.5f * _cellDimension[2]) * invDimensions.z;


  uint32_t idz = CLAMP(floor(R[2]), 0, _resolution[2] - 2);
  uint32_t idy = CLAMP(floor(R[1]), 0, _resolution[1] - 2);
  uint32_t idx = CLAMP(floor(R[0]), 0, _resolution[0] - 2);

  weights.x = MIN(MAX((((R.x - idx) * 
    _cellDimension[0]) / _cellDimension[0]), 0.f), 1.f);
  weights.y = MIN(MAX((((R.y - idy) * 
    _cellDimension[1]) / _cellDimension[1]), 0.f), 1.f);
  weights.z = MIN(MAX((((R.z - idz) * 
    _cellDimension[2]) / _cellDimension[2]), 0.f), 1.f);

  index = _resolution[0]*_resolution[1]*idz + _resolution[0]*idy + idx;
}

void Grid3D::IndexToXYZ(const uint32_t index, uint32_t& x, uint32_t& y, uint32_t& z)
{
    x = index % (GetResolutionX());
    y = (index  / (GetResolutionY())) % (GetResolutionX());
    z = index / ((GetResolutionX()) * (GetResolutionY()));

    x = MAX(MIN(x, GetResolutionX()), 0);
    y = MAX(MIN(y, GetResolutionY()), 0);
    z = MAX(MIN(z, GetResolutionZ()), 0);
}

void Grid3D::XYZToIndex(const uint32_t x, const uint32_t y, const uint32_t z, uint32_t& index){
    index = z * (GetResolutionX() * GetResolutionY()) + y * GetResolutionX() + x;
}

void Grid3D::GetNeighbors(uint32_t index, std::vector<uint32_t>& neighbors){
    Grid3D::Cell* cell = _cells[index];
    uint32_t neighborBits = 0;
    uint32_t x, y, z, c, n;
    Grid3D::Cell::InitNeighborBits(neighborBits);
    IndexToXYZ(index, x, y, z);
    if(x==0)Grid3D::Cell::ClearNeighborBitsSlice(neighborBits, 0,0);
    else if(x==_resolution[0]-1)Grid3D::Cell::ClearNeighborBitsSlice(neighborBits, 0,2);
    if(y==0)Grid3D::Cell::ClearNeighborBitsSlice(neighborBits, 1,0);
    else if(y==_resolution[1]-1)Grid3D::Cell::ClearNeighborBitsSlice(neighborBits, 1,2);
    if(z==0)Grid3D::Cell::ClearNeighborBitsSlice(neighborBits, 2,0);
    else if(z==_resolution[2]-1)Grid3D::Cell::ClearNeighborBitsSlice(neighborBits, 2,2);

    for(z=0;z<3;++z){
        for(y=0;y<3;++y){
            for(x=0;x<3;++x){
                c = z * 9 + y * 3 + x;
                if(c==13)continue;
                if(Grid3D::Cell::CheckNeighborBit(neighborBits, c)){
                    n = index;
                    if(z == 0){
                        n -= (_resolution[0] * _resolution[1]);
                    }
                    else if(z == 2){
                        n += (_resolution[0] * _resolution[1]);
                    }

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
}

void 
Grid3D::GetNeighbors(uint32_t index, std::vector<Grid3D::Cell*>& neighbors) 
{
  std::vector<uint32_t> indices;
  GetNeighbors(index, indices);
  for(int i=0;i<indices.size();++i){
    if(_cells[indices[i]]){
      neighbors.push_back(_cells[indices[i]]);
    }
  }
}
