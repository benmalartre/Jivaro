// HalfEdge
//----------------------------------------------
#include <algorithm>
#include <pxr/base/work/loops.h>
#include "../geometry/halfEdge.h"
#include "../geometry/triangle.h"
#include "../geometry/mesh.h"

#include "../utils/timer.h"


JVR_NAMESPACE_OPEN_SCOPE

HalfEdgeGraph::ItUniqueEdge::ItUniqueEdge(const HalfEdgeGraph& graph)
  : graph(graph)
  , index(HalfEdge::INVALID_INDEX)
{
}

HalfEdge* HalfEdgeGraph::ItUniqueEdge::Next()
{
  index++;
  while (true) {
    if (index >= graph._halfEdges.size())return NULL;
    HalfEdge* edge = (HalfEdge*)&graph._halfEdges[index];
    if (!graph.IsUsed(edge) || !graph.IsUnique(edge))
      index++;
    else return edge;
  }
}


void
HalfEdgeGraph::AllocateEdges(size_t num)
{
  size_t first = _halfEdges.size();
  _halfEdges.resize(first + num);
  _halfEdgeUsed.resize(first + num);
  for (size_t edgeIdx = first; edgeIdx < (first + num); ++edgeIdx) {
    HalfEdge* edge = &_halfEdges[edgeIdx];
    _halfEdgeUsed[edgeIdx] = false;
    _availableEdges.push(edgeIdx);
  }
}

float
HalfEdgeGraph::GetLength(const HalfEdge* edge, const pxr::GfVec3f* positions) const
{
  const HalfEdge* next = &_halfEdges[edge->next];
  return (positions[next->vertex] - positions[edge->vertex]).GetLength();
}

float
HalfEdgeGraph::GetLengthSq(const HalfEdge* edge, const pxr::GfVec3f* positions) const
{
  const HalfEdge* next = &_halfEdges[edge->next];
  return (positions[next->vertex] - positions[edge->vertex]).GetLengthSq();
}

HalfEdge*
HalfEdgeGraph::GetEdge(int index)
{
  return &_halfEdges[index];
}

const HalfEdge*
HalfEdgeGraph::GetEdge(int index) const
{
  return &_halfEdges[index];
}

HalfEdge*
HalfEdgeGraph::GetAvailableEdge()
{
  if (_availableEdges.empty())AllocateEdges(64);
  HalfEdge* edge = &_halfEdges[_availableEdges.front()];
  _availableEdges.pop();
  _halfEdgeUsed[_GetEdgeIndex(edge)] = true;
  return edge;
}

size_t
HalfEdgeGraph::GetNumRawEdges() const
{
  return _halfEdges.size();
}

size_t
HalfEdgeGraph::GetNumVertices() const
{
  return _vertexHalfEdge.size();
}

size_t
HalfEdgeGraph::GetNumEdges() const
{
  size_t numEdges = 0;
  for (auto& edge : _halfEdges) {
    size_t edgeIdx = _GetEdgeIndex(&edge);
    if (_halfEdgeUsed[edgeIdx] && IsUnique(&edge))numEdges++;
  }
  return numEdges;
}

HalfEdge*
HalfEdgeGraph::GetEdgeFromVertex(size_t vertex)
{
  return &_halfEdges[_vertexHalfEdge[vertex]];
}

HalfEdge* 
HalfEdgeGraph::GetEdgeFromVertices(size_t start, size_t end)
{
  HalfEdge* edge = &_halfEdges[_vertexHalfEdge[start]];
  if(_halfEdges[edge->next].vertex == end) return edge;

  HalfEdge* next = _GetNextAdjacentEdge(edge);

  while(next && next != edge) {
    if (_halfEdges[next->next].vertex == end)return next;
    next = _GetNextAdjacentEdge(next);
  }
  std::cout << "edge : " << edge << std::endl;
  HalfEdge* previous = _GetPreviousAdjacentEdge(edge);
  std::cout << "previous : " << previous << std::endl;
  while (previous && previous != edge) {
    if (_halfEdges[previous->next].vertex == end)return previous;
    previous = _GetPreviousAdjacentEdge(previous);
  }
  
  return NULL;
}

const HalfEdge*
HalfEdgeGraph::GetEdgeFromVertices(size_t start, size_t end) const
{
  return GetEdgeFromVertices(start, end);
}

void 
HalfEdgeGraph::GetEdgesFromFace(const HalfEdge* edge, pxr::VtArray<int>& indices)
{
  indices.push_back(_GetEdgeIndex(edge));
  const HalfEdge* current = &_halfEdges[edge->next];
  while (current != edge) {
    indices.push_back(_GetEdgeIndex(current));
    current = &_halfEdges[current->next];
  }
}

bool
HalfEdgeGraph::IsCollapsable(const HalfEdge* edge)
{
  if (edge->twin >= 0) {
    pxr::VtArray<int> edgeNeighbors;
    pxr::VtArray<int> twinNeighbors;
    size_t commonNeighbors = 0;
    const HalfEdge* twin = &_halfEdges[edge->twin];
    _ComputeVertexNeighbors(edge, edgeNeighbors);
    _ComputeVertexNeighbors(twin, twinNeighbors);

    for (auto& edgeNeighbor : edgeNeighbors)
      for (auto& twinNeighbor : twinNeighbors)
        commonNeighbors += (edgeNeighbor == twinNeighbor);

    if (commonNeighbors > 2) {
      return false;
    }
  }
  return true;
}

bool
HalfEdgeGraph::IsUnique(const HalfEdge* edge) const
{
  if (edge->twin < 0)return true;
  else return edge->vertex < _halfEdges[edge->next].vertex;
}

bool
HalfEdgeGraph::IsUsed(const HalfEdge* edge) const
{
  return _halfEdgeUsed[_GetEdgeIndex(edge)];
}

// custom comparator for sorting half edge by key
struct
{
  inline bool operator() (const HalfEdgeKey& edge1, const HalfEdgeKey& edge2)
  {
    return (edge1.first < edge2.first);
  }
} _SortEdgeByKey;

static HalfEdge*
_FindTwinEdge(const HalfEdgesKeys& halfEdgesKeys, uint64_t twinKey)
{
  int low = 0;
  int high = halfEdgesKeys.size();
  while (low < high) {
    int mid =  low + (high - low) / 2;
    if(halfEdgesKeys[mid].first == twinKey)
      return halfEdgesKeys[mid].second;

    if (twinKey <= halfEdgesKeys[mid].first) {
        high = mid;
    } else {
        low = mid + 1;
    }
  }
  return NULL;
}

void 
HalfEdgeGraph::ComputeGraph(Mesh* mesh)
{
  const size_t numPoints = mesh->GetNumPoints();
  const pxr::VtArray<int>& faceConnects = mesh->GetFaceConnects();
  const pxr::VtArray<int>& faceVertexCounts = mesh->GetFaceCounts();

  size_t numHalfEdges = 0;
  
  for (const auto& faceVertexCount : mesh->GetFaceCounts()) {
    numHalfEdges += faceVertexCount;
  }

  _halfEdges.resize(numHalfEdges);
  _halfEdgeUsed.resize(numHalfEdges);
  _halfEdgeFace.resize(numHalfEdges);

  size_t faceIdx = 0;
  size_t faceOffsetIdx = 0;
  size_t halfEdgeIdx = 0;

  HalfEdge* halfEdge = &_halfEdges[0];

  size_t halfEdgeIndex = 0;
  HalfEdgesKeys halfEdgesKeys;
  halfEdgesKeys.resize(numHalfEdges);

  _vertexHalfEdge.resize(numPoints);
  memset(&_vertexHalfEdge[0], HalfEdge::INVALID_INDEX, numPoints * sizeof(int));

  for (const auto& faceVertexCount: faceVertexCounts)
  { 
    size_t numFaceTriangles = faceVertexCount - 2;
    for (size_t faceEdgeIdx = 0; faceEdgeIdx < faceVertexCount; ++faceEdgeIdx) {
      // get vertices
      size_t v0 = faceConnects[faceOffsetIdx + faceEdgeIdx];
      size_t v1 = faceConnects[faceOffsetIdx + ((faceEdgeIdx + 1) % faceVertexCount)];
      if (_vertexHalfEdge[v0] < 0)_vertexHalfEdge[v0] = _GetEdgeIndex(halfEdge);

      size_t last = faceVertexCount - 1;
      halfEdgesKeys[halfEdgeIndex++] = {v1 | (v0 << 32), halfEdge};
      halfEdge->vertex = v0;
      if (!faceEdgeIdx) {
        halfEdge->prev = _GetEdgeIndex(halfEdge + last);
        halfEdge->next = _GetEdgeIndex(halfEdge + 1);
      } else if (faceEdgeIdx == last) {
        halfEdge->prev = _GetEdgeIndex(halfEdge - 1);
        halfEdge->next = _GetEdgeIndex(halfEdge - last);
      } else {
        halfEdge->prev = _GetEdgeIndex(halfEdge - 1);
        halfEdge->next = _GetEdgeIndex(halfEdge + 1);
      }

      _halfEdgeUsed[faceOffsetIdx + faceEdgeIdx] = true;
      _halfEdgeFace[faceOffsetIdx + faceEdgeIdx] = faceIdx;
      halfEdge++;
    }
    faceOffsetIdx += faceVertexCount;
    faceIdx++;
  }

  // sort the half-edges vector by edge key
  std::sort(halfEdgesKeys.begin(), halfEdgesKeys.end(), _SortEdgeByKey);

  _boundary.resize(numPoints, false);

  // populate the twin pointers by parallel processing the key vector:
  pxr::WorkParallelForEach(halfEdgesKeys.begin(), halfEdgesKeys.end(),
    [&](const HalfEdgeKey& halfEdge) {
      uint64_t edgeIndex = halfEdge.first;
      uint64_t twinIndex = ((edgeIndex & 0xffffffff) << 32) | (edgeIndex >> 32);
      HalfEdge* twinEdge = _FindTwinEdge(halfEdgesKeys, twinIndex);
      if(twinEdge)
      {
        twinEdge->twin = _GetEdgeIndex(halfEdge.second);
        halfEdge.second->twin = _GetEdgeIndex(twinEdge);
      }
      else {
        halfEdge.second->twin = HalfEdge::INVALID_INDEX;
        _boundary[halfEdge.second->vertex] = true;
        _boundary[_halfEdges[halfEdge.second->next].vertex] = true;
      }
  });
}

size_t 
HalfEdgeGraph::_GetEdgeIndex(const HalfEdge* edge) const
{
  return ((intptr_t)edge - (intptr_t)&_halfEdges[0]) / sizeof(HalfEdge);
}

size_t 
HalfEdgeGraph::GetEdgeIndex(const HalfEdge* edge) const
{
  return ((intptr_t)edge - (intptr_t)&_halfEdges[0]) / sizeof(HalfEdge);
}

size_t
HalfEdgeGraph::_GetFaceVerticesCount(const HalfEdge* edge)
{
  size_t faceVerticesCount = 1;
  const HalfEdge* current = _GetNextEdge(edge);
  while (current != edge) {
    faceVerticesCount++;
    current = &_halfEdges[current->next];
  }
  return faceVerticesCount;
}

size_t
HalfEdgeGraph::GetTotalNumNeighbors() const
{
  return _neighbors.size();
}


HalfEdge*
HalfEdgeGraph::_FindInAdjacentEdges(const HalfEdge* edge, size_t endVertex)
{
  HalfEdge* current = _GetNextAdjacentEdge(edge);
  while (current && current != edge) {
    if (_halfEdges[current->next].vertex == endVertex)return current;
    current = _GetNextAdjacentEdge(current);
  }
  if (!current) {
    current = _GetPreviousAdjacentEdge(edge);
    while (current && current != edge) {
      if (_halfEdges[current->next].vertex == endVertex)return current;
      current = _GetPreviousAdjacentEdge(current);
    }
  }
  return NULL;
}

void
HalfEdgeGraph::_RemoveOneEdge(const HalfEdge* edge, bool* modified)
{
  const int edgeIdx = _GetEdgeIndex(edge);
  if (_halfEdgeUsed[edgeIdx]) {
    _availableEdges.push(edgeIdx);
    _halfEdgeUsed[edgeIdx] = false;
    *modified = true;
  }
}

void
HalfEdgeGraph::RemoveEdge(HalfEdge* edge, bool* modified)
{
  if (!_halfEdgeUsed[_GetEdgeIndex(edge)])return;
  HalfEdge* prev = _GetPreviousEdge(edge);
  HalfEdge* next = _GetNextEdge(edge);

  bool isTriangle = _IsTriangle(edge);
  if (edge->twin > HalfEdge::INVALID_INDEX) _halfEdges[edge->twin].twin = HalfEdge::INVALID_INDEX;
  if (isTriangle) {
    if (prev->twin > HalfEdge::INVALID_INDEX)_halfEdges[prev->twin].twin = next->twin;
    if (next->twin > HalfEdge::INVALID_INDEX)_halfEdges[next->twin].twin = prev->twin;

    _RemoveOneEdge(edge, modified);
    _RemoveOneEdge(prev, modified);
    _RemoveOneEdge(next, modified);
  }
  else {
    next->prev = _GetEdgeIndex(prev);
    prev->next = _GetEdgeIndex(next);
    _RemoveOneEdge(edge, modified);
  }

  if (!*modified)std::cerr << "FAIL REMOVE EDGE " << edge->vertex << std::endl;
}

void 
HalfEdgeGraph::_UpdatePoint(size_t startIdx, size_t endIdx, size_t oldIdx, size_t replaceIdx)
{
  for (size_t edgeIdx = startIdx; edgeIdx < endIdx; ++edgeIdx) {
    if (!_halfEdgeUsed[edgeIdx])continue;
    HalfEdge& edge = _halfEdges[edgeIdx];
    if (edge.vertex == oldIdx) edge.vertex = replaceIdx;
    else if (edge.vertex > oldIdx) edge.vertex--;
  }
}

void
HalfEdgeGraph::RemovePoint(size_t index, size_t replace)
{
  _vertexHalfEdge.erase(_vertexHalfEdge.begin() + index);
  pxr::WorkParallelForN(
    _halfEdges.size(),
    std::bind(&HalfEdgeGraph::_UpdatePoint, this, 
      std::placeholders::_1, std::placeholders::_2, index, replace)
  );
}

HalfEdge* 
HalfEdgeGraph::_GetPreviousAdjacentEdge(const HalfEdge* edge)
{
  std::cout << "get previous edge : " << edge << std::endl;
  if (_halfEdges[edge->prev].twin != HalfEdge::INVALID_INDEX)
    return &_halfEdges[_halfEdges[edge->prev].twin];
  return NULL;
}

const HalfEdge*
HalfEdgeGraph::_GetPreviousAdjacentEdge(const HalfEdge* edge) const
{
  if (_halfEdges[edge->prev].twin != HalfEdge::INVALID_INDEX)
    return &_halfEdges[_halfEdges[edge->prev].twin];
  return NULL;
}

HalfEdge* 
HalfEdgeGraph::_GetNextAdjacentEdge(const HalfEdge* edge)
{
  if (edge->twin != HalfEdge::INVALID_INDEX)
    return &_halfEdges[_halfEdges[edge->twin].next];
    
  return NULL;

}

const HalfEdge*
HalfEdgeGraph::_GetNextAdjacentEdge(const HalfEdge* edge) const
{
  if (edge->twin != HalfEdge::INVALID_INDEX)
    return &_halfEdges[_halfEdges[edge->twin].next];

  return NULL;

}

HalfEdge* 
HalfEdgeGraph::_GetNextEdge(const HalfEdge* edge)
{
  return &_halfEdges[edge->next];
}

const HalfEdge*
HalfEdgeGraph::_GetNextEdge(const HalfEdge* edge) const
{
  return &_halfEdges[edge->next];
}

HalfEdge* 
HalfEdgeGraph::_GetPreviousEdge(const HalfEdge* edge)
{
  return &_halfEdges[edge->prev];
}

const HalfEdge*
HalfEdgeGraph::_GetPreviousEdge(const HalfEdge* edge) const
{
  return &_halfEdges[edge->prev];
}

bool
HalfEdgeGraph::_IsTriangle(const HalfEdge* edge) const
{
  int vertex = edge->vertex;
  int cnt = 1;
  const HalfEdge* current = &_halfEdges[edge->next];
  while (current->vertex != vertex) {
    cnt++;
    current = &_halfEdges[current->next];
  }
  return (cnt == 3);
}

void 
HalfEdgeGraph::_TriangulateFace(const HalfEdge* edge)
{
  /*
    triangulate a face given half-edge
    will generate 2 * (numTriangles - 1) new half edges

       3 _ _ _ 2      3 _ _ _ 2  
        |     |     3   \    |   
        |     |      |\  \   |   
        |     |      | \  \  |   
        |     |      |  \  \ |   
        |     |      |   \  \|   
        |_ _ _|      | _ _\   1  
       0       1    0      1     
  */
  size_t numFaceVertices = _GetFaceVerticesCount(edge);
  if (numFaceVertices < 4)return;

  size_t numTriangles = numFaceVertices - 2;
  size_t firstIdx = _GetEdgeIndex(edge);
  HalfEdge* next = &_halfEdges[edge->next];
  HalfEdge* prev = &_halfEdges[edge->prev];
  size_t currentIdx = _GetEdgeIndex(next);
  size_t nextIdx = _GetEdgeIndex(&_halfEdges[next->next]);
  size_t lastIdx = _GetEdgeIndex(prev);
  size_t previousInsertedIdx;

  for (size_t t = 0; t < numTriangles; ++t) {
    if (t == 0) {
      HalfEdge* inserted = GetAvailableEdge();
      size_t insertedIdx = _GetEdgeIndex(inserted);
      inserted->prev = currentIdx;
      inserted->next = firstIdx;
      inserted->vertex = _halfEdges[nextIdx].vertex;
      _halfEdges[currentIdx].next = insertedIdx;
      _halfEdges[firstIdx].prev = insertedIdx;
      previousInsertedIdx = insertedIdx;
    }
    else if (t == (numTriangles - 1)) {
      HalfEdge* inserted = GetAvailableEdge();
      size_t insertedIdx = _GetEdgeIndex(inserted);
      inserted->prev = lastIdx;
      inserted->next = currentIdx;
      inserted->vertex = _halfEdges[firstIdx].vertex;
      inserted->twin = previousInsertedIdx;
      _halfEdges[previousInsertedIdx].twin = insertedIdx;
      _halfEdges[currentIdx].prev = insertedIdx;
      _halfEdges[lastIdx].next = insertedIdx;
    }
    else {
      HalfEdge* left = GetAvailableEdge();
      HalfEdge* right = GetAvailableEdge();
      size_t leftIdx = _GetEdgeIndex(left);
      size_t rightIdx = _GetEdgeIndex(right);
      left->next = currentIdx;
      left->prev = rightIdx;
      left->vertex = _halfEdges[firstIdx].vertex;
      left->twin = previousInsertedIdx;
      _halfEdges[previousInsertedIdx].twin = leftIdx;
      right->prev = currentIdx;
      right->next = leftIdx;
      right->vertex = _halfEdges[nextIdx].vertex;
      _halfEdges[currentIdx].prev = leftIdx;
      _halfEdges[currentIdx].next = rightIdx;
      previousInsertedIdx = rightIdx;
    }
    currentIdx = nextIdx;
    nextIdx = _GetEdgeIndex(&_halfEdges[_halfEdges[nextIdx].next]);
  }
}

void HalfEdgeGraph::ComputeAdjacents()
{
  size_t numPoints = _boundary.size();
  _adjacents.clear();
  _adjacentsCount.resize(numPoints);
  _adjacentsOffset.resize(numPoints);

  std::vector<int> visited(numPoints, 0);

  pxr::VtArray<int> adjacents;
  size_t adjacentsOffset = 0;

  for (HalfEdge& halfEdge : _halfEdges) {
    if(visited[halfEdge.vertex]) continue;
    adjacents.clear();
    _ComputeVertexNeighbors(&halfEdge, adjacents, true);
    for(auto& adjacent: adjacents)_adjacents.push_back(adjacent);
    _adjacentsCount[halfEdge.vertex] = adjacents.size();
    _adjacentsOffset[halfEdge.vertex] = adjacentsOffset;
    adjacentsOffset += adjacents.size();
    visited[halfEdge.vertex]++;
  }
}

void
HalfEdgeGraph::ComputeAdjacents(const HalfEdge* edge, pxr::VtArray<int>& adjacents)
{
  _ComputeVertexNeighbors(edge, adjacents, true);
}

void HalfEdgeGraph::ComputeNeighbors()
{
  size_t numPoints = _boundary.size();
  _neighbors.clear();
  _neighborsCount.resize(numPoints);
  _neighborsOffset.resize(numPoints);

  std::vector<int> visited(numPoints, 0);

  pxr::VtArray<int> neighbors;
  size_t neighborsOffset = 0;

  for (HalfEdge& halfEdge : _halfEdges) {
    if(visited[halfEdge.vertex]) continue;
    neighbors.clear();
    _ComputeVertexNeighbors(&halfEdge, neighbors, false);
    for(auto& neighbor: neighbors)_neighbors.push_back(neighbor);
    _neighborsCount[halfEdge.vertex] = neighbors.size();
    _neighborsOffset[halfEdge.vertex] = neighborsOffset;
    neighborsOffset += neighbors.size();
    visited[halfEdge.vertex]++;
  }
}

void
HalfEdgeGraph::ComputeNeighbors(const HalfEdge* edge, pxr::VtArray<int>& neighbors)
{
  _ComputeVertexNeighbors(edge, neighbors, false);
}

void
HalfEdgeGraph::_ComputeVertexNeighbors(const HalfEdge* edge, pxr::VtArray<int>& neighbors, bool connected)
{
  if(connected) {
    const HalfEdge* current = edge;
    do {
      neighbors.push_back(_halfEdges[current->next].vertex);
      current = _GetNextAdjacentEdge(current);
      if(current == edge)return;
    } while(current);

    current = &_halfEdges[edge->prev];
    do {
      neighbors.push_back(current->vertex);
      current = _GetPreviousAdjacentEdge(current);
      if(current == edge)return;
    } while (current);
  } else {
    const HalfEdge* current = edge;
    do {
      HalfEdge* next = _GetNextEdge(current);
      do {
        if (_boundary[current->vertex] && _boundary[next->vertex])
          neighbors.push_back(next->vertex);
        else if (_GetNextEdge(next) != current)
          neighbors.push_back(next->vertex);

        next = _GetNextEdge(next);
      } while (next != current);
      current = _GetNextAdjacentEdge(current);
      if(current == edge)return;
    } while(current);

    current = &_halfEdges[edge->prev];
    if(current->twin == HalfEdge::INVALID_INDEX)return;
    current = &_halfEdges[current->twin];
    do {
      HalfEdge* next = _GetNextEdge(current);
      do {
        if (_boundary[current->vertex] && _boundary[next->vertex])
          neighbors.push_back(next->vertex);
        else if (_GetNextEdge(next) != current)
          neighbors.push_back(next->vertex);
        next = _GetNextEdge(next);
      } while (next != current);
      current = _GetNextAdjacentEdge(current);
      if(current == edge)return;
    } while(current);
  }
}

void 
HalfEdgeGraph::ComputeCotangentWeights(const pxr::GfVec3f *positions)
{
  size_t numPoints = _neighborsCount.size();

  _cotangentWeights.resize(_neighbors.size() + _neighborsCount.size());

  
  for (size_t i = 0; i < GetNumVertices(); ++i) {

    float sum = 0.f;
    int neighbor, previous, next;
    size_t numNeighbors = _neighborsCount[i];
    for(size_t j = 0; j < numNeighbors; ++j) {
      neighbor = GetNeighbor(i, j);
      previous = GetNeighbor(i, (j-1)%numNeighbors);
      next = GetNeighbor(i, (j+1)%numNeighbors);

      const pxr::GfVec3f& a = positions[i];
      const pxr::GfVec3f& b = positions[neighbor];
      const pxr::GfVec3f& u = positions[previous];
      const pxr::GfVec3f& v = positions[next];

      // compute the vectors in order to compute the triangles
      pxr::GfVec3f vec1(u - a);
      pxr::GfVec3f vec2(u - b);
      pxr::GfVec3f vec3(v - a);
      pxr::GfVec3f vec4(v - b);

      // compute alpha and beta
      float alpha = acos((vec1*vec2) / (vec1.GetLength() * vec2.GetLength()));
      float beta = acos((vec3*vec4) / (vec3.GetLength() * vec4.GetLength()));

      float cotan = _CotangentWeight(alpha) + _CotangentWeight(beta);

      _cotangentWeights[_neighborsOffset[i] + i + j] = cotan;
      sum += cotan;
      
    }
    _cotangentWeights[_neighborsOffset[i] + i + _neighborsCount[i]] = sum;
  }

  std::cout << _cotangentWeights << std::endl;


}

bool
HalfEdgeGraph::FlipEdge(HalfEdge* edge)
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
  if (edge->twin < 0) {
    std::cout << "flip edge : no twin edge ! aborted " << std::endl;
    return false;
  }
  if (!_IsTriangle(edge) || !_IsTriangle(&_halfEdges[edge->twin])) {
    std::cout << "flip edge : edges must belong to triangles ! aborted " << std::endl;
    return false;
  }

  HalfEdge* twin = &_halfEdges[edge->twin];

  HalfEdge* edges[6] = {
    edge, &_halfEdges[edge->next], &_halfEdges[edge->prev],
    twin, &_halfEdges[twin->next], &_halfEdges[twin->prev]
  };

  int vertices[4] = {
    edges[1]->vertex,
    edges[2]->vertex,
    edges[4]->vertex,
    edges[5]->vertex
  };

  edges[0]->vertex = vertices[3];
  edges[3]->vertex = vertices[1];
  edges[0]->next = _GetEdgeIndex(edges[2]);
  edges[1]->next = _GetEdgeIndex(edges[0]);
  edges[2]->next = _GetEdgeIndex(edges[1]);
  edges[3]->next = _GetEdgeIndex(edges[5]);
  edges[4]->next = _GetEdgeIndex(edges[3]);
  edges[5]->next = _GetEdgeIndex(edges[4]);

  return true;
}

bool 
HalfEdgeGraph::SplitEdge(HalfEdge* edge, size_t newPoint)
{
  /*
  slip edge will create 1 new half edges for the edge starting at new point
  if there is a twin edge it will also create 1 new half edges for it
                                  
               2              2  
              /|            /|   
             / |           / |   
            /  |          /  |   
           /   |       3 x   |   
          /    |        /    |   
         /_ _ _|       /_ _ _|   
        0       1     0       1  
                                 
  */
  size_t edgeIdx = _GetEdgeIndex(edge);

  HalfEdge* inserted = GetAvailableEdge();
  size_t insertedIdx = _GetEdgeIndex(inserted);
  inserted->vertex = newPoint;
  
  // reallocation mess pointer so retrieve edge
  edge = &_halfEdges[edgeIdx];
  HalfEdge* next = &_halfEdges[edge->next];
  inserted->next = _GetEdgeIndex(next);
  inserted->prev = edgeIdx;

  edge->next = insertedIdx;
  next->prev = insertedIdx;

  HalfEdge* twin = edge->twin >= 0 ? &_halfEdges[edge->twin] : NULL;

  if (twin) {
    inserted->twin = edge->twin;
    twin->twin = _GetEdgeIndex(inserted);

    HalfEdge* tInserted = GetAvailableEdge();
    tInserted->vertex = newPoint;
    size_t tInsertedIdx = _GetEdgeIndex(tInserted);
    HalfEdge* tNext = &_halfEdges[twin->next];
    tInserted->next = _GetEdgeIndex(tNext);
    tInserted->prev = _GetEdgeIndex(twin);
    tInserted->twin = edgeIdx;
    edge->twin = _GetEdgeIndex(tInserted);
    twin->next = tInsertedIdx;
    tNext->prev = tInsertedIdx;
  }
  _vertexHalfEdge.push_back(_GetEdgeIndex(inserted));
 
  return true;
}


bool 
HalfEdgeGraph::CollapseEdge(HalfEdge* edge)
{
  
  HalfEdge* twin = NULL;
  if(edge->twin >= 0) twin = &_halfEdges[edge->twin];

  size_t p1 = edge->vertex;
  size_t p2 = _halfEdges[edge->next].vertex;

  bool modified = false;
  RemoveEdge(edge, &modified);

  if (twin) {
    RemoveEdge(twin, &modified);
  }
  if (!modified)return false;

  if (p1 > p2) {
    RemovePoint(p1, p2);
  }
  else {
    RemovePoint(p2, p1);
  }
  return true;
}

bool
HalfEdgeGraph::CollapseStar(HalfEdge* edge, pxr::VtArray<int>& neighbors)
{
  std::cout << "collapse start begin .." << std::endl;

  bool modified = false;
  size_t vertex = edge->vertex;
  _ComputeVertexNeighbors(edge, neighbors);
  std::sort(neighbors.begin(), neighbors.end(), std::greater<>());
  
  pxr::VtArray<int> edgeIndices;
  for (auto& neighbor : neighbors) {
    edgeIndices.push_back(_GetEdgeIndex(GetEdgeFromVertices(vertex, neighbor)));
  }
  for (auto& edgeIdx : edgeIndices) {
    HalfEdge* current = &_halfEdges[edgeIdx];
    HalfEdge* twin = NULL;
    if (current->twin >= 0) twin = &_halfEdges[current->twin];
    RemoveEdge(current, &modified);
    if (twin)RemoveEdge(twin, &modified);
  }
  size_t lowest = neighbors.back();
  if (vertex > lowest) {
    for (size_t neighborIdx = 0; neighborIdx < neighbors.size()-1; ++neighborIdx) {
      RemovePoint(neighbors[neighborIdx], lowest);
    }
    RemovePoint(vertex, lowest);
  } else {
    for (auto& neighbor : neighbors)
      RemovePoint(neighbor, vertex);
  }
  std::cout << "collapse start end .." << std::endl;
  return modified;
}

bool 
HalfEdgeGraph::CollapseFace(HalfEdge* edge, pxr::VtArray<int>& vertices)
{
  vertices.clear();
  vertices.push_back(edge->vertex);

  pxr::VtArray<int> indices;
  GetEdgesFromFace(edge, indices);



  const HalfEdge* current = &_halfEdges[edge->next];
  return false;
}

void
HalfEdgeGraph::ComputeTopology(pxr::VtArray<int>& faceCounts, pxr::VtArray<int>& faceConnects) const
{
  faceCounts.clear();
  faceConnects.clear();
  std::vector<bool> visited(_halfEdges.size(), false);

  for (auto& edge : _halfEdges) {
    int edgeIdx = _GetEdgeIndex(&edge);
    if (!_halfEdgeUsed[edgeIdx] || visited[edgeIdx])continue;
    visited[edgeIdx] = true;
    const HalfEdge* start = &edge;
    const HalfEdge* current = _GetNextEdge(start);
    faceConnects.push_back(start->vertex);
    size_t faceVertexCount = 1;
    while (current != start) {
      visited[_GetEdgeIndex(current)] = true;
      faceConnects.push_back(current->vertex);
      faceVertexCount++;
      current = _GetNextEdge(current);
    }
    faceCounts.push_back(faceVertexCount);
  }
}

size_t 
HalfEdgeGraph::GetNumNeighbors(size_t index) const
{
  return _neighborsCount[index];
}

const int*  
HalfEdgeGraph::GetNeighbors(size_t index) const
{
  return &_neighbors[_neighborsOffset[index]];
}

int  
HalfEdgeGraph::GetNeighbor(size_t index, size_t neighbor) const
{
  return _neighbors[_neighborsOffset[index]+neighbor];
}

size_t  
HalfEdgeGraph::GetNumAdjacents(size_t index) const
{
  return _adjacentsCount[index];
}

const int*  
HalfEdgeGraph::GetAdjacents(size_t index) const
{
  return &_adjacents[_adjacentsOffset[index]];
}

int  
HalfEdgeGraph::GetAdjacent(size_t index, size_t adjacent) const
{
   return _adjacents[_adjacentsOffset[index]+adjacent];
}

const float*  
HalfEdgeGraph::GetCotangentWeights(size_t index) const
{
  return &_cotangentWeights[_neighborsOffset[index] + index];
}

float  
HalfEdgeGraph::GetCotangentWeight(size_t index, size_t neighbor) const
{
   return _cotangentWeights[_neighborsOffset[index] + index + neighbor];
}

JVR_NAMESPACE_CLOSE_SCOPE