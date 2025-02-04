#include "../acceleration/grid3d.h"
#include "../geometry/point.h"
#include "../geometry/edge.h"
#include "../geometry/triangle.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"

JVR_NAMESPACE_OPEN_SCOPE

void Grid3D::Cell::Insert(Point* point)
{
  components.push_back((Component*)point);
}

void Grid3D::Cell::Insert(Edge* edge)
{
  components.push_back((Component*)edge);
}

void Grid3D::Cell::Insert(Triangle* triangle)
{
  components.push_back((Component*)triangle);
}

bool Grid3D::Cell::Raycast(const Geometry* geometry, const GfRay& ray,
  Location* hit, double maxDistance, double* minDistance) const
{
  bool hitSomething = false;

  GfRay localRay(ray);
  localRay.Transform(geometry->GetInverseMatrix());
  const GfVec3f* points = ((const Deformable*)geometry)->GetPositionsCPtr();

  Location localHit(*hit);
  for (Component* component : components) {
    if (component->Raycast(points, localRay, &localHit)) {
      const GfVec3f localPoint(localRay.GetPoint(localHit.GetDistance()));
      const float distance = (ray.GetStartPoint() - geometry->GetMatrix().Transform(localPoint)).GetLength();
      if (distance < *minDistance && distance < maxDistance) {
        hit->Set(localHit);
        hit->SetDistance(distance);
        hit->SetGeometryIndex(0);
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

void Grid3D::InsertMesh(Mesh* mesh)
{
  size_t numTriangles = mesh->GetNumTriangles();
  const GfMatrix4d& matrix = mesh->GetMatrix();
  const GfVec3f* positions = mesh->GetPositionsCPtr();

  GfVec3f invDimensions(1.f / _cellDimension[0], 1.f / _cellDimension[1], 1.f / _cellDimension[2]);

  const GfVec3f bboxMin(GetMin());
  const GfVec3f bboxMax(GetMax());

  // insert all the triangles in the cells
  for(size_t t = 0; t < numTriangles; ++t)
  {
    GfVec3f tmin(FLT_MAX,FLT_MAX,FLT_MAX);
    GfVec3f tmax(-FLT_MAX,-FLT_MAX,-FLT_MAX);
    Triangle* triangle = mesh->GetTriangle(t);
    GfVec3f A(matrix.Transform(positions[triangle->vertices[0]]));
    GfVec3f B(matrix.Transform(positions[triangle->vertices[1]]));
    GfVec3f C(matrix.Transform(positions[triangle->vertices[2]]));

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
            _cells[o]->Insert(triangle);
        }
  }
}

void Grid3D::InsertCurve(Curve* curve)
{
  //Curve* curve = (Curve*)_geometries[idx];
} 

void Grid3D::InsertPoints(Points* points)
{
  //Points* points = (Points*)_geometries[idx];
}

size_t _GetGeometryNumComponents(const Geometry* geometry)
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
  _Init(geometries);

  Update();

  
}

void Grid3D::Update()
{
  // delete old cells
  DeleteCells();

  if (!GetNumGeometries())return;

  // compute bound of the grid
  size_t totalNumComponents = _GetGeometryNumComponents(GetGeometry(0));
  const GfBBox3d bbox = GetGeometry(0)->GetBoundingBox(true);
  GfRange3d accum = bbox.GetRange();
  for (size_t i = 1; i < GetNumGeometries(); ++i) {
    totalNumComponents += _GetGeometryNumComponents(GetGeometry(i));
    const GfBBox3d bbox = GetGeometry(i)->GetBoundingBox(true);
    accum.UnionWith(bbox.GetRange());
  }
  SetMin(accum.GetMin());
  SetMax(accum.GetMax());

  // create the grid
  GfVec3f size(GetMax() - GetMin());

  //float desiredCellSize = area*12;
  float volume = size[0] * size[1] * size[2];

  float cubeRoot = sqrt((12*totalNumComponents) / volume);

  for (uint8_t i = 0; i < 3; ++i) {
    _resolution[i] = floor(GfPow(size[i], 1/3) * cubeRoot);
    _resolution[i] = MAX(uint32_t(1), MIN(_resolution[i], uint32_t(128)));
  }

  _cellDimension[0] = size[0] /(float)_resolution[0];
  _cellDimension[1] = size[1] /(float)_resolution[1];
  _cellDimension[2] = size[2] /(float)_resolution[2];

  // allocate memory
  _numCells = _resolution[0] * _resolution[1] * _resolution[2];
  _cells = new Grid3D::Cell* [_numCells];

  // set all pointers to NULL
  memset(_cells, 0x0, sizeof(Grid3D::Cell*) * _numCells);

  // insert all the components in the cells
  switch (GetGeometry(0)->GetType()) {
    case Geometry::MESH:
      InsertMesh((Mesh*)GetGeometry(0));
      break;
    case Geometry::CURVE:
      InsertCurve((Curve*)GetGeometry(0));
      break;
    case Geometry::POINT:
      InsertPoints((Points*)GetGeometry(0));
      break;
  }
}

bool Grid3D::Raycast(const GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{
  double enterDistance, exitDistance;
  // if the ray doesn't intersect the grid return
  if(!ray.Intersect(GfBBox3d(*this), &enterDistance, &exitDistance))
  {
    return false;
  }

  // initialization step
  int32_t exit[3], step[3], cell[3];
  GfVec3d deltaT, nextCrossingT;
  GfVec3d invdir = -1.0 * ray.GetDirection();

  for (uint8_t i = 0; i < 3; ++i) {
    // convert ray starting point to cell coordinates
    double rayOrigCell = 
      ((ray.GetPoint(0.f)[i] + ray.GetDirection()[i] * enterDistance) - GetMin()[i]);
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
      nextCrossingT[i] = enterDistance + (cell[i] * _cellDimension[i] - rayOrigCell) * invdir[i];
      exit[i] = -1;
      step[i] = -1;
    }
    else {
      deltaT[i] = _cellDimension[i] * invdir[i];
      nextCrossingT[i] = enterDistance + ((cell[i] + 1)  * _cellDimension[i] - rayOrigCell) * invdir[i];
      exit[i] = _resolution[i];
      step[i] = 1;
    }
  }

  // start cell
  uint32_t o = cell[2] * _resolution[0] * _resolution[1] + cell[1] * _resolution[0] + cell[0];
  bool hitSomething = false;
  // walk through each cell of the grid and test for an intersection if
  // current cell contains geometry
  while(!hitSomething) {
    uint32_t o = cell[2] * _resolution[0] * _resolution[1] + cell[1] * _resolution[0] + cell[0];
    if (_cells[o] != NULL) 
      if(_cells[o]->Raycast(GetGeometry(0), ray, hit, maxDistance, minDistance))
        hitSomething=true;
        
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

bool Grid3D::Closest(const GfVec3f& point, Location* hit, double maxDistance) const
{
  return false;
}

GfVec3f Grid3D::GetCellPosition(uint32_t index) {
  GfVec3f position;
  const GfVec3d& bboxMin = GetMin();
  position[0] = bboxMin[0] + 
    _cellDimension[0] * (index % _resolution[0]);
  position[1] = bboxMin[1] + 
    _cellDimension[1] * ((index / _resolution[0]) % _resolution[1]);
  position[2] = bboxMin[2] + 
    _cellDimension[2] * (index / (_resolution[0] * _resolution[1]));
  return position;
}

GfVec3f Grid3D::GetCellMin(uint32_t index){
  return GetCellPosition(index) - _cellDimension * 0.5f;
}

GfVec3f Grid3D::GetCellMax(uint32_t index){
  return GetCellPosition(index) + _cellDimension * 0.5f;
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

Grid3D::Cell* Grid3D::GetCell(const GfVec3f& pos){
  GfVec3f rescale;
  GfVec3f invDimensions(1.f/_cellDimension[0],
                             1.f/_cellDimension[1],
                             1.f/_cellDimension[2]);

  // convert to cell coordinates
  const GfVec3d& bboxMin = GetMin();
  rescale[0] = (pos[0] - bboxMin[0]) * invDimensions[0];
  rescale[0] = (pos[1] - bboxMin[1]) * invDimensions[1];
  rescale[0] = (pos[2] - bboxMin[2]) * invDimensions[2];

  uint32_t idz = CLAMP(floor(rescale[2]), 0, _resolution[2] - 1);
  uint32_t idy = CLAMP(floor(rescale[1]), 0, _resolution[1] - 1);
  uint32_t idx = CLAMP(floor(rescale[0]), 0, _resolution[0] - 1);

  return 
    _cells[_resolution[0] * _resolution[1] * idz + _resolution[0] * idy + idx];
}


GfVec3f Grid3D::GetColorXYZ(const uint32_t index)
{
  GfVec3f rescale;
  GfVec3f invDimensions(1.f/(_cellDimension[0]),
                             1.f/(_cellDimension[1]),
                             1.f/(_cellDimension[2]));

  // convert to cell coordinates
  uint32_t x, y, z;
  IndexToXYZ(index, x, y, z);

  GfVec3f color;
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

void Grid3D::GetCells(VtArray<GfVec3f>& positions, 
  VtArray<GfVec3f>& scales, VtArray<GfVec3f>& colors, bool branchOrLeaf)
{
  for(size_t c = 0; c < _numCells; ++c) {
    if(_cells[c]) {
      const GfVec3f scale(_cellDimension);
      positions.push_back(GetCellPosition(c)+scale*0.5);
      scales.push_back(scale);
      colors.push_back(GetColorXYZ(c));
    }
  }
}


JVR_NAMESPACE_CLOSE_SCOPE
