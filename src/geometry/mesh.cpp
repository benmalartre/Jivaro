// Mesh
//----------------------------------------------

#include <cmath>
#include <unordered_map>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/mesh.h>

#include "../geometry/mesh.h"
#include "../geometry/utils.h"


JVR_NAMESPACE_OPEN_SCOPE

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

HalfEdge* 
HalfEdge::GetLongestInTriangle(const pxr::GfVec3f* positions, HalfEdge* edge)
{
  const float edge0 = (positions[edge->next->vertex] - positions[edge->vertex]).GetLength();
  const float edge1 = (positions[edge->next->next->vertex] - positions[edge->next->vertex]).GetLength();
  const float edge2 = (positions[edge->vertex] - positions[edge->next->next->vertex]).GetLength();
  if (edge0 > edge1 && edge0 > edge2)return edge;
  else if (edge1 > edge0 && edge1 > edge2)return edge->next;
  else return edge->next->next;
}

Mesh::Mesh()
  : Geometry(Geometry::MESH)
{
  _initialized = false;
  _numTriangles = 0;
  _numFaces = 0;
}

Mesh::Mesh(const Mesh* other, bool normalize)
  : Geometry(other, Geometry::MESH, normalize)
{
  _initialized = true;
  _numTriangles = other->_numTriangles;
  _numSamples = other->_numSamples;
  _numFaces = other->_numFaces;

  _normals = other->_normals;

  _triangles.resize(_numTriangles);
  memcpy(
    &_triangles[0], 
    &other->_triangles[0], 
    _numTriangles * sizeof(Triangle));
}

Mesh::Mesh(const pxr::UsdGeomMesh& mesh)
  : Geometry(Geometry::MESH)
{
  pxr::UsdAttribute pointsAttr = mesh.GetPointsAttr();
  pxr::UsdAttribute faceVertexCountsAttr = mesh.GetFaceVertexCountsAttr();
  pxr::UsdAttribute faceVertexIndicesAttr = mesh.GetFaceVertexIndicesAttr();

  pointsAttr.Get(&_positions, pxr::UsdTimeCode::Default());
  faceVertexCountsAttr.Get(&_faceVertexCounts, pxr::UsdTimeCode::Default());
  faceVertexIndicesAttr.Get(&_faceVertexIndices, pxr::UsdTimeCode::Default());
  _numFaces = _faceVertexCounts.size();
  _numSamples = _faceVertexIndices.size();

  Init();
}

uint32_t Mesh::GetFaceVertexIndex(uint32_t face, uint32_t vertex)
{
  size_t accum = 0;
  for(size_t i=0; i < face; ++i)accum += _faceVertexCounts[i];
  return _faceVertexIndices[accum + vertex];
}

void Mesh::SetAllEdgesLatencyReal()
{
  for (auto& halfEdge : _halfEdges) {
    halfEdge.latency = HalfEdge::REAL;
  }
}

void Mesh::UpdateTopologyFromHalfEdges()
{
  _numFaces = _halfEdges.size() / 3;
  _numSamples = _halfEdges.size();
  _numTriangles = _numFaces;

  _faceVertexCounts.assign(_numFaces, 3);
  _faceVertexIndices.resize(_numSamples);
  size_t faceVertexIndex = 0;
  for (auto& edge : _halfEdges) {
    _faceVertexIndices[faceVertexIndex++] = edge.vertex;
  }
}

void Mesh::GetCutVerticesFromUVs(const pxr::VtArray<pxr::GfVec2d>& uvs, pxr::VtArray<int>* cuts)
{
  size_t numVertices = _positions.size();

  pxr::VtArray<int> visited(numVertices);
  for (auto& v : visited) v = 0;
  
  pxr::VtArray<pxr::GfVec2d> uvPositions(numVertices);
  
  size_t uvIndex = 0;
  for (const int& faceVertexIndex : _faceVertexIndices) {
    pxr::GfVec2d uv = uvs[uvIndex++];
    switch (visited[faceVertexIndex]) {
      case 0:
        uvPositions[faceVertexIndex] = uv;
        visited[faceVertexIndex]++;
        break;
      case 1:
        if (uv != uvPositions[faceVertexIndex]) {
          cuts->push_back(faceVertexIndex);
          visited[faceVertexIndex]++;
        }
        break;
      default:
        continue;
    }
  }
  
  cuts->resize(2);
  (*cuts)[0] = 1;
  (*cuts)[1] = 3;
}

static bool _IsHalfEdgesUVSeparated(const HalfEdge* lhs, const HalfEdge* rhs)
{
  return false;
}


void Mesh::GetCutEdgesFromUVs(const pxr::VtArray<pxr::GfVec2d>& uvs, pxr::VtArray<int>* cuts)
{
  cuts->reserve(_uniqueEdges.size());
  for (const HalfEdge& halfEdge: _halfEdges) {
    if (/*halfEdge.latency == HalfEdge::REAL && */halfEdge.twin) {
      const pxr::GfVec2d& start_lhs = uvs[halfEdge.index];
      const pxr::GfVec2d& end_lhs = uvs[halfEdge.next->index];
      const pxr::GfVec2d& start_rhs = uvs[halfEdge.twin->next->index];
      const pxr::GfVec2d& end_rhs = uvs[halfEdge.twin->next->next->index];
      if (start_lhs != end_rhs) cuts->push_back(halfEdge.index);
      if (end_rhs != start_rhs) cuts->push_back(halfEdge.twin->index);
    }
  }
}

pxr::GfVec3f Mesh::GetPosition(size_t idx) const
{
  return _positions[idx];
}

pxr::GfVec3f Mesh::GetTrianglePosition(const Triangle* T) const
{
  pxr::GfVec3f center(0.f);
  for(uint32_t v=0;v<3;v++) center += _positions[T->vertices[v]];
  center *= 1.f/3.f;
  return center;
}

pxr::GfVec3f Mesh::GetTriangleVertexPosition(const Triangle *T, uint32_t index) const
{
  return _positions[T->vertices[index]];
}

pxr::GfVec3f Mesh::GetPosition(const Location& point) const
{
  const Triangle* T = &_triangles[point.id];
  pxr::GfVec3f pos(0.f);
  for(uint32_t i = 0; i < 3; ++i) 
    pos += _positions[T->vertices[i]] * point.baryCoords[i];
  return pos;
}

pxr::GfVec3f Mesh::GetTriangleNormal(const Triangle *T) const
{
  pxr::GfVec3f center(0.f);
  for(uint32_t v = 0; v < 3; ++v)
      center += _normals[T->vertices[v]];
  center *= 1.f/3.f;
  return center;
}

pxr::GfVec3f Mesh::GetTriangleVertexNormal(const Triangle *T, uint32_t index) const
{
  return _normals[T->vertices[index]];
}

pxr::GfVec3f Mesh::GetNormal(const Location& point) const
{
  const Triangle* T = &_triangles[point.id];
  pxr::GfVec3f nrm(0.f);
  for(uint32_t i=0;i<3;i++) 
    nrm += _normals[T->vertices[i]] * point.baryCoords[i];
  return nrm;
}

pxr::GfVec3f Mesh::GetTriangleNormal(uint32_t triangleID) const
{
  const Triangle* T = &_triangles[triangleID];
  pxr::GfVec3f A = _positions[T->vertices[0]];
  pxr::GfVec3f B = _positions[T->vertices[1]];
  pxr::GfVec3f C = _positions[T->vertices[2]];

  B -= A;
  C -= A;

  return (B ^ C).GetNormalized();
}

const std::vector<HalfEdge*> Mesh::GetUniqueEdges()
{
  size_t numUniqueEdges = _uniqueEdges.size();
  if (!numUniqueEdges) {
    ComputeHalfEdges();
    numUniqueEdges = _uniqueEdges.size();
  }
  std::vector<HalfEdge*> halfEdges(numUniqueEdges);
  for (size_t i = 0; i < numUniqueEdges; ++i) {
    halfEdges[i] = &_halfEdges[_uniqueEdges[i]];
  }
  return halfEdges;
}


static bool _GetEdgeLatency(int p0, int p1, const int* )
{
  return false;
}

static void _SetHalfEdgeLatency(HalfEdge* halfEdge, int numFaceTriangles, 
  int faceTriangleIndex, int triangleEdgeIndex)
{
  if (faceTriangleIndex == 0) {
    if (triangleEdgeIndex == 2) halfEdge->latency = HalfEdge::IMPLICIT;
    else halfEdge->latency = HalfEdge::REAL;
  } else if (faceTriangleIndex == numFaceTriangles - 1) {
    if (triangleEdgeIndex == 0) halfEdge->latency = HalfEdge::IMPLICIT;
    else halfEdge->latency = HalfEdge::REAL;
  } else {
    if (triangleEdgeIndex == 1) halfEdge->latency = HalfEdge::REAL;
    else halfEdge->latency = HalfEdge::IMPLICIT;
  }
}

static void _RemovePoint(pxr::VtArray<pxr::GfVec3f>& points, size_t idx)
{
  points.erase(points.begin() + idx);
}

static void _RemoveEdge(pxr::VtArray<HalfEdge>& halfEdges, HalfEdge* edge)
{
  for(size_t i=0; i < halfEdges.size(); ++i) {
    if(&halfEdges[i] == edge) {
      halfEdges.erase(halfEdges.begin() + i);
    }
  }
  /*
  for(std::VtArray<HalfEdge>::iterator& it = halfEdges.begin(); it < halfEdges.end(); ++it) {
    if (&(*it) == edge) {
      halfEdges.erase(it);
    }
  }
  */
}

void Mesh::ComputeHalfEdges()
{
  _halfEdges.resize(_numTriangles * 3);

  pxr::TfHashMap<uint64_t, HalfEdge*, pxr::TfHash> halfEdgesMap;
  std::vector<bool> used;
  used.assign(_numTriangles, false);

  HalfEdge* halfEdge = &_halfEdges[0];
  size_t numFaceTriangles;
  size_t faceIdx = 0;
  size_t offsetIdx = 0;
  size_t triangleIdx = 0;
  size_t faceTriangleIdx = 0;

  for (const auto& faceVertexCount: _faceVertexCounts)
  { 
    numFaceTriangles = faceVertexCount - 2;
    faceTriangleIdx = 0;
    for (int faceTriangle = 0; faceTriangle < numFaceTriangles; ++faceTriangle)
    {
      const Triangle* T = &_triangles[triangleIdx];
      uint64_t A = T->vertices[0];
      uint64_t B = T->vertices[1];
      uint64_t C = T->vertices[2];

      // create the half-edge that goes from C to A:
      halfEdgesMap[A | (C << 32)] = halfEdge;
      halfEdge->index = triangleIdx * 3;
      halfEdge->vertex = C;
      halfEdge->next = 1 + halfEdge;
      _SetHalfEdgeLatency(halfEdge, numFaceTriangles, faceTriangleIdx, 0);
      ++halfEdge;

      // create the half-edge that goes from A to B:
      halfEdgesMap[B | (A << 32)] = halfEdge;
      halfEdge->index = triangleIdx * 3 + 1;
      halfEdge->vertex = A;
      halfEdge->next = 1 + halfEdge;
      _SetHalfEdgeLatency(halfEdge, numFaceTriangles, faceTriangleIdx, 1);
      ++halfEdge;

      // create the half-edge that goes from B to C:
      halfEdgesMap[C | (B << 32)] = halfEdge;
      halfEdge->index = triangleIdx * 3 + 2;
      halfEdge->vertex = B;
      halfEdge->next = halfEdge - 2;
      _SetHalfEdgeLatency(halfEdge, numFaceTriangles, faceTriangleIdx, 2);
      ++halfEdge;

      triangleIdx++;
      faceTriangleIdx++;
    }
  }

  // verify that the mesh is clean:
  size_t numEntries = halfEdgesMap.size();
  bool problematic = false;
  if(numEntries != (size_t)(_numTriangles * 3))problematic = true;

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

  _uniqueEdges.reserve(_halfEdges.size());
  size_t halfEdgeIndex = 0;
  for (const HalfEdge& halfEdge : _halfEdges) {
    if (halfEdge.twin) {
      if (halfEdge.vertex < halfEdge.twin->vertex)
        _uniqueEdges.push_back(halfEdgeIndex);
    }
    else {
      _uniqueEdges.push_back(halfEdgeIndex);
    }
    halfEdgeIndex++;
  }

  for (HalfEdge& halfEdge : _halfEdges) {
    if (used[halfEdge.GetTriangleIndex()])continue;
    HalfEdge* longest = HalfEdge::GetLongestInTriangle(GetPositionsCPtr(), &halfEdge);
    size_t triPairIdx = _trianglePairs.size();
    if (longest->twin) {
      _trianglePairs.push_back(TrianglePair(
        triPairIdx,
        &_triangles[longest->GetTriangleIndex()],
        &_triangles[longest->twin->GetTriangleIndex()]
      ));
      used[longest->GetTriangleIndex()] = true;
      used[longest->twin->GetTriangleIndex()] = true;
    } else {
      _trianglePairs.push_back(TrianglePair(
        triPairIdx,
        &_triangles[longest->GetTriangleIndex()],
        NULL
      ));
      used[longest->GetTriangleIndex()] = true;
    }
  }
}

static HalfEdge* _GetPreviousAdjacentEdge(Mesh* mesh, HalfEdge* edge)
{
  int vertex = edge->vertex;
  HalfEdge* current = edge->next;
  while (current->next->vertex != vertex) {
    current = current->next;
  }
  if (current->twin)return current->twin->next;
  return NULL;
}

static HalfEdge* _GetNextjacentEdge(Mesh* mesh, HalfEdge* edge)
{
  if (edge->twin)return edge->twin->next;
  return NULL;

}

static HalfEdge* _GetNextEdge(HalfEdge* edge, short latency = HalfEdge::ANY)
{
  int vertex = edge->vertex;
  HalfEdge* current = edge->next;
  switch (latency) {
  case HalfEdge::REAL:
    while (current->next) {
      if (current->next->latency == HalfEdge::REAL)return current->next;
      if (!current->twin)return NULL;
      current = current->twin->next;
    }
  case HalfEdge::IMPLICIT:
  case HalfEdge::VIRTUAL:
  case HalfEdge::ANY:
    return current->next;
  default:
    return current->next;
  }
}

static HalfEdge* _GetPreviousEdge(HalfEdge* edge, short latency=HalfEdge::ANY)
{
  int vertex = edge->vertex;
  HalfEdge* current = edge->next;
  switch (latency) {
  case HalfEdge::REAL:
    while (current->next->vertex != vertex) {
      if (current->latency != HalfEdge::REAL && current->twin) {
        current = current->twin->next;
      }
      else {
        current = current->next;
      }
    }
    return current;
  case HalfEdge::IMPLICIT:
  case HalfEdge::VIRTUAL:
  case HalfEdge::ANY:
    while (current->next->vertex != vertex) {
      current = current->next;
    }
    return current;
  default:
    while (current->next->vertex != vertex) {
      current = current->next;
    }
    return current;
  }
  
}

static bool _IsNeighborRegistered(const pxr::VtArray<int>& neighbors, int idx)
{
  for (int neighbor: neighbors) {
    if (neighbor == idx)return true;
  }
  return false;
}

void Mesh::ComputeNeighbors()
{
  size_t numPoints = _positions.size();
  _neighbors.clear();
  _neighbors.resize(numPoints);

  for (HalfEdge& halfEdge : _halfEdges) {
    int edgeIndex = halfEdge.index;
    int vertexIndex = halfEdge.vertex;
    HalfEdge* startEdge = &halfEdge;
    HalfEdge* currentEdge = startEdge;
    HalfEdge* nextEdge = NULL;
    pxr::VtArray<int>& neighbors = _neighbors[vertexIndex];
    if (!_IsNeighborRegistered(neighbors, startEdge->next->vertex)) {
      neighbors.push_back(startEdge->next->vertex);
    }

    short state = 1;
    while (state == 1) {
      nextEdge = _GetNextjacentEdge(this, currentEdge);
      if (!nextEdge) {
        state = 0;
      } else if (nextEdge == startEdge) {
        state = -1;
      } else {
        if (!_IsNeighborRegistered(neighbors, nextEdge->next->vertex)) {
          neighbors.push_back(nextEdge->next->vertex);
        }
        currentEdge = nextEdge;
      }
    }
    
    if(state == 0) {
      HalfEdge* previousEdge = NULL;
      currentEdge = startEdge;
      previousEdge = _GetPreviousEdge(currentEdge);
      if (!_IsNeighborRegistered(neighbors, previousEdge->vertex)) {
        neighbors.push_back(previousEdge->vertex);
      }
      state = 1;
      while (state == 1) {
        previousEdge = _GetPreviousAdjacentEdge(this, currentEdge);
        if (!previousEdge) {
          state = 0;
        }
        else if (previousEdge == startEdge) {
          state = -1;
        }
        else {
          if (!_IsNeighborRegistered(neighbors, previousEdge->next->vertex)) {
            neighbors.push_back(previousEdge->next->vertex);
          }
          currentEdge = previousEdge;
        }
      }
    }
  }
}

void Mesh::SetTopology(
  const pxr::VtArray<pxr::GfVec3f>& positions,
  const pxr::VtArray<int>& faceVertexCounts,
  const pxr::VtArray<int>& faceVertexIndices
)
{
  _faceVertexCounts = faceVertexCounts;
  _numFaces = _faceVertexCounts.size();
  _faceVertexIndices = faceVertexIndices;
  _numSamples = _faceVertexIndices.size();
  _positions = positions;
  _normals = positions;

  Init();
}

void Mesh::Init()
{
  size_t numPoints = _positions.size();
  // initialize boundaries
  _boundary.resize(numPoints);
  memset(&_boundary[0], false, numPoints * sizeof(bool));
  
  // build triangles
  TriangulateMesh(_faceVertexCounts, _faceVertexIndices, _triangles);
  _numTriangles = _triangles.size();

  // compute normals
  ComputeVertexNormals(_positions, _faceVertexCounts, 
    _faceVertexIndices, _triangles, _normals);

  // compute half-edges
  ComputeHalfEdges();
  ComputeBoundingBox();
  // compute neighbors
  //ComputeNeighbors();
}

void 
Mesh::Update(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _positions = positions;
}

const HalfEdge* 
_GetNextCutEdge(const HalfEdge* edge, 
  const std::vector<bool>& doCutEdge, int& cnt)
{
  HalfEdge* current = (HalfEdge*)edge;
  while (current->next->twin) {
    current = current->next->twin;
    cnt++;
    if (current == edge) {
      return edge;
    }
    else {
      if (doCutEdge[current->index]) {
        return current;
      }
    }
  }
  return edge;
}

static int 
_CountPointCuts(const std::vector<bool>& doCutEdge, HalfEdge* start) {
  HalfEdge* current = start;
  int count = 0;
  bool search = true;
  while (search) {
    if (current->twin) {
      if (doCutEdge[current->index] && doCutEdge[current->twin->next->index]) {
        count++;
      }
      current = current->twin->next;
    } else {
      search = false;
    }
    if (current == start)search = false;
  }
  current = start;
  /*
  bool reverseSearch = true;
  while (reverseSearch) {
    HalfEdge* previous = _GetPreviousEdge(current)->twin;
    if (doCutEdge[current->index] && doCutEdge[previous->twin->index]) {
      count++;
      current = previous;
    }
    else {
      reverseSearch = false;
    }
  }
  */
  return count;
}

void Mesh::FlipEdge(size_t index)
{
  /* 
  flip-flop two triangle by their common edge
                                     
       3 _ _ _ 2        3 _ _ _ 2    
        |    /   2    3   \    |     
        |   /  /|      |\  \   |     
        |  /  / |      | \  \  |     
        | /  /  |      |  \  \ |     
        |/  /   |      |   \  \|     
       0   /_ _ |      | _ _\   1    
          0      1    0      1       
  */
  HalfEdge* edge = &_halfEdges[index];
  std::cout << "flip edge : " << edge << "," << edge->twin << std::endl;
  if(!edge->twin)return;

  HalfEdge* twin = edge->twin;

  HalfEdge* edges[6] = {
    edge, edge->next, edge->next->next,
    twin, twin->next, twin->next->next
  };

  uint32_t vertices[4] = {
    edges[1]->vertex,
    edges[2]->vertex,
    edges[4]->vertex,
    edges[5]->vertex
  };

  Triangle& t1 = _triangles[edge->GetTriangleIndex()];
  Triangle& t2 = _triangles[twin->GetTriangleIndex()];
  std::cout << t1.vertices << std::endl;
  std::cout << t2.vertices << std::endl;

  std::swap(edge->index, twin->index);
  edges[0]->vertex = vertices[3];
  edges[3]->vertex = vertices[1];
  edges[0]->next = edges[2];
  edges[1]->next = edges[0];
  edges[2]->next = edges[1];
  edges[3]->next = edges[5];
  edges[4]->next = edges[3];
  edges[5]->next = edges[4];
  std::swap(edges[2]->index, edges[5]->index);

  t1.vertices = pxr::GfVec3i(vertices[0], vertices[1], vertices[3]);
  t2.vertices = pxr::GfVec3i(vertices[3], vertices[1], vertices[2]);

  std::cout << t1.vertices << std::endl;
  std::cout << t2.vertices << std::endl;
}

void Mesh::SplitEdge(size_t index)
{
  size_t numPoints = _positions.size();
  size_t numEdges = _halfEdges.size();
  HalfEdge& currentEdge = _halfEdges[index];
  if (currentEdge.latency != HalfEdge::REAL)return;

  HalfEdge* twinEdge = currentEdge.twin;
  if(twinEdge) _halfEdges.resize(numEdges + 6);
  else _halfEdges.resize(numEdges + 3);

  const pxr::GfVec3f p = (_positions[currentEdge.vertex] + _positions[currentEdge.next->vertex]) * 0.5f;
  _positions.push_back(p);
  _normals.push_back(p);

  HalfEdge* nextEdge = _GetNextEdge(&currentEdge, HalfEdge::REAL);
  HalfEdge* previousEdge = _GetPreviousEdge(&currentEdge, HalfEdge::REAL);

  HalfEdge* halfEdge = &_halfEdges[numEdges];
  halfEdge->index = numEdges;
  halfEdge->vertex = numPoints;
  halfEdge->next = nextEdge;
  halfEdge->latency = HalfEdge::REAL;
  HalfEdge* n1 = halfEdge;
  halfEdge++;

  halfEdge->index = numEdges + 1;
  halfEdge->vertex = numPoints;
  halfEdge->next = previousEdge;
  halfEdge->latency = HalfEdge::IMPLICIT;
  HalfEdge* n2 = halfEdge;
  halfEdge++;

  halfEdge->index = numEdges + 2;
  halfEdge->vertex = numPoints;
  halfEdge->next = n2;
  halfEdge->latency = HalfEdge::IMPLICIT;
  HalfEdge* n3 = halfEdge;

  nextEdge->next = n3;
  currentEdge.next = n1;

  numPoints++;
  _numTriangles++;
  
  if (twinEdge) {
    HalfEdge* twinNextEdge = _GetNextEdge(twinEdge, HalfEdge::REAL);
    HalfEdge* twinPreviousEdge = _GetPreviousEdge(twinEdge, HalfEdge::REAL);

    halfEdge->index = numEdges + 3;
    halfEdge->vertex = numPoints;
    halfEdge->next = twinNextEdge;
    halfEdge->latency = HalfEdge::REAL;
    HalfEdge* n4 = halfEdge;
    halfEdge++;

    halfEdge->index = numEdges + 4;
    halfEdge->vertex = numPoints;
    halfEdge->next = twinNextEdge;
    halfEdge->latency = HalfEdge::IMPLICIT;
    HalfEdge* n5 = halfEdge;
    halfEdge++;

    halfEdge->index = numEdges + 5;
    halfEdge->vertex = twinEdge->vertex;
    halfEdge->next = n4;
    halfEdge->latency = HalfEdge::IMPLICIT;
    HalfEdge* n6 = halfEdge;

    twinNextEdge->next = n3;
    twinEdge->next = n1;

    numPoints++;
    _numTriangles++;
  }
}

void Mesh::CollapseEdge(size_t index)
{
  HalfEdge* currentEdge = &_halfEdges[index];
  HalfEdge* twinEdge = currentEdge->twin;

  int p1 = currentEdge->vertex;
  int p2 = currentEdge->next->vertex;

  if (p1 > p2) {
    _positions[p2] = (_positions[p1] + _positions[p2]) * 0.5f;
    _RemovePoint(_positions, p1);
  } else {
    _positions[p1] = (_positions[p1] + _positions[p2]) * 0.5f;
    _RemovePoint(_positions, p2);
  }
  HalfEdge* nextEdge = _GetNextEdge(currentEdge);
  HalfEdge* previousEdge = _GetPreviousEdge(currentEdge);
  if (nextEdge->twin && previousEdge->twin) {
    nextEdge->twin->twin = previousEdge;
    previousEdge->twin->twin = nextEdge;
  }

  _RemoveEdge(_halfEdges, nextEdge);
  _RemoveEdge(_halfEdges, previousEdge);
  

  //previousEdge->next = nextEd
}

void Mesh::DisconnectEdges(const pxr::VtArray<int>& cutEdges)
{
  size_t numHalfEdges = _halfEdges.size();
  std::vector<std::vector<int>> insertedPoints(_positions.size());
  std::vector<bool> doCutEdge(numHalfEdges);

  pxr::VtArray<pxr::GfVec3f> points = _positions;
  size_t baseNewPoints = points.size();
  pxr::VtArray<int> indices = _faceVertexIndices;
  for(size_t i = 0; i < numHalfEdges; ++i) doCutEdge[i] = false;
  for(const auto& cutEdge: cutEdges) doCutEdge[cutEdge] = true;
  /*
  for(size_t faceVertex = 0; faceVertex < _faceVertexIndices.size(); ++faceVertex) {
    if(doCutEdge[faceVertex]) {
      if(! insertedPoints[_faceVertexIndices[faceVertex]].size()) {
        insertedPoints[_faceVertexIndices[faceVertex]].push_back(baseNewPoints++);
        points.push_back(_positions[_faceVertexIndices[faceVertex]]);
      } else {
        indices[faceVertex] = insertedPoints[_faceVertexIndices[faceVertex]][0];
      }
    }
  }

  SetTopology(points, _faceVertexCounts, indices);
  */

  for (size_t faceVertex = 0; faceVertex < _faceVertexIndices.size(); ++faceVertex) {
    HalfEdge* halfEdge = &_halfEdges[faceVertex];
    int numCuts = _CountPointCuts(doCutEdge, halfEdge);
    if(numCuts)std::cout << "NUM CUTS : " << numCuts << std::endl;
  }
}

void Mesh::Flatten(const pxr::VtArray<pxr::GfVec2d>& uvs, const pxr::TfToken& interpolation)
{
  if (interpolation == pxr::UsdGeomTokens->vertex) {
    for (size_t i = 0; i < uvs.size(); ++i) {
      const pxr::GfVec2d& uv = uvs[i];
      _positions[i] = pxr::GfVec3f(uv[0], 0.f, uv[1]);
    }
  } else if (interpolation == pxr::UsdGeomTokens->faceVarying) {
    for (size_t i = 0; i < uvs.size(); ++i) {
      const pxr::GfVec2d& uv = uvs[i];
      _positions[_faceVertexIndices[i]] = pxr::GfVec3f(uv[0], 0.f, uv[1]);
    }
  }
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
  pxr::GfVec3f A = GetTriangleVertexPosition(T, 0);
  pxr::GfVec3f B = GetTriangleVertexPosition(T, 1);
  pxr::GfVec3f C = GetTriangleVertexPosition(T, 2);
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
  const pxr::GfVec3f& direction, Location& result, float maxDistance)
{
  pxr::GfRay ray(origin, direction);

  pxr::GfVec3d p0, p1, p2;
  pxr::GfVec3d baryCoords;
  double distance;
  double minDistance = DBL_MAX;
  bool frontFacing;
  bool hit = false;
  Triangle* tri;
  for(uint32_t t = 0;t<_triangles.size();t++)
  {
    tri = &_triangles[t];
    p0 = GetTriangleVertexPosition(tri, 0);
    p1 = GetTriangleVertexPosition(tri, 1);
    p2 = GetTriangleVertexPosition(tri, 2);

    if(ray.Intersect(p0, p1, p2, &distance, &baryCoords, &frontFacing, maxDistance))
    {
      if(distance < minDistance)
      {
        minDistance = distance;
        result.baryCoords = pxr::GfVec3f(baryCoords);
        result.id = tri->id;
        hit = true;
      }
    }
  }
  return hit;
}

void Mesh::PolygonSoup(size_t numPolygons, const pxr::GfVec3f& minimum, 
  const pxr::GfVec3f& maximum)
{
  size_t numTriangles = numPolygons;
  size_t numPoints = numPolygons * 3;
  pxr::VtArray<pxr::GfVec3f> position(numPoints);

  float radius = (maximum - minimum).GetLength() / 10.f;
  for(size_t i=0; i < numPolygons; ++i) {
    float bx = RANDOM_0_1;
    float by = RANDOM_0_1;
    float bz = RANDOM_0_1;
    pxr::GfVec3f center(
      minimum[0] * (1.f - bx) + maximum[0] * bx,
      minimum[1] * (1.f - by) + maximum[1] * by,
      minimum[2] * (1.f - bz) + maximum[2] * bz
    );

    pxr::GfVec3f normal(
      RANDOM_LO_HI(-1.f, 1.f),
      RANDOM_LO_HI(-1.f, 1.f),
      RANDOM_LO_HI(-1.f, 1.f)
    );

    normal.Normalize();

    position[i * 3 + 0] = center + pxr::GfVec3f(1.f, 0.f, 0.f);
    position[i * 3 + 1] = center + pxr::GfVec3f(0.f, 1.f, 0.f);
    position[i * 3 + 2] = center + pxr::GfVec3f(0.f, 0.f, 1.f);
  }

  pxr::VtArray<int> faceVertexCount(numTriangles);
  for(size_t i=0; i < numTriangles; ++i) {
    faceVertexCount[i] = 3;
  }

  pxr::VtArray<int> faceVertexConnect(numPoints);
  for(size_t i=0; i < numPoints; ++i) {
    faceVertexConnect[i] = i;
  }
  SetTopology(position, faceVertexCount, faceVertexConnect);
}

void 
Mesh::MaterializeSamples(const pxr::VtArray<pxr::GfVec3f>& points, float size)
{
  size_t numTriangles = points.size();
  size_t numPoints = numTriangles * 3;
  pxr::VtArray<int> faceVertexCount(numTriangles);
  for (size_t i = 0; i < numTriangles; ++i) {
    faceVertexCount[i] = 3;
  }

  pxr::VtArray<int> faceVertexConnect(numPoints);
  for (size_t i = 0; i < numPoints; ++i) {
    faceVertexConnect[i] = i;
  }

  pxr::VtArray<pxr::GfVec3f> positions(numPoints);
  for (size_t p = 0; p < numTriangles; ++p) {
    positions[p * 3] = points[p] - pxr::GfVec3f(size, 0, 0);
    positions[p * 3 + 1] = points[p] + pxr::GfVec3f(size, 0, 0);
    positions[p * 3 + 2] = points[p] + pxr::GfVec3f(0, size, 0);
  }
  SetTopology(positions, faceVertexCount, faceVertexConnect);
}


void Mesh::ColoredPolygonSoup(size_t numPolygons, 
  const pxr::GfVec3f& minimum, const pxr::GfVec3f& maximum)
{
  //mesh->PolygonSoup(65535);
  pxr::GfMatrix4f space(1.f);
  TriangularGrid2D(10.f, 6.f, space, 0.2f);
  Randomize(0.05f);
}

Mesh* MakeOpenVDBSphere(pxr::UsdStageRefPtr& stage, const pxr::TfToken& path)
{
  Mesh* mesh = new Mesh();
  /*
  mesh->OpenVDBSphere(6.66, pxr::GfVec3f(3.f, 7.f, 4.f));

  pxr::SdfPath path(path);
  pxr::UsdGeomMesh vdbSphere = pxr::UsdGeomMesh::Define(stage, path);
  vdbSphere.CreatePointsAttr(pxr::VtValue(mesh->GetPositions()));
  vdbSphere.CreateNormalsAttr(pxr::VtValue(mesh->GetNormals()));
  vdbSphere.CreateFaceVertexIndicesAttr(pxr::VtValue(mesh->GetFaceConnects()));
  vdbSphere.CreateFaceVertexCountsAttr(pxr::VtValue(mesh->GetFaceCounts()));

  vdbSphere.CreateSubdivisionSchemeAttr(pxr::VtValue(pxr::UsdGeomTokens->none));

  std::cout << "CREATED OPENVDB SPHERE !!!" << std::endl;
  */

  return mesh;
}



void Mesh::TriangularGrid2D(float width, float height, const pxr::GfMatrix4f& space, float size)
{
  size_t numX = (width / size ) * 0.5 + 1;
  size_t numY = (height / size) + 1;
  size_t numPoints = numX * numY;
  size_t numTriangles = (numX - 1) * 2 * (numY - 1);
  size_t numSamples = numTriangles * 3;
  pxr::VtArray<pxr::GfVec3f> position(numPoints);

  float spaceX = size * 2.0 / width;
  float spaceY = size / height;

  for(size_t y = 0; y < numY; ++y) {
    for(size_t x = 0; x < numX; ++x) {
      size_t vertexId = y * numX + x;
      if(y %2 == 0)position[vertexId][0] = x * spaceX + spaceX * 0.5f;
      else position[vertexId][0] = x * spaceX;
      position[vertexId][1] = 0.f;
      position[vertexId][2] = y * spaceY;
      position[vertexId] = space.Transform(position[vertexId]);
    }
  }
  
  pxr::VtArray<int> faceVertexCount(numTriangles);
  for(size_t i=0; i < numTriangles; ++i) {
    faceVertexCount[i] = 3;
  }

  size_t numRows = numY - 1;
  size_t numTrianglesPerRow = (numX - 1) ;
  pxr::VtArray<int> faceVertexConnect(numSamples);
  
  size_t k = 0;
  for(size_t i=0; i < numRows; ++i) {
    for (size_t j = 0; j < numTrianglesPerRow; ++j) {
      if (i % 2 == 0) {
        faceVertexConnect[k++] = i * numX + j;
        faceVertexConnect[k++] = i * numX + j + 1;
        faceVertexConnect[k++] = (i + 1) * numX + j + 1;

        faceVertexConnect[k++] = (i + 1) * numX + j + 1;
        faceVertexConnect[k++] = (i + 1) * numX + j;
        faceVertexConnect[k++] = i * numX + j;
      }
      else {
        faceVertexConnect[k++] = i * numX + j;
        faceVertexConnect[k++] = i * numX + j + 1;
        faceVertexConnect[k++] = (i + 1) * numX + j;

        faceVertexConnect[k++] = (i + 1) * numX + j + 1;
        faceVertexConnect[k++] = (i + 1) * numX + j;
        faceVertexConnect[k++] = i * numX + j + 1;
      }
    }
  }
  
  pxr::VtArray<pxr::GfVec3f> colors(numPoints);
  for(size_t i=0; i < numPoints / 3; ++i) {
    pxr::GfVec3f color(
      RANDOM_0_1,
      RANDOM_0_1,
      RANDOM_0_1
    );
    colors[i * 3] = color;
    colors[i * 3 + 1] = color;
    colors[i * 3 + 2] = color;
  }
 

  SetTopology(position, faceVertexCount, faceVertexConnect);
}

void Mesh::OpenVDBSphere(float radius, const pxr::GfVec3f& center)
{

}

void Mesh::VoronoiDiagram(const std::vector<pxr::GfVec3f>& points)
{
  /*
  size_t numPoints = points.size();
  std::vector<mygal::Vector2<double>> _positions(numPoints);
  for (size_t p=0; p < numPoints; ++p) {
    _positions[p].x = points[p][0];
    _positions[p].y = points[p][2];
  }
  mygal::FortuneAlgorithm<double> algorithm(_positions);
  algorithm.construct();

  algorithm.bound(mygal::Box<double>{-0.55, -0.55, 0.55, 0.55});
  mygal::Diagram<double> diagram = algorithm.getDiagram();
  diagram.intersect(mygal::Box<double>{-0.5, -0.5, 0.5, 0.5});

  pxr::VtArray<pxr::GfVec3f> positions;
  pxr::VtArray<int> faceVertexCounts;
  pxr::VtArray<int> faceVertexConnects;
  pxr::VtArray<pxr::GfVec3f> colors;

  for (const auto& site : diagram.getSites())
  {
    auto center = site.point;
    auto face = site.face;
    auto halfEdge = face->outerComponent;
    if (halfEdge == nullptr)
      continue;
    while (halfEdge->prev != nullptr)
    {
      halfEdge = halfEdge->prev;
      if (halfEdge == face->outerComponent)
        break;
    }
    auto start = halfEdge;
    const pxr::GfVec3f color(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    size_t numFaceVertices = 0;
    if (halfEdge->origin != nullptr) {
      auto origin = (halfEdge->origin->point - center) + center;
      faceVertexConnects.push_back(positions.size());
      positions.push_back(pxr::GfVec3f(origin.x, 0.f, origin.y));
      colors.push_back(color);
      numFaceVertices++;
    }

    while (halfEdge != nullptr)
    {
      if (halfEdge->origin != nullptr && halfEdge->destination != nullptr)
      {
        auto destination = (halfEdge->destination->point - center) + center;
        faceVertexConnects.push_back(positions.size());
        positions.push_back(pxr::GfVec3f(destination.x, 0.f, destination.y));
        colors.push_back(color);
        numFaceVertices++;
      }
      halfEdge = halfEdge->next;
      if (halfEdge == start) {
        faceVertexCounts.push_back(numFaceVertices);
        break;
      }
    }
  }

  SetTopology(positions, faceVertexCounts, faceVertexConnects);
  */
}

void Mesh::Randomize(float value)
{
  for(auto& pos: _positions) {
    pos += pxr::GfVec3f(
      RANDOM_LO_HI(-value, value),
      RANDOM_LO_HI(-value, value),
      RANDOM_LO_HI(-value, value));
  }
}

JVR_NAMESPACE_CLOSE_SCOPE