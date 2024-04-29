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
#include "../geometry/location.h"
#include "../geometry/intersection.h"

#include "../utils/timer.h"


JVR_NAMESPACE_OPEN_SCOPE


Mesh::Mesh(const pxr::GfMatrix4d& xfo)
  : Deformable(Geometry::MESH, xfo)
  , _flags(0)
  , _halfEdges()
{
}

Mesh::Mesh(const pxr::UsdGeomMesh& mesh, const pxr::GfMatrix4d& world)
  : Deformable(Geometry::MESH, world)
  , _flags(0)
{
  pxr::UsdAttribute pointsAttr = mesh.GetPointsAttr();
  pxr::UsdAttribute faceVertexCountsAttr = mesh.GetFaceVertexCountsAttr();
  pxr::UsdAttribute faceVertexIndicesAttr = mesh.GetFaceVertexIndicesAttr();

  pointsAttr.Get(&_positions, pxr::UsdTimeCode::Default());
  faceVertexCountsAttr.Get(&_faceVertexCounts, pxr::UsdTimeCode::Default());
  faceVertexIndicesAttr.Get(&_faceVertexIndices, pxr::UsdTimeCode::Default());

  Prepare();
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
  /*
  cuts->reserve(_uniqueEdges.size());
  for (const HalfEdge& halfEdge: _halfEdges) {
    if (halfEdge.latency == HalfEdge::REAL && halfEdge.twin) {
      const HalfEdge* next = &_halfEdges[halfEdge.next];
      const pxr::GfVec2d& start_lhs = uvs[halfEdge.index];
      const pxr::GfVec2d& end_lhs = uvs[next->index];
      const HalfEdge* twin = &_halfEdges[halfEdge.twin];
      const pxr::GfVec2d& start_rhs = uvs[twin->next];
      const pxr::GfVec2d& end_rhs = uvs[_halfEdges[twin->next].next];
      if (start_lhs != end_rhs) cuts->push_back(halfEdge.index);
      if (end_rhs != start_rhs) cuts->push_back(halfEdge.twin);
    }
  }
*/
}

pxr::GfVec3f 
Mesh::GetPosition(size_t idx) const
{
  return _positions[idx];
}

pxr::GfVec3f 
Mesh::GetTrianglePosition(const Triangle* T) const
{
  pxr::GfVec3f center(0.f);
  for(uint32_t v=0;v<3;v++) center += _positions[T->vertices[v]];
  center *= 1.f/3.f;
  return center;
}

pxr::GfVec3f 
Mesh::GetTriangleVertexPosition(const Triangle *T, uint32_t index) const
{
  return _positions[T->vertices[index]];
}

pxr::GfVec3f 
Mesh::GetTriangleNormal(const Triangle *T) const
{
  pxr::GfVec3f center(0.f);
  for(uint32_t v = 0; v < 3; ++v)
      center += _normals[T->vertices[v]];
  center *= 1.f/3.f;
  return center;
}

pxr::GfVec3f 
Mesh::GetTriangleVertexNormal(const Triangle *T, uint32_t index) const
{
  return _normals[T->vertices[index]];
}

pxr::GfVec3f 
Mesh::GetTriangleNormal(uint32_t triangleID) const
{
  const Triangle* T = &_triangles[triangleID];
  pxr::GfVec3f A = _positions[T->vertices[0]];
  pxr::GfVec3f B = _positions[T->vertices[1]];
  pxr::GfVec3f C = _positions[T->vertices[2]];

  B -= A;
  C -= A;

  return (B ^ C).GetNormalized();
}

pxr::VtArray<HalfEdge>&
Mesh::GetEdges()
{
  return _halfEdges.GetEdges();
}

HalfEdgeGraph*
Mesh::GetEdgesGraph()
{
  return &_halfEdges;
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

float Mesh::GetAverageEdgeLength()
{
  const pxr::GfVec3f* positions = &_positions[0];
  float accum = 0;
  size_t cnt = 0;
  HalfEdgeGraph::ItUniqueEdge it(_halfEdges);
  HalfEdge* edge = it.Next();
  while(edge) {
    accum += _halfEdges.GetLength(edge, positions);
    cnt++;
    edge = it.Next();
  }
  return cnt > 0 ? accum / (float)cnt : 0.f;
}


static int _TwinTriangleIndex(size_t polygonEdgeIdx, size_t triangleEdgeIdx, 
  size_t polygonNumEdges, size_t triangleIdx)
{
  if (polygonNumEdges < 4)return -1;
  if (polygonEdgeIdx == 0) {
    if (triangleEdgeIdx == 2)return triangleIdx + 1;
    else return -1;
  } else if (polygonEdgeIdx == polygonNumEdges - 1) {
    if (triangleEdgeIdx == 0)return triangleIdx - 1;
    else return -1;
  } else {
    if (triangleEdgeIdx == 0)return triangleIdx - 1;
    else if (triangleEdgeIdx == 2)return triangleIdx + 1;
    else return -1;
  }
}




void TrianglePairGraph::_RemoveTriangleOpenEdges(int tri)
{
  size_t shift = 0;
  size_t numOpenEdges = _openEdges.size();
  for(size_t o = 0; o < numOpenEdges; ++o) {
    if(_openEdges[o].second == tri)shift++;
    if(shift>0 && (o + shift) < numOpenEdges) {
      _openEdges[o] = _openEdges[o+shift];
    } 
  }
  size_t size = numOpenEdges - shift;
  if(shift > 0)_openEdges.resize(size > 0 ? size : 0);
}

void TrianglePairGraph::_SingleTriangle(int tri, bool isOpenEdgeTriangle)
{
  _paired[tri] = true;
  _pairs.push_back(std::make_pair(tri, -1));

  if (isOpenEdgeTriangle) _RemoveTriangleOpenEdges(tri);
}

bool TrianglePairGraph::_PairTriangles(int tri0, int tri1, bool isOpenEdgeTriangle)
{
  if(_paired[tri0] || _paired[tri1])return false;

  _paired[tri0] = true;
  _paired[tri1] = true;
  _pairs.push_back(std::make_pair(tri0, tri1));

  if(isOpenEdgeTriangle) _RemoveTriangleOpenEdges(tri1);
  return true;
}

bool TrianglePairGraph::_PairTriangle(TrianglePairGraph::_Key common, int tri)
{
  if(_paired[tri])return false;

  int pairedTriangle = -1;
  size_t numOpenEdges = _openEdges.size();
  for(size_t o = 0; o < numOpenEdges; ++o) {
    uint64_t value = _openEdges[0].first; 
    uint64_t reversed = ((value & 0xffffffff) << 32) | (value>> 32);
    if(_openEdges[o].first == reversed) 
      return _PairTriangles(tri, _openEdges[0].second, true);
  }
  return false;
}

TrianglePairGraph::TrianglePairGraph(
  const pxr::VtArray<int>& faceVertexCounts, 
  const pxr::VtArray<int>& faceVertexIndices)
{
  int numTriangles = 0;
  for(int faceVertexCount : faceVertexCounts)numTriangles += faceVertexCount - 2;

  _allEdges.resize(numTriangles * 3);
  _paired.resize(numTriangles, false);

  int baseFaceVertexIdx = 0;
  int triangleIdx = 0;
  int trianglePairIdx;
  int triangleEdgeIdx = 0;

  TrianglePairGraph::_Key key0, key1, key2;

  for (int faceVertexCount : faceVertexCounts) {
    for (int i = 1; i < faceVertexCount - 1; ++i) {
      int a = faceVertexIndices[baseFaceVertexIdx        ];
      int b = faceVertexIndices[baseFaceVertexIdx + i    ];
      int c = faceVertexIndices[baseFaceVertexIdx + i + 1];

      key0 = { b | (a << 32), triangleIdx};
      key1 = { c | (b << 32), triangleIdx};
      key2 = { a | (c << 32), triangleIdx};

      _allEdges[triangleEdgeIdx++] = key0;
      _allEdges[triangleEdgeIdx++] = key1;
      _allEdges[triangleEdgeIdx++] = key2;
      if (faceVertexCount == 3) {
        _SingleTriangle(triangleIdx, false);
      }
      // we first pair by triangle by polygon if possible
      else if(((i-1) % 2) == 0 ) {
        _PairTriangles(triangleIdx, triangleIdx+1, false);
      } // else try pair with open edges from the list
      else {
        if(i < (faceVertexCount-2) && !_PairTriangle(key0, triangleIdx))
          _openEdges.push_back(key0);

        if(!_paired[triangleIdx] && !_PairTriangle(key1, triangleIdx))
          _openEdges.push_back(key1);

      }

      triangleIdx++;
    }

    baseFaceVertexIdx += faceVertexCount;
  }
}

void Mesh::ComputeTrianglePairs()
{
  TrianglePairGraph graph(_faceVertexCounts, _faceVertexIndices);
  _trianglePairs.clear();
  uint32_t triPairId = 0;
  for(auto& triPair: graph.GetPairs()) {
    _trianglePairs.push_back({ triPairId++, &_triangles[triPair.first],
       triPair.second >= 0 ? &_triangles[triPair.second] : NULL});
  }

  BITMASK_SET(_flags, Mesh::TRIANGLEPAIRS);
}

pxr::VtArray<TrianglePair>& 
Mesh::GetTrianglePairs()
{
  if (!(_flags & Mesh::TRIANGLEPAIRS)) {
    ComputeTrianglePairs();
  }
  return _trianglePairs;
}


const pxr::VtArray<pxr::VtArray<int>>& 
Mesh::GetNeighbors()
{
  
  if (!BITMASK_CHECK(_flags, Mesh::NEIGHBORS)) {
    _halfEdges.ComputeNeighbors();
  }
  return _halfEdges.GetNeighbors();
}

void Mesh::ComputeNeighbors()
{
  if (!BITMASK_CHECK(_flags, Mesh::HALFEDGES)) {
    ComputeHalfEdges();
  }
  _halfEdges.ComputeNeighbors();
  BITMASK_SET(_flags, Mesh::NEIGHBORS);
}

void Mesh::ComputeNeighbors(size_t pointIdx, pxr::VtArray<int>& neighbors)
{
  HalfEdge* edge = _halfEdges.GetEdgeFromVertex(pointIdx);
  _halfEdges._ComputeVertexNeighbors(edge, neighbors);
}

void Mesh::Set(
  const pxr::VtArray<pxr::GfVec3f>& positions,
  const pxr::VtArray<int>& faceVertexCounts,
  const pxr::VtArray<int>& faceVertexIndices,
  bool init
)
{
  _faceVertexCounts = faceVertexCounts;
  _faceVertexIndices = faceVertexIndices;
  _positions = positions;
  _previous = _positions;
  _normals = positions;
  if(init)Prepare();
}

void Mesh::SetTopology(
  const pxr::VtArray<int>& faceVertexCounts,
  const pxr::VtArray<int>& faceVertexIndices,
  bool init
)
{
  _faceVertexCounts = faceVertexCounts;
  _faceVertexIndices = faceVertexIndices;

  if(init)Prepare();
}

void Mesh::SetPositions(const pxr::GfVec3f* positions, size_t n)
{
  if(n == GetNumPoints()) {
    memcpy(&_positions[0], positions, n * sizeof(pxr::GfVec3f));
    // recompute normals
    ComputeVertexNormals(_positions, _faceVertexCounts, 
    _faceVertexIndices, _triangles, _normals);
  }
}

void Mesh::SetPositions(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  const size_t n = positions.size();
  if(n == GetNumPoints()) {
    _positions = positions;
    // recompute normals
    ComputeVertexNormals(_positions, _faceVertexCounts, 
    _faceVertexIndices, _triangles, _normals);
  }
}

void Mesh::Prepare(bool connectivity)
{
  size_t numPoints = _positions.size();
  // compute triangles
  TriangulateMesh(_faceVertexCounts, _faceVertexIndices, _triangles);

  // compute normals
  ComputeVertexNormals(_positions, _faceVertexCounts, 
    _faceVertexIndices, _triangles, _normals);

  // compute bouding box
  ComputeBoundingBox();

  if(connectivity) {
    // compute half-edges
    ComputeHalfEdges();

    // compute neighbors
    ComputeNeighbors();
  } else {
    BITMASK_CLEAR(_flags,Mesh::HALFEDGES|Mesh::NEIGHBORS);

  }
}

Geometry::DirtyState 
Mesh::_Sync(const pxr::UsdPrim& prim, const pxr::GfMatrix4d& matrix, const pxr::UsdTimeCode& time)
{
  if(prim.IsValid() && prim.IsA<pxr::UsdGeomMesh>())
  {
    _previous = _positions;
    pxr::UsdGeomMesh usdMesh(prim);
    const size_t nbPositions = _positions.size();
    usdMesh.GetPointsAttr().Get(&_positions, time);
  }
  return Geometry::DirtyState::DEFORM;
}

void 
Mesh::_Inject(pxr::UsdPrim& prim, const pxr::GfMatrix4d& parent,
    const pxr::UsdTimeCode& time)
{
  if(prim.IsA<pxr::UsdGeomMesh>()) {
    pxr::UsdGeomMesh usdMesh(prim);

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
  const pxr::GfVec3f p =
    (_positions[edge->vertex] + _positions[next->vertex]) * 0.5f;

  AddPoint(p, 0.01f);
  
  if (_halfEdges.SplitEdge(edge, numPoints)) {
    // reallocation mess pointer retrieve edge
    edge = _halfEdges.GetEdge(edgeIdx);
    TriangulateFace(_halfEdges.GetEdge(edge->next));
    if (edge->twin >= 0) {
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

void Mesh::DisconnectEdges(const pxr::VtArray<int>& cutEdges)
{

}

void Mesh::Flatten(const pxr::VtArray<pxr::GfVec2d>& uvs, 
  const pxr::TfToken& interpolation)
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

void Mesh::Triangulate()
{
  size_t numTriangles = _triangles.size();
  pxr::VtArray<int> faceVertexCount(numTriangles, 3);
  pxr::VtArray<int> faceVertexConnect(numTriangles * 3);
  for (size_t t = 0; t < numTriangles; ++t) {
    memcpy(&faceVertexConnect[t * 3], &_triangles[t].vertices[0], sizeof(pxr::GfVec3i));
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

void Mesh::GetAllTrianglePairs(pxr::VtArray<TrianglePair>& pairs)
{
  pxr::VtArray<int> edgeTriIndex(_halfEdges.GetNumRawEdges(), -1);

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
        result.SetCoordinates(pxr::GfVec3f(baryCoords));
        result.SetComponentIndex(tri->id);
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
  Set(position, faceVertexCount, faceVertexConnect);
}


void Mesh::ColoredPolygonSoup(size_t numPolygons, 
  const pxr::GfVec3f& minimum, const pxr::GfVec3f& maximum)
{
  //mesh->PolygonSoup(65535);
  pxr::GfMatrix4f space(1.f);
  TriangularGrid2D(0.05f, space);
  Randomize(0.05f);
}

Mesh* MakeOpenVDBSphere(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path)
{
  Mesh* mesh = new Mesh();
  
  mesh->OpenVDBSphere(6.66, pxr::GfVec3f(3.f, 7.f, 4.f));

  pxr::UsdGeomMesh vdbSphere = pxr::UsdGeomMesh::Define(stage, path);
  vdbSphere.CreatePointsAttr(pxr::VtValue(mesh->GetPositions()));
  vdbSphere.CreateNormalsAttr(pxr::VtValue(mesh->GetNormals()));
  vdbSphere.CreateFaceVertexIndicesAttr(pxr::VtValue(mesh->GetFaceConnects()));
  vdbSphere.CreateFaceVertexCountsAttr(pxr::VtValue(mesh->GetFaceCounts()));

  vdbSphere.CreateSubdivisionSchemeAttr(pxr::VtValue(pxr::UsdGeomTokens->none));

  std::cout << "CREATED OPENVDB SPHERE !!!" << std::endl;

  return mesh;
}


void Mesh::Random2DPattern(size_t numFaces)
{
  pxr::VtArray<pxr::GfVec3f> points(numFaces * 2 + 2);
  pxr::VtArray<int> faceCounts(numFaces * 2);
  pxr::VtArray<int> faceConnects(numFaces * 6);

  for (size_t face = 0; face < numFaces; ++face) {
    faceCounts[face * 2    ] = 3;
    faceCounts[face * 2 + 1] = 3;
    faceConnects[face * 6    ] = face * 2;
    faceConnects[face * 6 + 1] = face * 2 + 2;
    faceConnects[face * 6 + 2] = face * 2 + 1;
    faceConnects[face * 6 + 3] = face * 2 + 2;
    faceConnects[face * 6 + 4] = face * 2 + 3;
    faceConnects[face * 6 + 5] = face * 2 + 1;

    points[face * 2] = pxr::GfVec3f(face, 0, RANDOM_0_1);
    points[face * 2 + 1] = pxr::GfVec3f(face, 0, -RANDOM_0_1);
  }

  points[numFaces * 2] = pxr::GfVec3f(numFaces, 0, RANDOM_0_1);
  points[numFaces * 2 + 1] = pxr::GfVec3f(numFaces, 0, -RANDOM_0_1);

  Set(points, faceCounts, faceConnects);
}

void Mesh::RegularGrid2D(float spacing, const pxr::GfMatrix4f& matrix)
{
  size_t num = (1.f / spacing ) * 0.5 + 1;
  size_t numPoints = num * num;
  size_t numPolygons = (num - 1) * (num - 1);
  size_t numSamples = numPolygons * 4;
  pxr::VtArray<pxr::GfVec3f> positions(numPoints);

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
  
  pxr::VtArray<int> faceCounts(numPolygons);
  for(size_t i=0; i < numPolygons; ++i) {
    faceCounts[i] = 4;
  }

  size_t numRows = num - 1;
  size_t numPolygonPerRow = (num - 1) ;
  pxr::VtArray<int> faceIndices(numSamples);
  
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
  pxr::VtArray<pxr::GfVec3f> positions = {
    {-0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f},
    {-0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f},
    {-0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f},
    {-0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f}
  };

  pxr::VtArray<int> faceCounts = {
    4,4,4,4,4,4
  };

  pxr::VtArray<int> faceIndices = {
    0,2,3,1,4,5,7,6,
    0,4,6,2,1,3,7,5,
    0,1,5,4,2,6,7,3
  };

  Set(positions, faceCounts, faceIndices);
}

void Mesh::TriangularGrid2D(float spacing, const pxr::GfMatrix4f& matrix)
{
  const float scaleY = 1.5708;
  const float invScaleY = 1.f/scaleY;

  size_t numX = (1.f / spacing ) * 0.5 + 1;
  size_t numY = (1.f / spacing) * invScaleY + 1;
  size_t numPoints = numX * numY;
  size_t numTriangles = (numX - 1) * 2 * (numY - 1);
  size_t numSamples = numTriangles * 3;
  pxr::VtArray<pxr::GfVec3f> positions(numPoints);

  float spaceX = spacing * 2.0;
  float spaceY = spacing * scaleY;

  for(size_t y = 0; y < numY; ++y) {
    for(size_t x = 0; x < numX; ++x) {
      size_t vertexId = y * numX + x;
      if(y %2 == 0)positions[vertexId][0] = x * spaceX + spaceX * 0.5f;
      else positions[vertexId][0] = x * spaceX - 0.5f;
      positions[vertexId][1] = 0.f;
      positions[vertexId][2] = y * spaceY - 0.5f;
      positions[vertexId] = matrix.Transform(positions[vertexId]);
    }
  }
  
  pxr::VtArray<int> faceCounts(numTriangles);
  for(size_t i=0; i < numTriangles; ++i) {
    faceCounts[i] = 3;
  }

  size_t numRows = numY - 1;
  size_t numTrianglesPerRow = (numX - 1) ;
  pxr::VtArray<int> faceIndices(numSamples);
  
  size_t k = 0;
  for(size_t i=0; i < numRows; ++i) {
    for (size_t j = 0; j < numTrianglesPerRow; ++j) {
      if (i % 2 == 0) {
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
    }
  }
  Set(positions, faceCounts, faceIndices);
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

void 
Mesh::Randomize(float value)
{
  for(auto& pos: _positions) {
    pos += pxr::GfVec3f(
      RANDOM_LO_HI(-value, value),
      RANDOM_LO_HI(-value, value),
      RANDOM_LO_HI(-value, value));
  }
}

// query 3d position on geometry
bool 
Mesh::Raycast(const pxr::GfRay& ray, Location* hit,
  double maxDistance, double* minDistance) const
{
  const pxr::GfVec3f* positions = &_positions[0];
  pxr::GfRay localRay(ray);
  localRay.Transform(_invMatrix);

  Location localHit;
  localHit.SetGeometryIndex(0);
  for(const Triangle& triangle: _triangles)
    triangle.Raycast(positions, localRay, &localHit);

  bool success = false;
  if(localHit.IsValid()) {
    const Triangle* hitTri = &_triangles[localHit.GetComponentIndex()];
    const pxr::GfVec3f intersection = localHit.ComputePosition(positions, &hitTri->vertices[0], 3, &_matrix);
    const float distance = (ray.GetStartPoint() - intersection).GetLength();
    hit->Set(localHit);
    hit->SetT(distance);
    success = true;
  } 
  return success;
};

bool 
Mesh::Closest(const pxr::GfVec3f& point, Location* hit,
  double maxDistance, double* minDistance) const 
{
  return false;
};

JVR_NAMESPACE_CLOSE_SCOPE