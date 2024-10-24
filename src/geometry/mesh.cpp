// Mesh
//----------------------------------------------

#include <cmath>
#include <algorithm>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/mesh.h>

#include "../geometry/mesh.h"
#include "../geometry/utils.h"
#include "../geometry/location.h"
#include "../geometry/intersection.h"

#include "../utils/timer.h"


JVR_NAMESPACE_OPEN_SCOPE


Mesh::Mesh(const GfMatrix4d& xfo)
  : Deformable(Geometry::MESH, xfo)
  , _flags(0)
  , _halfEdges()
{
}

Mesh::Mesh(const UsdGeomMesh& mesh, const GfMatrix4d& world, size_t connectivity)
  : Deformable(mesh.GetPrim(), world)
  , _flags(0)
  , _halfEdges()
{
  UsdAttribute pointsAttr = mesh.GetPointsAttr();
  UsdAttribute faceVertexCountsAttr = mesh.GetFaceVertexCountsAttr();
  UsdAttribute faceVertexIndicesAttr = mesh.GetFaceVertexIndicesAttr();

  pointsAttr.Get(&_positions, 1);
  faceVertexCountsAttr.Get(&_faceVertexCounts, 1);
  faceVertexIndicesAttr.Get(&_faceVertexIndices, 1);
  Init(connectivity);
}

Mesh::~Mesh()
{
}

size_t Mesh::GetFaceVertexIndex(uint32_t face, uint32_t vertex)
{
  size_t accum = 0;
  for(size_t i=0; i < face; ++i)accum += _faceVertexCounts[i];
  return _faceVertexIndices[accum + vertex];
}

void Mesh::GetCutVerticesFromUVs(const VtArray<GfVec2d>& uvs, VtArray<int>* cuts)
{
  size_t numVertices = _positions.size();

  VtArray<int> visited(numVertices);
  for (auto& v : visited) v = 0;
  
  VtArray<GfVec2d> uvPositions(numVertices);
  
  size_t uvIndex = 0;
  for (const int& faceVertexIndex : _faceVertexIndices) {
    GfVec2d uv = uvs[uvIndex++];
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


void Mesh::GetCutEdgesFromUVs(const VtArray<GfVec2d>& uvs, VtArray<int>* cuts)
{
  /*
  cuts->reserve(_uniqueEdges.size());
  for (const HalfEdge& halfEdge: _halfEdges) {
    if (halfEdge.latency == HalfEdge::REAL && halfEdge.twin) {
      const HalfEdge* next = &_halfEdges[halfEdge.next];
      const GfVec2d& start_lhs = uvs[halfEdge.index];
      const GfVec2d& end_lhs = uvs[next->index];
      const HalfEdge* twin = &_halfEdges[halfEdge.twin];
      const GfVec2d& start_rhs = uvs[twin->next];
      const GfVec2d& end_rhs = uvs[_halfEdges[twin->next].next];
      if (start_lhs != end_rhs) cuts->push_back(halfEdge.index);
      if (end_rhs != start_rhs) cuts->push_back(halfEdge.twin);
    }
  }
*/
}

GfVec3f 
Mesh::GetPosition(size_t idx) const
{
  return _positions[idx];
}

GfVec3f 
Mesh::GetTrianglePosition(const Triangle* T) const
{
  GfVec3f center(0.f);
  for(uint32_t v=0;v<3;v++) center += _positions[T->vertices[v]];
  center *= 1.f/3.f;
  return center;
}

GfVec3f 
Mesh::GetTriangleVertexPosition(const Triangle *T, uint32_t index) const
{
  return _positions[T->vertices[index]];
}

GfVec3f 
Mesh::GetTriangleNormal(const Triangle *T) const
{
  GfVec3f center(0.f);
  for(uint32_t v = 0; v < 3; ++v)
      center += _normals[T->vertices[v]];
  center *= 1.f/3.f;
  return center;
}

GfVec3f 
Mesh::GetTriangleVertexNormal(const Triangle *T, uint32_t index) const
{
  return _normals[T->vertices[index]];
}

GfVec3f 
Mesh::GetTriangleNormal(uint32_t triangleId) const
{
  const Triangle* T = &_triangles[triangleId];
  GfVec3f A = _positions[T->vertices[0]];
  GfVec3f B = _positions[T->vertices[1]];
  GfVec3f C = _positions[T->vertices[2]];

  B -= A;
  C -= A;

  return (B ^ C).GetNormalized();
}

GfVec3f Mesh::GetTriangleVelocity(uint32_t triangleId) const
{
  const Triangle* triangle = &_triangles[triangleId];
  return triangle->GetVelocity(&_positions[0], &_previous[0]);
}

bool Mesh::RemovePoint(size_t index)
{
  if (index >= _positions.size()) {
    std::cout << "error index out of range" << std::endl;
    return false;
  }
  _positions.erase(_positions.begin() + index);
  _normals.erase(_normals.begin() + index);

  return true;
}

void Mesh::ComputeHalfEdges()
{
  _halfEdges.ComputeGraph(this);
  BITMASK_SET(_flags, Mesh::HALFEDGES);
}

static size_t _TwinTriangleIndex(size_t polygonEdgeIdx, size_t triangleEdgeIdx, 
  size_t polygonNumEdges, size_t triangleIdx)
{
  if (polygonNumEdges < 4)return Component::INVALID_INDEX;
  if (polygonEdgeIdx == 0) {
    if (triangleEdgeIdx == 2)return triangleIdx + 1;
    else return Component::INVALID_INDEX;
  } else if (polygonEdgeIdx == polygonNumEdges - 1) {
    if (triangleEdgeIdx == 0)return triangleIdx - 1;
    else return Component::INVALID_INDEX;
  } else {
    if (triangleEdgeIdx == 0)return triangleIdx - 1;
    else if (triangleEdgeIdx == 2)return triangleIdx + 1;
    else return Component::INVALID_INDEX;
  }
}

float 
TrianglePairGraph::_ComputeEdgeBoundingBoxArea(const GfVec3f *positions, size_t vertex0, size_t vertex1) 
{
  const float width = GfAbs(positions[vertex0][0] - positions[vertex1][0]);
  const float height = GfAbs(positions[vertex0][1] - positions[vertex1][1]);
  const float depth = GfAbs(positions[vertex0][2] - positions[vertex1][2]);
  return 2.f * (width * height + width * depth + height * depth);
}

void 
TrianglePairGraph::_SingleTriangle(size_t tri)
{
  _paired[tri] = true;
  _pairs.push_back(std::make_pair(tri, Component::INVALID_INDEX));
}

bool 
TrianglePairGraph::_PairTriangles(size_t tri0, size_t tri1)
{
  if(_paired[tri0] || _paired[tri1]) return false;

  _paired[tri0] = true;
  _paired[tri1] = true;
  _pairs.push_back(std::make_pair(tri0, tri1));

  return true;
}

TrianglePairGraph::TrianglePairGraph(
  const VtArray<int>& faceVertexCounts, 
  const VtArray<int>& faceVertexIndices,
  const GfVec3f *positions)
{
  size_t numTriangles = 0;
  for(int faceVertexCount : faceVertexCounts)
    numTriangles += faceVertexCount - 2;

  _halfEdges.resize(numTriangles * 3);
  _paired.resize(numTriangles, false);

  size_t baseFaceVertexIdx = 0;
  size_t triangleIdx = 0;
  size_t triangleEdgeIdx = 0;
  size_t removeHalfEdges = 0;

  for (int faceVertexCount : faceVertexCounts) {
    if(faceVertexCount == 4) {
      _PairTriangles(triangleIdx, triangleIdx+1);
      triangleIdx += 2;
      removeHalfEdges += 6;
    } else {
      for (int i = 1; i < faceVertexCount - 1; ++i) {
        size_t a = faceVertexIndices[baseFaceVertexIdx        ];
        size_t b = faceVertexIndices[baseFaceVertexIdx + i    ];
        size_t c = faceVertexIndices[baseFaceVertexIdx + i + 1];

        _halfEdges[triangleEdgeIdx++] = a < b ? 
          _HalfEdge({_ComputeEdgeBoundingBoxArea(positions, a, b), a, b, triangleIdx}) :
          _HalfEdge({_ComputeEdgeBoundingBoxArea(positions, b, a), b, a, triangleIdx});

        _halfEdges[triangleEdgeIdx++] = b < c ? 
          _HalfEdge({_ComputeEdgeBoundingBoxArea(positions, b, c), b, c, triangleIdx}) :
          _HalfEdge({_ComputeEdgeBoundingBoxArea(positions, c, b), c, b, triangleIdx});

        _halfEdges[triangleEdgeIdx++] = c < a ? 
          _HalfEdge({_ComputeEdgeBoundingBoxArea(positions, c, a), c, a, triangleIdx}) :
          _HalfEdge({_ComputeEdgeBoundingBoxArea(positions, a, c), a, c, triangleIdx});

        triangleIdx++;
      }
    }

    baseFaceVertexIdx += faceVertexCount;
  }

  if(removeHalfEdges)_halfEdges.resize(_halfEdges.size() - removeHalfEdges);
  if(!_halfEdges.size())return;

  // sort triangle half edges lexicographicaly
  std::sort(_halfEdges.begin(), _halfEdges.end());

  size_t numTrianglesPaired = 0;

  for(size_t e = 0; e < _halfEdges.size() - 1; ++e)
  {
    if(_paired[_halfEdges[e].triangle]) continue;
    if(_halfEdges[e] == _halfEdges[e+1] && _PairTriangles(_halfEdges[e].triangle, _halfEdges[e+1].triangle))
      {numTrianglesPaired+=2;e++;}
    if(numTrianglesPaired == numTriangles)break;
  }

  if(numTrianglesPaired != numTriangles)
    for(size_t t = 0; t < numTriangles; ++t)
      if(!_paired[t]) {
        _paired[t] = true;
        _pairs.push_back(std::make_pair(t, Component::INVALID_INDEX));
      }
}

// custom comparator for triangle edges
struct
{
  inline bool operator() (const GfVec4i& e1, const GfVec4i& e2)
  {
    return e1[0] < e2[0] || (e1[0] == e2[0] && e1[1] < e2[1]);
  }
} _CompareTriangleEdge;

void  _FindTriangleNeighbors(const VtArray<Triangle>& triangles, VtArray<int>& neighbors)
{
  VtArray<GfVec4i> edges;
  const size_t numTriangles = triangles.size();
  int id0, id1;
  for (size_t i = 0; i < numTriangles; ++i) {
    const Triangle& triangle = triangles[i];
    for (size_t j = 0; j < 3; ++j) {
      id0 = triangle.vertices[j];
      id1 = triangle.vertices[(j + 1) % 3];
      edges.push_back({GfMin(id0, id1), GfMax(id0, id1), (int)(3 * i + j), (int)triangle.id});
    }
  }

  int invalidIdx = Component::INVALID_INDEX;
  std::sort(edges.begin(), edges.end(), _CompareTriangleEdge);
  neighbors.assign(3 * numTriangles, invalidIdx);

  size_t i = 0;
  while (i < edges.size()) {
    const GfVec4i& e0 = edges[i];
    const GfVec4i& e1 = edges[i + 1];

    if (e0[0] == e1[0] && e0[1] == e1[1]) {
      neighbors[e0[2]] = e1[3];
      neighbors[e1[2]] = e0[3];
    }
    i ++;
  }
}

void Mesh::ComputeTrianglePairs()
{
  TrianglePairGraph graph(_faceVertexCounts, _faceVertexIndices, &_positions[0]);
  _trianglePairs.clear();
  uint32_t triPairId = 0;
  for(auto& triPair: graph.GetPairs()) {
    _trianglePairs.push_back({ triPairId++, &_triangles[triPair.first],
       triPair.second != Component::INVALID_INDEX ? &_triangles[triPair.second] : NULL});
  }

  BITMASK_SET(_flags, Mesh::TRIANGLEPAIRS);
      
}

VtArray<TrianglePair>& 
Mesh::GetTrianglePairs()
{
  if (!(_flags & Mesh::TRIANGLEPAIRS)) {
    ComputeTrianglePairs();
  }
  return _trianglePairs;
}


size_t
Mesh::GetNumAdjacents(size_t index) const
{
  return _halfEdges.GetNumAdjacents(index);
}

size_t
Mesh::GetTotalNumAdjacents() const
{
  return _halfEdges.GetTotalNumAdjacents();
}

const int* 
Mesh::GetAdjacents(size_t index) const
{
  return _halfEdges.GetAdjacents(index);
}

int 
Mesh::GetAdjacent(size_t index, size_t adjacent) const
{
  return _halfEdges.GetAdjacent(index, adjacent);
}

int 
Mesh::GetAdjacentIndex(size_t index, size_t adjacent) const
{
  return _halfEdges.GetAdjacentIndex(index, adjacent);
}

void 
Mesh::ComputeAdjacents()
{
  if (!BITMASK_CHECK(_flags, Mesh::HALFEDGES)) {
    ComputeHalfEdges();
  }
  _halfEdges.ComputeAdjacents();
  BITMASK_SET(_flags, Mesh::ADJACENTS);
}

size_t
Mesh::GetNumNeighbors(size_t index) const
{
  return _halfEdges.GetNumNeighbors(index);
}

size_t
Mesh::GetTotalNumNeighbors() const
{
  return _halfEdges.GetTotalNumNeighbors();
}

const int* 
Mesh::GetNeighbors(size_t index) const
{
  return _halfEdges.GetNeighbors(index);
}

int 
Mesh::GetNeighbor(size_t index, size_t neighbor) const
{
  return _halfEdges.GetNeighbor(index, neighbor);
}

int 
Mesh::GetNeighborIndex(size_t index, size_t neighbor) const
{
  return _halfEdges.GetNeighborIndex(index, neighbor);
}

void Mesh::ComputeNeighbors()
{
  if (!BITMASK_CHECK(_flags, Mesh::HALFEDGES)) {
    ComputeHalfEdges();
  }
  _halfEdges.ComputeNeighbors();
  BITMASK_SET(_flags, Mesh::NEIGHBORS);
}

float 
Mesh::ComputeAverageEdgeLength()
{
  if (!BITMASK_CHECK(_flags, Mesh::HALFEDGES)) {
    ComputeHalfEdges();
  }

  double sum = 0.0;
  int cnt = 0;
  HalfEdgeGraph::ItUniqueEdge it(_halfEdges);
  HalfEdge* edge = it.Next();
  while(edge) {
    sum += _halfEdges.GetLengthSq(edge, &_positions[0]);
    cnt++;
    edge = it.Next();
  }
  return cnt > 0 ? static_cast<float>(GfSqrt(sum) / (double)cnt) : 0.f;
}


static inline float _CotangentWeight(float x)
{
  static const float cotanMax = GfCos( 1.e-6 ) / GfSin( 1.e-6 );
  //return GfCos(x)/GfSin(x);
  float cotan = GfCos(x)/GfSin(x);
  return cotan < -cotanMax ? -cotanMax : cotan > cotanMax ? cotanMax : cotan;
}

void 
Mesh::ComputeCotangentWeights(MeshCotangentWeights& weights)
{
  if(!_flags & Mesh::ADJACENTS)
    ComputeAdjacents();
    
  const GfVec3f *positions = GetPositionsCPtr();
  size_t numPoints = _halfEdges.GetNumVertices();

  weights.values.resize(_halfEdges.GetTotalNumAdjacents() + numPoints);
  weights.offsets.resize(numPoints);

  int i0, i1, i2, i3;
  size_t k, na, offset = 0;
  float alpha, beta, cotan;

  for (i0 = 0; i0 < numPoints; ++i0) {

    float sum = 0.f;
    na = _halfEdges.GetNumAdjacents(i0);

    for(size_t k = 0; k < na; ++k) {
      i1 = _halfEdges.GetAdjacent(i0, k);

      i2 = _halfEdges.GetAdjacent(i0, k > 0 ? k - 1 : na - 1);
      i3 = _halfEdges.GetAdjacent(i0, (k+1) % na);

      const GfVec3f& p0 = positions[i0];
      const GfVec3f& p1 = positions[i1];
      const GfVec3f& p2 = positions[i2];
      const GfVec3f& p3 = positions[i3];

      // compute the vectors in order to compute the triangles
      GfVec3f v0(p2 - p0);
      GfVec3f v1(p2 - p1);
      GfVec3f v2(p3 - p0);
      GfVec3f v3(p3 - p1);

      // compute alpha and beta
      alpha = acos((v0*v1) / GfSqrt(v0.GetLengthSq() * v1.GetLengthSq()));
      beta = acos((v2*v3) / GfSqrt(v2.GetLengthSq() * v3.GetLengthSq()));
      cotan = (_CotangentWeight(alpha) + _CotangentWeight(beta)) * 0.5f;
      
      weights.values[offset + k] = cotan;
      sum += cotan;
      
    }

    weights.values[offset + na] = -sum;
    weights.offsets[i0] = offset;
    offset += na + 1;
  }
}


void
Mesh::ComputeAreas(MeshAreas& result)
{
  if(!BITMASK_CHECK(_flags, Mesh::ADJACENTS))
    ComputeAdjacents();
  const GfVec3f *positions = GetPositionsCPtr();

  size_t numPoints = _positions.size();
  size_t numFaces = _faceVertexCounts.size();
  result.face.resize(numFaces);
  result.vertex.resize(numPoints);

  result.faceInfos = GfVec4f(0.f, FLT_MAX, FLT_MIN, 0.f);
  result.vertexInfos = GfVec4f(0.f, FLT_MAX, FLT_MIN, 0.f);

  size_t numHalfEdges = _halfEdges.GetNumRawEdges();
  std::vector<bool> visited(numHalfEdges, false);

  for(size_t i = 0; i < numHalfEdges; ++i) {
    const HalfEdge* start = _halfEdges.GetEdge(i);
    if(!start || visited[i])continue;

    const HalfEdge* edge = _halfEdges.GetEdge(start->next);
    visited[i] = true;
    visited[start->next] = true;
    float area = 0.f;
    while(edge && edge->next != i) {
      visited[edge->next] = true;
      const HalfEdge* next = _halfEdges.GetEdge(edge->next);
      Triangle triangle(Component::INVALID_INDEX, 
        GfVec3i(start->vertex, edge->vertex, next->vertex));
      area += triangle.GetArea(positions);
      edge = next;
    }

    result.face[start->face] = area;
    result.faceInfos[0] += area;
    if(area < result.faceInfos[1])result.faceInfos[1] = area;
    if(area > result.faceInfos[2])result.faceInfos[2] = area;
  }

  result.faceInfos[3] = result.faceInfos[0] / static_cast<float>(numFaces);

  for(size_t i = 0; i < numPoints; ++i) {
    const HalfEdge* start = _halfEdges.GetEdgeFromVertex(i);

    float area = result.face[start->face];
    size_t numFaces = 0;
    const HalfEdge* edge = _halfEdges.GetNextAdjacentEdge(start);
    while(edge && edge != start) {
      area += result.face[edge->face];
      edge = _halfEdges.GetNextAdjacentEdge(edge);
      numFaces++;
    }
    if(edge != start) {
      edge = _halfEdges.GetPreviousAdjacentEdge(start);
      while(edge) {
        area += result.face[edge->face];
        edge = _halfEdges.GetPreviousAdjacentEdge(edge);
        numFaces++;
      }
    }
    area /= static_cast<float>(numFaces);
    result.vertex[i] = area;

    result.vertexInfos[0] += area;
    if(area < result.vertexInfos[1])result.vertexInfos[1] = area;
    if(area > result.vertexInfos[2])result.vertexInfos[2] = area;
    
  }

  result.vertexInfos[3] = result.vertexInfos[0] / static_cast<float>(numPoints);
}


void 
Mesh::Set(
  const VtArray<GfVec3f>& positions,
  const VtArray<int>& faceVertexCounts,
  const VtArray<int>& faceVertexIndices,
  bool init, 
  size_t connectivity
)
{
  _faceVertexCounts = faceVertexCounts;
  _faceVertexIndices = faceVertexIndices;
  _positions = positions;
  _previous = _positions;
  _normals = positions;
  if(init)Init(connectivity);
}

void Mesh::SetTopology(
  const VtArray<int>& faceVertexCounts,
  const VtArray<int>& faceVertexIndices,
  bool init,
  size_t connectivity
)
{
  _faceVertexCounts = faceVertexCounts;
  _faceVertexIndices = faceVertexIndices;

  if(init)Init(connectivity);
}

void Mesh::SetPositions(const GfVec3f* positions, size_t n)
{
  if(n == GetNumPoints()) {
    memcpy(&_positions[0], positions, n * sizeof(GfVec3f));
    // recompute normals
    ComputeVertexNormals(_positions, _faceVertexCounts, 
    _faceVertexIndices, _triangles, _normals);
  }
}

void Mesh::SetPositions(const VtArray<GfVec3f>& positions)
{
  const size_t n = positions.size();
  if(n == GetNumPoints()) {
    _positions = positions;
    // recompute normals
    ComputeVertexNormals(_positions, _faceVertexCounts, 
    _faceVertexIndices, _triangles, _normals);
  }
}

void Mesh::Init(size_t connectivity)
{
  size_t numPoints = _positions.size();
  // compute triangles
  TriangulateMesh(_faceVertexCounts, _faceVertexIndices, _triangles);
  // compute normals
  ComputeVertexNormals(_positions, _faceVertexCounts, 
    _faceVertexIndices, _triangles, _normals);
  // compute bouding box
  ComputeBoundingBox();

  // compute connectivity if required
  //    - adjacents = vertex connected vertices
  //    - neighbors = vertex first ring vertices
  switch(connectivity) {
    case Mesh::ADJACENTS:
      ComputeHalfEdges();
      ComputeAdjacents();
      BITMASK_SET(_flags,Mesh::HALFEDGES|Mesh::ADJACENTS);
      break;

    case Mesh::NEIGHBORS:
      ComputeHalfEdges();
      ComputeNeighbors();
      BITMASK_SET(_flags,Mesh::HALFEDGES|Mesh::NEIGHBORS);
      break;

    case Mesh::ADJACENTS|Mesh::NEIGHBORS:
      ComputeHalfEdges();
      ComputeAdjacents();
      ComputeNeighbors();
      BITMASK_SET(_flags, Mesh::HALFEDGES|Mesh::NEIGHBORS|Mesh::ADJACENTS);
      break;

    default:
      BITMASK_CLEAR(_flags, Mesh::HALFEDGES | Mesh::NEIGHBORS | Mesh::ADJACENTS);
      break;
  }
}

Geometry::DirtyState 
Mesh::_Sync(const GfMatrix4d& matrix, const UsdTimeCode& time)
{
  if(_prim.IsValid() && _prim.IsA<UsdGeomMesh>())
  {
    UsdGeomMesh usdMesh(_prim);
    if(_previous.size() == _positions.size())
      memcpy(&_previous[0], &_positions[0], _positions.size() * sizeof(GfVec3f));
    else
      usdMesh.GetPointsAttr().Get(&_previous, time);

    usdMesh.GetPointsAttr().Get(&_positions, time);

    // recompute normals if needed
    if(VtValue(_positions).GetHash() != VtValue(_previous).GetHash()) {
      ComputeVertexNormals(_positions, _faceVertexCounts, 
        _faceVertexIndices, _triangles, _normals);
    
      return Geometry::DirtyState::DEFORM;
    }
  }
  return Geometry::DirtyState::CLEAN;
}

void 
Mesh::_Inject(const GfMatrix4d& parent,
    const UsdTimeCode& time)
{
  if(_prim.IsA<UsdGeomMesh>()) {
    UsdGeomMesh usdMesh(_prim);

    usdMesh.CreatePointsAttr().Set(GetPositions(), time);
    usdMesh.CreateFaceVertexCountsAttr().Set(GetFaceCounts(), time);
    usdMesh.CreateFaceVertexIndicesAttr().Set(GetFaceConnects(), time);
  }
}

static int 
_CountPointCuts(const std::vector<bool>& doCutEdge, HalfEdge* start) {
  HalfEdge* current = start;

  int count = 0;
  /*
  bool search = true;
  while (search) {
    if (current->twin != HalfEdge::INVALID_INDEX) {
      if (doCutEdge[current->index] && doCutEdge[_halfEdges[current->twin]]) {
        count++;
      }
      current = current->twin->next;
    } else {
      search = false;
    }
    if (current == start)search = false;
  }
  current = start;
  
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

bool Mesh::FlipEdge(HalfEdge* edge)
{
  return _halfEdges.FlipEdge(edge);
}

bool Mesh::SplitEdge(HalfEdge* edge)
{
  if (!edge) edge = _halfEdges.GetEdge(0);

  size_t edgeIdx = _halfEdges._GetEdgeIndex(edge);

  const HalfEdge* next = _halfEdges.GetEdge(edge->next);
  size_t numPoints = GetNumPoints();
  const GfVec3f p =
    (_positions[edge->vertex] + _positions[next->vertex]) * 0.5f;

  AddPoint(p, 0.01f);
  
  if (_halfEdges.SplitEdge(edge, numPoints)) {
    // reallocation mess pointer retrieve edge
    edge = _halfEdges.GetEdge(edgeIdx);
    TriangulateFace(_halfEdges.GetEdge(edge->next));
    if (edge->twin != HalfEdge::INVALID_INDEX) {
      TriangulateFace(_halfEdges.GetEdge(edge->twin));
    }
    _halfEdges.ComputeTopology(_faceVertexCounts, _faceVertexIndices);
    return true;
  }
  return false;
}


bool Mesh::CollapseEdge(HalfEdge* edge)
{
  const HalfEdge* next = _halfEdges.GetEdge(edge->next);
  size_t p1 = edge->vertex;
  size_t p2 = next->vertex;

  if (_halfEdges.CollapseEdge(edge)) {
    if (p1 > p2) {
      _positions[p2] = (_positions[p1] + _positions[p2]) * 0.5f;
      RemovePoint(p1);
    }
    else {
      _positions[p1] = (_positions[p1] + _positions[p2]) * 0.5f;
      RemovePoint(p2);
    }
  }
  return true;
}

void Mesh::DisconnectEdges(const VtArray<int>& cutEdges)
{

}

void Mesh::Flatten(const VtArray<GfVec2d>& uvs, 
  const TfToken& interpolation)
{
  if (interpolation == UsdGeomTokens->vertex) {
    for (size_t i = 0; i < uvs.size(); ++i) {
      const GfVec2d& uv = uvs[i];
      _positions[i] = GfVec3f(uv[0], 0.f, uv[1]);
    }
  } else if (interpolation == UsdGeomTokens->faceVarying) {
    for (size_t i = 0; i < uvs.size(); ++i) {
      const GfVec2d& uv = uvs[i];
      _positions[_faceVertexIndices[i]] = GfVec3f(uv[0], 0.f, uv[1]);
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
  GfVec3f A = GetTriangleVertexPosition(T, 0);
  GfVec3f B = GetTriangleVertexPosition(T, 1);
  GfVec3f C = GetTriangleVertexPosition(T, 2);
  GfVec3f AB = B - A;
  GfVec3f AC = C - A;
  GfVec3f BC = C - B;

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

void Mesh::Triangulate()
{
  size_t numTriangles = _triangles.size();
  VtArray<int> faceVertexCount(numTriangles, 3);
  VtArray<int> faceVertexConnect(numTriangles * 3);
  for (size_t t = 0; t < numTriangles; ++t) {
    memcpy(&faceVertexConnect[t * 3], &_triangles[t].vertices[0], sizeof(GfVec3i));
  }
  SetTopology(faceVertexCount, faceVertexConnect);
}

void Mesh::TriangulateFace(const HalfEdge* edge)
{
  size_t numFaceVertices = _halfEdges._GetFaceVerticesCount(edge);
  if (numFaceVertices < 4) {
    std::cerr << "[Mesh] can't triangulate face with " << numFaceVertices << 
      "vertices" << std::endl;
    return;
  }

  _halfEdges._TriangulateFace(edge);
}

void Mesh::GetAllTrianglePairs(VtArray<TrianglePair>& pairs, bool unique)
{
  VtArray<int> neighbors;
  _FindTriangleNeighbors(_triangles, neighbors);
  std::vector<bool> used(_triangles.size(), false);
  uint32_t triPairId = 0;

  if(unique) {
    bool found;
    for(size_t i = 0; i< neighbors.size();i+=3) {

      found = false;
      for(size_t j=0; j < 3; ++j) {
        if(neighbors[i+j] == Component::INVALID_INDEX) continue;
        if(used[neighbors[i+j]])continue;
        if(!used[i/3] && !used[neighbors[i+j]]) {
          pairs.push_back({ triPairId++, &_triangles[i/3], &_triangles[neighbors[i+j]]});
          used[i/3] = true;
          used[neighbors[i+j]] = true;
          found = true;
          break;
        }
        if(!found){
            pairs.push_back({triPairId++, &_triangles[i/3], NULL});
            used[i/3] = true;
        }
      }
    }
  } else 
    for(size_t i = 0; i< neighbors.size();i+=3) 
      for(size_t j=0; j < 3; ++j)
        if(neighbors[i+j] == Component::INVALID_INDEX) continue;
        else if(i/3 < neighbors[i+j]){
          pairs.push_back({ triPairId++, &_triangles[i/3], &_triangles[neighbors[i+j]]});
        }
  /*
  VtArray<int> edgeTriIndex(_halfEdges.GetNumRawEdges(), -1);

  int triangleIdx = -1;
  int edgeIdx = 0;
  for(auto& faceVertexCount: _faceVertexCounts) {
    for(int vertexIdx = 0; vertexIdx < faceVertexCount; ++vertexIdx) {
      if(vertexIdx == 0) {
        edgeTriIndex[edgeIdx++] = triangleIdx + 1;
      } else if(vertexIdx == (faceVertexCount-1)) {
        edgeTriIndex[edgeIdx++] = triangleIdx;
      } else {
        edgeTriIndex[edgeIdx++] = ++triangleIdx;
      }
    }
  }

  uint32_t triPairIdx = 0;
  triangleIdx = 0;
  edgeIdx = 0;
  for(auto& faceVertexCount: _faceVertexCounts) {
    for(int vertexIdx = 1; vertexIdx < (faceVertexCount - 1); ++vertexIdx) {
      
      HalfEdge* e = _halfEdges.GetEdge(edgeIdx + vertexIdx);
      HalfEdge* p = _halfEdges.GetEdge(e->prev);
      HalfEdge* n = _halfEdges.GetEdge(e->next);

      Triangle* t = &_triangles[triangleIdx++];
      
      if(e->twin >= 0) {
        Triangle* o = &_triangles[edgeTriIndex[e->twin]];
        if(t->id < o->id) {
          pairs.push_back({triPairIdx++, t, o});
        }
      }

      Triangle* lo = &_triangles[edgeTriIndex[_halfEdges.GetEdgeIndex(p)]];
      if(t->id == lo->id && p->twin >= 0) {
        lo = &_triangles[edgeTriIndex[p->twin]];
        if(t->id < lo->id) {
          pairs.push_back({triPairIdx++, t, lo});
        }
      } else if(t->id < lo->id) {
        pairs.push_back({triPairIdx++, t, lo});
      }

      Triangle* ro = &_triangles[edgeTriIndex[_halfEdges.GetEdgeIndex(n)]];
      if(t->id == ro->id && n->twin >= 0) {
        ro = &_triangles[edgeTriIndex[n->twin]];
        if(t->id < ro->id) {
          pairs.push_back({triPairIdx++, t, ro});
        }
      } else if(t->id < ro->id) {
        pairs.push_back({triPairIdx++, t, ro});
      }
    }
    edgeIdx += faceVertexCount;
  }
  */
}

bool Mesh::ClosestIntersection(const GfVec3f& origin, 
  const GfVec3f& direction, Location& result, float maxDistance)
{
  GfRay ray(origin, direction);

  GfVec3d p0, p1, p2;
  GfVec3d baryCoords;
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
        result.SetCoordinates(GfVec3f(baryCoords));
        result.SetComponentIndex(tri->id);
        hit = true;
      }
    }
  }
  return hit;
}

void Mesh::PolygonSoup(size_t numPolygons, const GfVec3f& minimum, 
  const GfVec3f& maximum)
{
  size_t numTriangles = numPolygons;
  size_t numPoints = numPolygons * 3;
  VtArray<GfVec3f> position(numPoints);

  float radius = (maximum - minimum).GetLength() / 10.f;
  for(size_t i=0; i < numPolygons; ++i) {
    float bx = RANDOM_0_1;
    float by = RANDOM_0_1;
    float bz = RANDOM_0_1;
    GfVec3f center(
      minimum[0] * (1.f - bx) + maximum[0] * bx,
      minimum[1] * (1.f - by) + maximum[1] * by,
      minimum[2] * (1.f - bz) + maximum[2] * bz
    );

    GfVec3f normal(
      RANDOM_LO_HI(-1.f, 1.f),
      RANDOM_LO_HI(-1.f, 1.f),
      RANDOM_LO_HI(-1.f, 1.f)
    );

    normal.Normalize();

    position[i * 3 + 0] = center + GfVec3f(1.f, 0.f, 0.f);
    position[i * 3 + 1] = center + GfVec3f(0.f, 1.f, 0.f);
    position[i * 3 + 2] = center + GfVec3f(0.f, 0.f, 1.f);
  }

  VtArray<int> faceVertexCount(numTriangles);
  for(size_t i=0; i < numTriangles; ++i) {
    faceVertexCount[i] = 3;
  }

  VtArray<int> faceVertexConnect(numPoints);
  for(size_t i=0; i < numPoints; ++i) {
    faceVertexConnect[i] = i;
  }
  Set(position, faceVertexCount, faceVertexConnect);
}


void Mesh::ColoredPolygonSoup(size_t numPolygons, 
  const GfVec3f& minimum, const GfVec3f& maximum)
{
  //mesh->PolygonSoup(65535);
  GfMatrix4f space(1.f);
  TriangularGrid2D(0.05f, space);
  Randomize(0.05f);
}

Mesh* MakeOpenVDBSphere(UsdStageRefPtr& stage, const SdfPath& path)
{
  Mesh* mesh = new Mesh();
  
  mesh->OpenVDBSphere(6.66, GfVec3f(3.f, 7.f, 4.f));

  UsdGeomMesh vdbSphere = UsdGeomMesh::Define(stage, path);
  vdbSphere.CreatePointsAttr(VtValue(mesh->GetPositions()));
  vdbSphere.CreateNormalsAttr(VtValue(mesh->GetNormals()));
  vdbSphere.CreateFaceVertexIndicesAttr(VtValue(mesh->GetFaceConnects()));
  vdbSphere.CreateFaceVertexCountsAttr(VtValue(mesh->GetFaceCounts()));

  vdbSphere.CreateSubdivisionSchemeAttr(VtValue(UsdGeomTokens->none));

  std::cout << "CREATED OPENVDB SPHERE !!!" << std::endl;

  return mesh;
}


void Mesh::Random2DPattern(size_t numFaces)
{
  VtArray<GfVec3f> points(numFaces * 2 + 2);
  VtArray<int> faceCounts(numFaces * 2);
  VtArray<int> faceConnects(numFaces * 6);

  for (size_t face = 0; face < numFaces; ++face) {
    faceCounts[face * 2    ] = 3;
    faceCounts[face * 2 + 1] = 3;
    faceConnects[face * 6    ] = face * 2;
    faceConnects[face * 6 + 1] = face * 2 + 2;
    faceConnects[face * 6 + 2] = face * 2 + 1;
    faceConnects[face * 6 + 3] = face * 2 + 2;
    faceConnects[face * 6 + 4] = face * 2 + 3;
    faceConnects[face * 6 + 5] = face * 2 + 1;

    points[face * 2] = GfVec3f(face, 0, RANDOM_0_1);
    points[face * 2 + 1] = GfVec3f(face, 0, -RANDOM_0_1);
  }

  points[numFaces * 2] = GfVec3f(numFaces, 0, RANDOM_0_1);
  points[numFaces * 2 + 1] = GfVec3f(numFaces, 0, -RANDOM_0_1);

  Set(points, faceCounts, faceConnects);
}

void Mesh::RegularGrid2D(float spacing, const GfMatrix4f& matrix)
{
  size_t num = (1.f / spacing ) * 0.5 + 1;
  size_t numPoints = num * num;
  size_t numPolygons = (num - 1) * (num - 1);
  size_t numSamples = numPolygons * 4;
  VtArray<GfVec3f> positions(numPoints);

  float space = 1.f / static_cast<float>(num);

  for(size_t y = 0; y < num; ++y) {
    for(size_t x = 0; x < num; ++x) {
      size_t vertexId = y * num + x;
      positions[vertexId][0] = x * space - 0.5f;
      positions[vertexId][1] = 0.f;
      positions[vertexId][2] = y * space - 0.5f;
      positions[vertexId] = matrix.Transform(positions[vertexId]);
    }
  }
  
  VtArray<int> faceCounts(numPolygons);
  for(size_t i=0; i < numPolygons; ++i) {
    faceCounts[i] = 4;
  }

  size_t numRows = num - 1;
  size_t numPolygonPerRow = (num - 1) ;
  VtArray<int> faceIndices(numSamples);
  
  size_t k = 0;
  for(size_t i=0; i < numRows; ++i) {
    for (size_t j = 0; j < numPolygonPerRow; ++j) {
      faceIndices[k++] = (i + 1) * num + j + 1; 
      faceIndices[k++] = i * num + j + 1;
      faceIndices[k++] = i * num + j;
      faceIndices[k++] = (i + 1) * num + j; 
    }
  }
 
  Set(positions, faceCounts, faceIndices);
}

void Mesh::Cube()
{
  VtArray<GfVec3f> positions = {
    {-0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f},
    {-0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f},
    {-0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f},
    {-0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f}
  };

  VtArray<int> faceCounts = {
    4,4,4,4,4,4
  };

  VtArray<int> faceIndices = {
    0,2,3,1,4,5,7,6,
    0,4,6,2,1,3,7,5,
    0,1,5,4,2,6,7,3
  };

  Set(positions, faceCounts, faceIndices);
}

void Mesh::TriangularGrid2D(float spacing, const GfMatrix4f& matrix)
{
  size_t numX = ((1.f / spacing )+ 1) * 0.5f;
  size_t numY = ((1.f / spacing) + 1) * 0.5f;
  size_t numPoints = numX * numY;
  size_t numTriangles = (numX - 1) * 2 * (numY - 1);
  size_t numSamples = numTriangles * 3;
  VtArray<GfVec3f> positions(numPoints);

  float spaceX = spacing * 2.f;
  float spaceY = spacing * 2.f;

  for(size_t y = 0; y < numY; ++y) {
    for(size_t x = 0; x < numX; ++x) {
      size_t vertexId = y * numX + x;
      positions[vertexId][0] = spaceX * x - 0.5f /*- spaceX * (y % 2 +1)*/;
      positions[vertexId][1] = 0.f;
      positions[vertexId][2] = spaceY * y - 0.5f;
      positions[vertexId] = matrix.Transform(positions[vertexId]);
    }
  }
  
  VtArray<int> faceCounts(numTriangles);
  for(size_t i=0; i < numTriangles; ++i) {
    faceCounts[i] = 3;
  }

  size_t numRows = numY - 1;
  size_t numTrianglesPerRow = (numX - 1) ;
  VtArray<int> faceIndices(numSamples);
  
  size_t k = 0;
  for(size_t i=0; i < numRows; ++i) {
    for (size_t j = 0; j < numTrianglesPerRow; ++j) {
      if(i%2) {
        if (j % 2)  {
          faceIndices[k++] = (i + 1) * numX + j + 1; 
          faceIndices[k++] = i * numX + j + 1;
          faceIndices[k++] = i * numX + j;

          faceIndices[k++] = i * numX + j; 
          faceIndices[k++] = (i + 1) * numX + j;
          faceIndices[k++] = (i + 1) * numX + j + 1;
        }
        else {
          faceIndices[k++] = (i + 1) * numX + j; 
          faceIndices[k++] = i * numX + j + 1;
          faceIndices[k++] = i * numX + j;

          faceIndices[k++] = i * numX + j + 1; 
          faceIndices[k++] = (i + 1) * numX + j;
          faceIndices[k++] = (i + 1) * numX + j + 1;
        }
      } else {
        if (j % 2)  {
          faceIndices[k++] = (i + 1) * numX + j; 
          faceIndices[k++] = i * numX + j + 1;
          faceIndices[k++] = i * numX + j;

          faceIndices[k++] = i * numX + j + 1; 
          faceIndices[k++] = (i + 1) * numX + j;
          faceIndices[k++] = (i + 1) * numX + j + 1;
        }
        else {
          faceIndices[k++] = (i + 1) * numX + j + 1; 
          faceIndices[k++] = i * numX + j + 1;
          faceIndices[k++] = i * numX + j;

          faceIndices[k++] = i * numX + j; 
          faceIndices[k++] = (i + 1) * numX + j;
          faceIndices[k++] = (i + 1) * numX + j + 1;
        }
      }
      
    }
  }
  Set(positions, faceCounts, faceIndices);
}

void Mesh::OpenVDBSphere(float radius, const GfVec3f& center)
{

}

void Mesh::VoronoiDiagram(const std::vector<GfVec3f>& points)
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

  VtArray<GfVec3f> positions;
  VtArray<int> faceVertexCounts;
  VtArray<int> faceVertexConnects;
  VtArray<GfVec3f> colors;

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
    const GfVec3f color(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    size_t numFaceVertices = 0;
    if (halfEdge->origin != nullptr) {
      auto origin = (halfEdge->origin->point - center) + center;
      faceVertexConnects.push_back(positions.size());
      positions.push_back(GfVec3f(origin.x, 0.f, origin.y));
      colors.push_back(color);
      numFaceVertices++;
    }

    while (halfEdge != nullptr)
    {
      if (halfEdge->origin != nullptr && halfEdge->destination != nullptr)
      {
        auto destination = (halfEdge->destination->point - center) + center;
        faceVertexConnects.push_back(positions.size());
        positions.push_back(GfVec3f(destination.x, 0.f, destination.y));
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


void 
Mesh::Randomize(float value)
{
  for(auto& pos: _positions) {
    pos += GfVec3f(
      RANDOM_LO_HI(-value, value),
      RANDOM_LO_HI(-value, value),
      RANDOM_LO_HI(-value, value));
  }
}

// query 3d position on geometry
bool 
Mesh::Raycast(const GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{
  GfRay localRay(ray);

  localRay.Transform(_invMatrix);
  
  bool found(false);
  for(auto& pair: _trianglePairs) {
    Location localHit(*hit);
    if (pair.Raycast(&_positions[0], localRay, &localHit)) {

      const GfVec3d localPoint(localRay.GetPoint(localHit.GetDistance()));
      const double distance = (ray.GetStartPoint() - _matrix.Transform(localPoint)).GetLength();
      
      if ((distance < hit->GetDistance())) {
        hit->Set(localHit);
        hit->SetDistance(distance);
        if(minDistance)
          *minDistance = distance;
        found = true;
      }
    }
  }
  
  return found;
};

bool 
Mesh::Closest(const GfVec3f& point, Location* hit,
  double maxDistance, double* minDistance) const 
{
  size_t index = Component::INVALID_INDEX;

  size_t numTriangles = _triangles.size();

  bool found = false;

  Location closest;

  for(size_t t = 0; t < numTriangles; ++t)
    if (_triangles[t].Closest(&_positions[0], point, &closest)) {
      found = true;
    }

  if (!found)
    return false;

  hit->SetGeometryIndex(closest.GetGeometryIndex());
  hit->SetComponentIndex(closest.GetComponentIndex());
  hit->SetCoordinates(closest.GetCoordinates());

  double distance = (point - closest.GetPoint()).GetLength();

  if (minDistance)*minDistance = distance;
  return true;

};

JVR_NAMESPACE_CLOSE_SCOPE