// Mesh
//----------------------------------------------
#include "mesh.h"
#include <pxr/base/gf/ray.h>

AMN_NAMESPACE_OPEN_SCOPE

Mesh::~Mesh()
{
};

Mesh::Mesh()
{
  _initialized = false;
  _numTriangles = 0;
  _numVertices = 0;
}

Mesh::Mesh(const Mesh* other, bool normalize)
{
  _initialized = true;
  _numTriangles = other->_numTriangles;
  _numVertices = other->_numVertices;
  _numSamples = other->_numSamples;
  _numFaces = other->_numFaces;

  _positions = other->_positions;
  _normals = other->_positions;

  _triangles.resize(_numTriangles);
  memcpy(
    &_triangles[0], 
    &other->_triangles[0], 
    _numTriangles * sizeof(Triangle));

  _bbox = other->_bbox;

  if (normalize) {
    // compute center of mass
    pxr::GfVec3f center(0.f);
    for (size_t v = 0; v < other->_numVertices; ++v) {
      center += other->GetPosition(v);
    }
    
    center *= 1.0 / (float)_numVertices;

    // translate to origin
    for (size_t v = 0; v < other->_numVertices; ++v) {
      _positions[v]  = other->GetPosition(v) - center;
    }

    // determine radius
    float rMax = 0;
    for (size_t v = 0; v < other->_numVertices; ++v) {
      rMax = std::max(rMax, _positions[v].GetLength());
    }

    // rescale to unit sphere
    float invRMax = 1.f / rMax;
    for (size_t v = 0; v < other->_numVertices; ++v) {
      _positions[v] *= invRMax;
    }

  }
}

void Mesh::Triangulate()
{
  _triangles.resize(GetNumTriangles());
  for(uint32_t j = 0; j < _numTriangles; ++j)
  {
    _triangles[j]._id = j;
  }
}

pxr::GfVec3f Mesh::GetPosition(const Triangle* T) const
{
  pxr::GfVec3f center(0.f);
  for(uint32_t v=0;v<3;v++) center += _positions[T->_vertices[v]];
  center *= 1.f/3.f;
  return center;
}

pxr::GfVec3f Mesh::GetPosition(const Triangle *T, uint32_t index) const
{
  return _positions[T->_vertices[index]];
}

pxr::GfVec3f Mesh::GetPosition(uint32_t index) const
{
  return _positions[index];
}

pxr::GfVec3f Mesh::GetPosition(const PointOnMesh& MP) const
{
  const Triangle* T = &_triangles[MP._triangleId];
  pxr::GfVec3f pos(0.f);
  for(uint32_t i = 0; i < 3; ++i) 
    pos += _positions[T->_vertices[i]] * MP._baryCoords[i];
  return pos;
}

pxr::GfVec3f Mesh::GetNormal(const Triangle *T) const
{
  pxr::GfVec3f center(0.f);
  for(uint32_t v = 0; v < 3; ++v)
      center += _normals[T->_vertices[v]];
  center *= 1.f/3.f;
  return center;
}

pxr::GfVec3f Mesh::GetNormal(const Triangle *T, uint32_t index) const
{
  return _normals[T->_vertices[index]];
}

pxr::GfVec3f Mesh::GetNormal(uint32_t index) const
{
  return _normals[index];
}

pxr::GfVec3f Mesh::GetNormal(const PointOnMesh& point) const
{
  const Triangle* T = &_triangles[point._triangleId];
  pxr::GfVec3f nrm(0.f);
  for(uint32_t i=0;i<3;i++) 
    nrm += _normals[T->_vertices[i]] * point._baryCoords[i];
  return nrm;
}

pxr::GfVec3f Mesh::GetTriangleNormal(uint32_t triangleID) const{
  const Triangle* T = &_triangles[triangleID];
  pxr::GfVec3f A = _positions[T->_vertices[0]];
  pxr::GfVec3f B = _positions[T->_vertices[1]];
  pxr::GfVec3f C = _positions[T->_vertices[2]];

  B -= A;
  C -= A;

  return (B ^ C).GetNormalized();
}

void Mesh::ComputeBoundingBox()
{
  /*
  SubMesh* subMesh;
  _bbox.clear();
  for(uint32_t i=0;i<getNumMeshes();i++)
  {
    subMesh = getSubMesh(i);
    subMesh->_bbox.compute(subMesh);
    _bbox.addInPlace(subMesh->_bbox);
  }
  */
}

pxr::GfBBox3d& Mesh::GetBoundingBox()
{
  return _bbox;
}

void Mesh::Init(const std::vector<pxr::GfVec3f>& positions, 
  const std::vector<int>& counts, 
  const std::vector<int>& connects)
{
  _faceCount = counts;
  _faceConnect = connects;
  _positions = positions;
}

void Mesh::Update(const std::vector<pxr::GfVec3f>& positions)
{

}

void Mesh::Inflate(uint32_t index, float value)
{
  /*
  uint32_t offsetP = _submeshes[index]._offsetPositions;
  for(uint32_t i=0;i<_submeshes[index]._numVertices;i++)
  {
      _positions[offsetP+i*3] += _normals[offsetP+i*3]*value;
      _positions[offsetP+i*3+1] += _normals[offsetP+i*3+1]*value;
      _positions[offsetP+i*3+2] += _normals[offsetP+i*3+2]*value;
  }
  */
}

float Mesh::TriangleArea(uint32_t i)
{
  Triangle* T = &_triangles[i];
  pxr::GfVec3f A = GetPosition(T, 0);
  pxr::GfVec3f B = GetPosition(T, 1);
  pxr::GfVec3f C = GetPosition(T, 2);
  pxr::GfVec3f AB = B - A;
  pxr::GfVec3f AC = C - A;
  pxr::GfVec3f BC = C - B;

  float a = AB.GetLength();
  float b = AC.GetLength();
  float c = BC.GetLength();
  float s = (a + b + c) / 2;
  return sqrt(s * (s - a) * (s - b) * (s - c));
}

float Mesh::AveragedTriangleArea()
{
  if(_triangles.size())
  {
    float averaged = 0;
    for(uint32_t t=0;t<_triangles.size();t++)
        averaged += TriangleArea(t);
    averaged /= (float)_triangles.size();
    return averaged;
  }
  else return 0;
}

bool Mesh::ClosestIntersection(const pxr::GfVec3f& origin, 
  const pxr::GfVec3f& direction, PointOnMesh& result, float maxDistance)
{
  pxr::GfRay ray(origin, direction);

  pxr::GfVec3d A, B, C;
  pxr::GfVec3d baryCoords;
  double distance;
  double minDistance = DBL_MAX;
  bool frontFacing;
  bool hit = false;
  Triangle* T;
  for(uint32_t t = 0;t<_triangles.size();t++)
  {
    T = &_triangles[t];
    A = GetPosition(T, 0);
    B = GetPosition(T, 1);
    C = GetPosition(T, 2);

    if(ray.Intersect(A, B, C, &distance, &baryCoords, &frontFacing, maxDistance))
    {
      if(distance < minDistance)
      {
        minDistance = distance;
        result._baryCoords = pxr::GfVec3f(baryCoords);
        result._triangleId = T->_id;
        hit = true;
      }
    }
  }
  return hit;
}

AMN_NAMESPACE_CLOSE_SCOPE