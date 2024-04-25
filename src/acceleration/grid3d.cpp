#include "../acceleration/grid3d.h"
#include "../geometry/point.h"
#include "../geometry/edge.h"
#include "../geometry/triangle.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"

JVR_NAMESPACE_OPEN_SCOPE

void Grid3D::Cell::Insert(Geometry* geometry, Point* point)
{
  components[geometry].push_back((Component*)point);
}

void Grid3D::Cell::Insert(Geometry* geometry, Edge* edge)
{
  components[geometry].push_back((Component*)edge);
}

void Grid3D::Cell::Insert(Geometry* geometry, Triangle* triangle)
{
  components[geometry].push_back((Component*)triangle);
}


bool Grid3D::Cell::Raycast(Geometry* geometry, const pxr::GfRay& ray,
  Location* hit, double maxDistance, double* minDistance) const
{
  auto& componentsIt = components.find(geometry);
  if (componentsIt == components.end()) return false;

  const _Components& components = componentsIt->second;
  pxr::GfRay localRay(ray);
  localRay.Transform(geometry->GetInverseMatrix());
  const pxr::GfVec3f* points = ((const Deformable*)geometry)->GetPositionsCPtr();

  bool hitSomething = false;
  Location localHit(*hit);
  for (Component* component : components){
    if (component->Raycast(points, localRay, &localHit)) {
      const pxr::GfVec3f localPoint(localRay.GetPoint(localHit.GetT()));
      const float distance = (ray.GetStartPoint() - geometry->GetMatrix().Transform(localPoint)).GetLength();
      if (distance < *minDistance && distance < maxDistance) {
        hit->Set(localHit);
        hit->SetT(distance);
        *minDistance = distance;
        hitSomething = true;
      }
    }
  }
  return hitSomething;
}

// delete all cells
void Grid3D::DeleteCells()
{
  if(!_cells)return;

  for (uint32_t i = 0; i < _numCells; ++i)
    if (_cells[i] != NULL) delete _cells[i];
  delete [] _cells;
  _cells = NULL;
}

void Grid3D::InsertMesh(Mesh* mesh, size_t geomIdx)
{
  std::cout << "insert mesh : " << geomIdx << " : " << mesh << std::endl;

  size_t numTriangles = mesh->GetNumTriangles();
  const pxr::GfMatrix4d& matrix = mesh->GetMatrix();
  const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();

  pxr::GfVec3f invDimensions(1.f / _cellDimension[0], 1.f / _cellDimension[1], 1.f / _cellDimension[2]);

  const pxr::GfVec3f bboxMin(GetMin());
  const pxr::GfVec3f bboxMax(GetMax());

  // insert all the triangles in the cells
  for(uint32_t t = 0; t < numTriangles; ++t)
  {
    pxr::GfVec3f tmin(FLT_MAX,FLT_MAX,FLT_MAX);
    pxr::GfVec3f tmax(-FLT_MAX,-FLT_MAX,-FLT_MAX);
    Triangle* triangle = mesh->GetTriangle(t);
    pxr::GfVec3f A = matrix.Transform(positions[triangle->vertices[0]]);
    pxr::GfVec3f B = matrix.Transform(positions[triangle->vertices[1]]);
    pxr::GfVec3f C = matrix.Transform(positions[triangle->vertices[2]]);

    for (uint8_t k = 0; k < 3; ++k) {
      if (A[k] < tmin[k]) tmin[k] = A[k];
      if (B[k] < tmin[k]) tmin[k] = B[k];
      if (C[k] < tmin[k]) tmin[k] = C[k];
      if (A[k] > tmax[k]) tmax[k] = A[k];
      if (B[k] > tmax[k]) tmax[k] = B[k];
      if (C[k] > tmax[k]) tmax[k] = C[k];
    }
    
    // convert to cell coordinates
    tmin[0] = (tmin[0] - bboxMin[0]) * invDimensions[0];
    tmin[1] = (tmin[1] - bboxMin[1]) * invDimensions[1];
    tmin[2] = (tmin[2] - bboxMin[2]) * invDimensions[2];
    
    tmax[0] = (tmax[0] - bboxMin[0]) * invDimensions[0];
    tmax[1] = (tmax[1] - bboxMin[1]) * invDimensions[1];
    tmax[2] = (tmax[2] - bboxMin[2]) * invDimensions[2];
    
    uint32_t zmin = CLAMP(floor(tmin[2]), 0, _resolution[2] - 1);
    uint32_t zmax = CLAMP(floor(tmax[2]), 0, _resolution[2] - 1);
    uint32_t ymin = CLAMP(floor(tmin[1]), 0, _resolution[1] - 1);
    uint32_t ymax = CLAMP(floor(tmax[1]), 0, _resolution[1] - 1);
    uint32_t xmin = CLAMP(floor(tmin[0]), 0, _resolution[0] - 1);
    uint32_t xmax = CLAMP(floor(tmax[0]), 0, _resolution[0] - 1);
    
    // loop over all the cells the triangle overlaps and insert
    for (uint32_t z = zmin; z <= zmax; ++z)
      for (uint32_t y = ymin; y <= ymax; ++y)
        for (uint32_t x = xmin; x <= xmax; ++x) {
          uint32_t o = z * _resolution[0] * _resolution[1] + y * _resolution[0] + x;
          if (_cells[o] == NULL) _cells[o] = new Cell(o);
          _cells[o]->Insert((Geometry*)mesh, triangle);
        }
  }
}

void Grid3D::InsertCurve(Curve* curve, size_t idx)
{
  //Curve* curve = (Curve*)_geometries[idx];
} 

void Grid3D::InsertPoints(Points* points, size_t idx)
{
  //Points* points = (Points*)_geometries[idx];
}

size_t _GetGeometryNumComponents(Geometry* geometry)
{
  switch(geometry->GetType()) {
    case Geometry::POINT:
      return ((Points*)geometry)->GetNumPoints();
    case Geometry::CURVE:
      return ((Curve*)geometry)->GetTotalNumSegments();
    case Geometry::MESH:
      return ((Mesh*)geometry)->GetNumTriangles();
  }
  return 0;
}

// construct the grid from a list of geometries
void Grid3D::Init(const std::vector<Geometry*>& geometries)
{
  std::cout << "init grid 3d with " << geometries.size() << " geometries" << std::endl;
  _geometries = geometries;
  // delete old cells
  DeleteCells();

  if (!_geometries.size())return;

  // compute bound of the scene
  size_t totalNumComponents = _GetGeometryNumComponents(_geometries[0]);
  const pxr::GfBBox3d bbox = _geometries[0]->GetBoundingBox(true);
  pxr::GfRange3d accum = bbox.GetRange();
  for (size_t i = 1; i < _geometries.size(); ++i) {
    totalNumComponents += _GetGeometryNumComponents(_geometries[i]);
    const pxr::GfBBox3d bbox = _geometries[i]->GetBoundingBox(true);
    accum.UnionWith(bbox.GetRange());
  }
  SetMin(accum.GetMin());
  SetMax(accum.GetMax());

  std::cout << "bbox " << *this << std::endl;

  // create the grid
  pxr::GfVec3f size(GetMax() - GetMin());

  //float desiredCellSize = area*12;
  float cubeRoot = 
    powf(1 * totalNumComponents / (size[0] * size[1] * size[2]), 1 / 3.f);

  for (uint8_t i = 0; i < 3; ++i) {
    _resolution[i] = floor(size[i] * cubeRoot);
    _resolution[i] = 
      MAX(uint32_t(1), MIN(_resolution[i], uint32_t(128)));
  }

  _cellDimension[0] = size[0] /(float)_resolution[0];
  _cellDimension[1] = size[1] /(float)_resolution[1];
  _cellDimension[2] = size[2] /(float)_resolution[2];

  // allocate memory
  _numCells = _resolution[0] * _resolution[1] * _resolution[2];
  _cells = new Grid3D::Cell* [_numCells];

  // set all pointers to NULL
  memset(_cells, 0x0, sizeof(Grid3D::Cell*) * _numCells);

  // insert all the triangles in the cells
  for (size_t geomIdx = 0; geomIdx < geometries.size(); ++ geomIdx) {
    switch (geometries[geomIdx]->GetType()) {
      case Geometry::MESH:
        InsertMesh((Mesh*)geometries[geomIdx], geomIdx);
        break;
      case Geometry::CURVE:
        InsertCurve((Curve*)geometries[geomIdx], geomIdx);
        break;
      case Geometry::POINT:
        InsertPoints((Points*)geometries[geomIdx], geomIdx);
        break;
    }
  }
}

void Grid3D::Update()
{
  Init(_geometries);
}

bool Grid3D::Raycast(const pxr::GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{
  double bmin, bmax;
  // if the ray doesn't intersect the grid return
  if(!ray.Intersect(pxr::GfBBox3d(*this), &bmin, &bmax))
  {
    return false;
  }

  // initialization step
  int32_t exit[3], step[3], cell[3];
  pxr::GfVec3d deltaT, nextCrossingT;
  pxr::GfVec3d invdir = -1.0 * ray.GetDirection();

  for (uint8_t i = 0; i < 3; ++i) {
    // convert ray starting point to cell coordinates
    double rayOrigCell = 
      ((ray.GetPoint(0.f)[i] + ray.GetDirection()[i] * bmin) - GetMin()[i]);
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
  uint32_t o = cell[2] * _resolution[0] * _resolution[1] + cell[1] * _resolution[0] + cell[0];
  bool hitSomething = false;
  // walk through each cell of the grid and test for an intersection if
  // current cell contains geometry
  while(true) {
    uint32_t o = cell[2] * _resolution[0] * _resolution[1] + cell[1] * _resolution[0] + cell[0];
    if (_cells[o] != NULL) 
      for(size_t geomIdx = 0; geomIdx < _geometries.size(); ++geomIdx)
        if(_cells[o]->Raycast(_geometries[geomIdx], ray, hit, maxDistance, minDistance))
          {hit->SetGeometryIndex(geomIdx); hitSomething=true;}
        
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
  return hitSomething;
}

bool Grid3D::Closest(const pxr::GfVec3f& point, Location* hit,
  double maxDistance) const
{
  return false;
}

pxr::GfVec3f Grid3D::GetCellPosition(uint32_t index) {
  pxr::GfVec3f position;
  const pxr::GfVec3d& bboxMin = GetMin();
  position[0] = bboxMin[0] + 
    _cellDimension[0] * (index % _resolution[0]);
  position[1] = bboxMin[1] + 
    _cellDimension[1] * ((index / _resolution[0]) % _resolution[1]);
  position[2] = bboxMin[2] + 
    _cellDimension[2] * (index / (_resolution[0] * _resolution[1]));
  return position;
}

pxr::GfVec3f Grid3D::GetCellMin(uint32_t index){
  return GetCellPosition(index) - _cellDimension * 0.5f;
}

pxr::GfVec3f Grid3D::GetCellMax(uint32_t index){
  return GetCellPosition(index) + _cellDimension * 0.5f;
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
    return _cells[MIN(MAX(index, 0), _numCells-1)];
}

Grid3D::Cell* Grid3D::GetCell(uint32_t x, uint32_t y, uint32_t z)
{
    uint32_t cx = MIN(MAX(x, 0), _resolution[0]);
    uint32_t cy = MIN(MAX(y, 0), _resolution[1]);
    uint32_t cz = MIN(MAX(z, 0), _resolution[2]);
    return 
      _cells[_resolution[0] * _resolution[1] * cz + _resolution[0] * cy + cx];
}

Grid3D::Cell* Grid3D::GetCell(const pxr::GfVec3f& pos){
  pxr::GfVec3f rescale;
  pxr::GfVec3f invDimensions(1.f/_cellDimension[0],
                             1.f/_cellDimension[1],
                             1.f/_cellDimension[2]);

  // convert to cell coordinates
  const pxr::GfVec3d& bboxMin = GetMin();
  rescale[0] = (pos[0] - bboxMin[0]) * invDimensions[0];
  rescale[0] = (pos[1] - bboxMin[1]) * invDimensions[1];
  rescale[0] = (pos[2] - bboxMin[2]) * invDimensions[2];

  uint32_t idz = CLAMP(floor(rescale[2]), 0, _resolution[2] - 1);
  uint32_t idy = CLAMP(floor(rescale[1]), 0, _resolution[1] - 1);
  uint32_t idx = CLAMP(floor(rescale[0]), 0, _resolution[0] - 1);

  return 
    _cells[_resolution[0] * _resolution[1] * idz + _resolution[0] * idy + idx];
}


pxr::GfVec3f Grid3D::GetColorXYZ(const uint32_t index)
{
  pxr::GfVec3f rescale;
  pxr::GfVec3f invDimensions(1.f/(_cellDimension[0]),
                             1.f/(_cellDimension[1]),
                             1.f/(_cellDimension[2]));

  // convert to cell coordinates
  uint32_t x, y, z;
  IndexToXYZ(index, x, y, z);

  pxr::GfVec3f color;
  color[0] = static_cast<float>(x) / _resolution[0];
  color[1] = static_cast<float>(y) / _resolution[1];
  color[2] = static_cast<float>(z) / _resolution[2];

  return color;

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

void Grid3D::GetNeighbors(uint32_t index, std::vector<uint32_t>& neighbors) {
  Grid3D::Cell* cell = _cells[index];
  uint32_t neighborBits = 0;
  uint32_t x, y, z, c, n;
  Grid3D::Cell::InitNeighborBits(neighborBits);
  IndexToXYZ(index, x, y, z);
  if (x == 0)Grid3D::Cell::ClearNeighborBitsSlice(neighborBits, 0, 0);
  else if (x == _resolution[0] - 1)Grid3D::Cell::ClearNeighborBitsSlice(neighborBits, 0, 2);
  if (y == 0)Grid3D::Cell::ClearNeighborBitsSlice(neighborBits, 1, 0);
  else if (y == _resolution[1] - 1)Grid3D::Cell::ClearNeighborBitsSlice(neighborBits, 1, 2);
  if (z == 0)Grid3D::Cell::ClearNeighborBitsSlice(neighborBits, 2, 0);
  else if (z == _resolution[2] - 1)Grid3D::Cell::ClearNeighborBitsSlice(neighborBits, 2, 2);

  for (z = 0; z < 3; ++z)
    for (y = 0; y < 3; ++y)
      for (x = 0; x < 3; ++x) {
        c = z * 9 + y * 3 + x;
        if (c == 13)continue;
        if (Grid3D::Cell::CheckNeighborBit(neighborBits, c)) {
          n = index;
          if (z == 0)
            n -= (_resolution[0] * _resolution[1]);

          else if (z == 2)
            n += (_resolution[0] * _resolution[1]);

          if (y == 0)
            n -= _resolution[0];

          else if (y == 2)
            n += _resolution[0];

          if (x == 0)
            n -= 1;

          else if (x == 2)
            n += 1;

          neighbors.push_back(n);
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

void Grid3D::GetCells(pxr::VtArray<pxr::GfVec3f>& positions, 
  pxr::VtArray<pxr::GfVec3f>& scales, pxr::VtArray<pxr::GfVec3f>& colors)
{
  for(size_t c = 0; c < _numCells; ++c) {
    if(_cells[c]) {
      const pxr::GfVec3f scale(GetCellMax(c) - GetCellMin(c));
      positions.push_back(GetCellPosition(c)+scale*0.5f);
      scales.push_back(scale);
      colors.push_back(GetColorXYZ(c));
    }
  }
}


JVR_NAMESPACE_CLOSE_SCOPE
