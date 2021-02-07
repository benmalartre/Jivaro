// Mesh
//----------------------------------------------
#include "mesh.h"
#include "utils.h"
#include <pxr/base/gf/ray.h>

AMN_NAMESPACE_OPEN_SCOPE


void HalfEdge::GetTriangleNormal(const pxr::GfVec3f* positions, 
  pxr::GfVec3f& normal) const
{
  pxr::GfVec3f ab = positions[vertex] - positions[next->vertex];
  pxr::GfVec3f ac = positions[vertex] - positions[next->next->vertex];
  normal = (ab ^ ac).GetNormalized();
}

void HalfEdge::GetVertexNormal(const pxr::GfVec3f* normals, 
  pxr::GfVec3f& normal) const
{
  bool closed = false;
  normal = normals[GetTriangleIndex()];
  size_t numTriangles = 1;
  HalfEdge* current = (HalfEdge*)this;
  while(current->next->twin) {
    current = current->next->twin;
    if(current == this) {
      closed = true;
      break;
    }
    else {
      normal += normals[current->GetTriangleIndex()];
      numTriangles++;
    }
  }

  if(!closed && this->twin) {
    current = this->twin;
    normal += normals[current->GetTriangleIndex()];
    numTriangles++;
    while(current->next->next->twin) {
      current = current->next->next->twin;
      normal += normals[current->GetTriangleIndex()];
      numTriangles++;
    }
  }
  normal *= 1.f/(float)numTriangles;
}

bool HalfEdge::GetFacing(const pxr::GfVec3f* positions, 
  const pxr::GfVec3f& v) const
{
  pxr::GfVec3f tn;
  GetTriangleNormal(positions, tn);
  pxr::GfVec3f dir = (positions[vertex] - v).GetNormalized();
  return pxr::GfDot(tn, dir) > 0.0;
}

bool HalfEdge::GetFacing(const pxr::GfVec3f* positions, 
  const pxr::GfVec3f* normals, const pxr::GfVec3f& v) const
{
  pxr::GfVec3f dir = (positions[vertex] - v).GetNormalized();
  return pxr::GfDot(normals[GetTriangleIndex()], dir) > 0.0;
}

float HalfEdge::GetDot(const pxr::GfVec3f* positions, 
  const pxr::GfVec3f* normals, const pxr::GfVec3f& v) const
{
  pxr::GfVec3f dir = (positions[vertex] - v).GetNormalized();
  return pxr::GfDot(normals[GetTriangleIndex()], dir);
}

Mesh::~Mesh()
{
};

Mesh::Mesh()
{
  _initialized = false;
  _numTriangles = 0;
  _numPoints = 0;
  _numFaces = 0;
}

Mesh::Mesh(const Mesh* other, bool normalize)
{
  _initialized = true;
  _numTriangles = other->_numTriangles;
  _numPoints = other->_numPoints;
  _numSamples = other->_numSamples;
  _numFaces = other->_numFaces;

  _position = other->_position;
  _normal = other->_position;

  _triangles.resize(_numTriangles);
  memcpy(
    &_triangles[0], 
    &other->_triangles[0], 
    _numTriangles * sizeof(Triangle));

  _bbox = other->_bbox;

  if (normalize) {
    // compute center of mass
    pxr::GfVec3f center(0.f);
    for (size_t v = 0; v < other->_numPoints; ++v) {
      center += other->GetPosition(v);
    }
    
    center *= 1.0 / (float)_numPoints;

    // translate to origin
    for (size_t v = 0; v < other->_numPoints; ++v) {
      _position[v]  = other->GetPosition(v) - center;
    }

    // determine radius
    float rMax = 0;
    for (size_t v = 0; v < other->_numPoints; ++v) {
      rMax = std::max(rMax, _position[v].GetLength());
    }

    // rescale to unit sphere
    float invRMax = 1.f / rMax;
    for (size_t v = 0; v < other->_numPoints; ++v) {
      _position[v] *= invRMax;
    }

  }
}

pxr::GfVec3f Mesh::GetPosition(const Triangle* T) const
{
  pxr::GfVec3f center(0.f);
  for(uint32_t v=0;v<3;v++) center += _position[T->vertices[v]];
  center *= 1.f/3.f;
  return center;
}

pxr::GfVec3f Mesh::GetPosition(const Triangle *T, uint32_t index) const
{
  return _position[T->vertices[index]];
}

pxr::GfVec3f Mesh::GetPosition(uint32_t index) const
{
  return _position[index];
}

pxr::GfVec3f Mesh::GetPosition(const PointOnMesh& point) const
{
  const Triangle* T = &_triangles[point.triangleId];
  pxr::GfVec3f pos(0.f);
  for(uint32_t i = 0; i < 3; ++i) 
    pos += _position[T->vertices[i]] * point.baryCoords[i];
  return pos;
}

pxr::GfVec3f Mesh::GetNormal(const Triangle *T) const
{
  pxr::GfVec3f center(0.f);
  for(uint32_t v = 0; v < 3; ++v)
      center += _normal[T->vertices[v]];
  center *= 1.f/3.f;
  return center;
}

pxr::GfVec3f Mesh::GetNormal(const Triangle *T, uint32_t index) const
{
  return _normal[T->vertices[index]];
}

pxr::GfVec3f Mesh::GetNormal(uint32_t index) const
{
  return _normal[index];
}

pxr::GfVec3f Mesh::GetNormal(const PointOnMesh& point) const
{
  const Triangle* T = &_triangles[point.triangleId];
  pxr::GfVec3f nrm(0.f);
  for(uint32_t i=0;i<3;i++) 
    nrm += _normal[T->vertices[i]] * point.baryCoords[i];
  return nrm;
}

pxr::GfVec3f Mesh::GetTriangleNormal(uint32_t triangleID) const{
  const Triangle* T = &_triangles[triangleID];
  pxr::GfVec3f A = _position[T->vertices[0]];
  pxr::GfVec3f B = _position[T->vertices[1]];
  pxr::GfVec3f C = _position[T->vertices[2]];

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

static bool _GetEdgeLatency(int p0, int p1, const int* )
{

}

void Mesh::ComputeHalfEdges()
{
  std::cout << "COMPUTE HALF EDGES MESH :D" << std::endl;
  _halfEdges.resize(_numTriangles * 3);
  std::cout << "NUM HALF EDGES : " << _halfEdges.size() << std::endl;

  pxr::TfHashMap<uint64_t, HalfEdge*, pxr::TfHash> halfEdgesMap;

  HalfEdge* halfEdge = &_halfEdges[0];
  size_t faceIdx = 0;
  size_t numFaceVertices = _faceCounts[faceIdx++];
  size_t numFaceTriangles = numFaceVertices - 2;
  size_t offsetIdx = 0;
  size_t triangleCnt = 0;

  for (int triIndex = 0; triIndex < _numTriangles; ++triIndex)
  {
    const Triangle* T = &_triangles[triIndex];
    uint64_t A = T->vertices[0];
    uint64_t B = T->vertices[1];
    uint64_t C = T->vertices[2];

    // create the half-edge that goes from C to A:
    halfEdgesMap[A | (C << 32)] = halfEdge;
    halfEdge->index = triIndex * 3;
    halfEdge->vertex = C;
    halfEdge->next = 1 + halfEdge;
    ++halfEdge;

    // create the half-edge that goes from A to B:
    halfEdgesMap[B | (A << 32)] = halfEdge;
    halfEdge->index = triIndex * 3 + 1;
    halfEdge->vertex = A;
    halfEdge->next = 1 + halfEdge;
    ++halfEdge;

    // create the half-edge that goes from B to C:
    halfEdgesMap[C | (B << 32)] = halfEdge;
    halfEdge->index = triIndex * 3 + 2;
    halfEdge->vertex = B;
    halfEdge->next = halfEdge - 2;
    ++halfEdge;

    // update face-triangle
    ++triangleCnt;
    if(triangleCnt >= numFaceTriangles) {
      triangleCnt = 0;
      numFaceVertices = _faceCounts[faceIdx++];
      numFaceTriangles = numFaceVertices - 2;
    }
  }

  // verify that the mesh is clean:
  size_t numEntries = halfEdgesMap.size();
  bool problematic = false;
  if(numEntries != _numTriangles * 3)problematic = true;

  // populate the twin pointers by iterating over the hash map:
  uint64_t edgeIndex; 
  size_t boundaryCount = 0;
  for(auto& halfEdge: halfEdgesMap)
  {
    edgeIndex = halfEdge.first;
    uint64_t twinIndex = ((edgeIndex & 0xffffffff) << 32) | (edgeIndex >> 32);
    const auto& it = halfEdgesMap.find(twinIndex);
    if(it != halfEdgesMap.end())
    {
      HalfEdge* twinEdge = (HalfEdge*)it->second;
      twinEdge->twin = halfEdge.second;
      halfEdge.second->twin = twinEdge;
    }
    else {
      _boundary[halfEdge.second->vertex] = true;
      _boundary[halfEdge.second->next->vertex] = true;
      ++boundaryCount;
    }
  }
}

void Mesh::Init(
  const pxr::VtArray<pxr::GfVec3f>& positions, 
  const pxr::VtArray<int>& counts, 
  const pxr::VtArray<int>& connects)
{
  _faceCounts = counts;
  _numFaces = _faceCounts.size();
  _faceConnects = connects;
  _numSamples = _faceConnects.size();
  _position = positions;
  _normal = positions;
  _numPoints = _position.size();

  // initialize boundaries
  _boundary.resize(_numPoints);
  memset(&_boundary[0], false, _numPoints * sizeof(bool));
  
  // build triangles
  TriangulateMesh(_faceCounts, _faceConnects, _triangles);
  _numTriangles = _triangles.size();

  // compute half-edges
  ComputeHalfEdges();
}

void Mesh::Update(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _position = positions;
}

void Mesh::Inflate(uint32_t index, float value)
{
  /*
  uint32_t offsetP = _submeshes[index]._offsetPositions;
  for(uint32_t i=0;i<_submeshes[index]._numPoints;i++)
  {
      _position[offsetP+i*3] += _normal[offsetP+i*3]*value;
      _position[offsetP+i*3+1] += _normal[offsetP+i*3+1]*value;
      _position[offsetP+i*3+2] += _normal[offsetP+i*3+2]*value;
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
        result.baryCoords = pxr::GfVec3f(baryCoords);
        result.triangleId = T->id;
        hit = true;
      }
    }
  }
  return hit;
}

AMN_NAMESPACE_CLOSE_SCOPE