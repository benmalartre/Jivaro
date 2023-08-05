// HalfEdge
//----------------------------------------------
#include <algorithm>
#include <pxr/base/work/loops.h>
#include "../geometry/halfEdge.h"
#include "../geometry/triangle.h"
#include "../geometry/mesh.h"

#include "../utils/timer.h"


JVR_NAMESPACE_OPEN_SCOPE


struct _T {
  uint64_t t;
  uint64_t accum;
  size_t   num;
  void Start() {
    t = CurrentTime();
  }
  void End() {
    accum += CurrentTime() - t;
    num++;
  };
  void Reset() {
    accum = 0;
    num = 0;
  };
  double Average() {
    if (num) {
      return ((double)accum * 1e-9) / (double)num;
    }
    return 0;
  }
  double Elapsed() {
    return (double)accum * 1e-9;
  }
};
static _T REMOVE_EDGE_AVG_T = { 0,0};
static _T REMOVE_POINT_AVG_T = { 0,0 };


HalfEdgeGraph::ItUniqueEdge::ItUniqueEdge(HalfEdgeGraph* graph)
{
  edges = graph;
  index = -1;
}

HalfEdge* HalfEdgeGraph::ItUniqueEdge::Next()
{
  index++;
  while (true) {
    if (index >= edges->_halfEdges.size())return NULL;
    HalfEdge* edge = &edges->_halfEdges[index];
    if (!edges->IsUsed(edge) || !edges->IsUnique(edge))
      index++;
    else return edge;
  }
}

size_t
HalfEdgeGraph::GetLongestEdgeInTriangle(const pxr::GfVec3i& vertices, const pxr::GfVec3f* positions) const
{
  const float edge0 = (positions[vertices[1]] - positions[vertices[0]]).GetLength();
  const float edge1 = (positions[vertices[2]] - positions[vertices[1]]).GetLength();
  const float edge2 = (positions[vertices[0]] - positions[vertices[2]]).GetLength();
  if (edge0 > edge1 && edge0 > edge2)return 0;
  else if (edge1 > edge0 && edge1 > edge2)return 1;
  else return 2;
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
HalfEdgeGraph::GetNumEdges() const
{
  size_t numEdges = 0;
  for (auto& edge : _halfEdges) {
    size_t edgeIdx = _GetEdgeIndex(&edge);
    if (_halfEdgeUsed[edgeIdx] && IsUnique(&edge))numEdges++;
  }
  return numEdges;
}

pxr::VtArray<HalfEdge>& 
HalfEdgeGraph::GetEdges()
{
  return _halfEdges;
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
  if (_halfEdges[edge->next].vertex == end)return edge;
  HalfEdge* current = _GetNextAdjacentEdge(edge);
  while (current && (current != edge)) {
    if (_halfEdges[current->next].vertex == end)return current;
    current = _GetNextAdjacentEdge(current);
  }

  if (!current) {
    HalfEdge* current = _GetPreviousAdjacentEdge(edge);
    while (current && (current != edge)) {
      if (_halfEdges[current->next].vertex == end)return current;
      current = _GetPreviousAdjacentEdge(current);
    }
  }
  return NULL;
}

const HalfEdge*
HalfEdgeGraph::GetEdgeFromVertices(size_t start, size_t end) const
{
  return GetEdgeFromVertices(start, end);
}

void 
HalfEdgeGraph::GetEdgeFromFace(const HalfEdge* edge, pxr::VtArray<int>& indices)
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


void 
HalfEdgeGraph::ComputeGraph(Mesh* mesh)
{
  const size_t numPoints = mesh->GetNumPoints();
  const pxr::VtArray<int>& faceConnects = mesh->GetFaceConnects();
  size_t numHalfEdges = 0;
  size_t numTriangles = 0;
  
  for (const auto& faceVertexCount : mesh->GetFaceCounts()) {
    numHalfEdges += faceVertexCount;
    numTriangles += faceVertexCount - 2;
  }

  _halfEdges.resize(numHalfEdges);
  _halfEdgeUsed.resize(numHalfEdges);

  size_t faceOffsetIdx = 0;
  size_t halfEdgeIdx = 0;

  HalfEdge* halfEdge = &_halfEdges[0];

  pxr::TfHashMap<uint64_t, HalfEdge*, pxr::TfHash> halfEdgesMap;
  _vertexHalfEdge.resize(numPoints);

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    _vertexHalfEdge[pointIdx] = -1;
  }

  for (const auto& faceVertexCount: mesh->GetFaceCounts())
  { 
    size_t numFaceTriangles = faceVertexCount - 2;
    for (size_t faceEdgeIdx = 0; faceEdgeIdx < faceVertexCount; ++faceEdgeIdx) {
      // get vertices
      size_t v0 = faceConnects[faceOffsetIdx + faceEdgeIdx];
      size_t v1 = faceConnects[faceOffsetIdx + ((faceEdgeIdx + 1) % faceVertexCount)];
      if (_vertexHalfEdge[v0] < 0)_vertexHalfEdge[v0] = _GetEdgeIndex(halfEdge);

      size_t last = faceVertexCount - 1;
      halfEdgesMap[v1 | (v0 << 32)] = halfEdge;
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
      halfEdge++;
    }
    faceOffsetIdx += faceVertexCount;
  }

  _boundary.resize(numPoints);
  memset(&_boundary[0], false, numPoints * sizeof(bool));

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
      twinEdge->twin = _GetEdgeIndex(halfEdge.second);
      halfEdge.second->twin = _GetEdgeIndex(twinEdge);
    }
    else {
      _boundary[halfEdge.second->vertex] = true;
      _boundary[_halfEdges[halfEdge.second->next].vertex] = true;
      ++boundaryCount;
    }
  }
}

size_t 
HalfEdgeGraph::_GetEdgeIndex(const HalfEdge* edge) const
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
  if (edge->twin > -1) _halfEdges[edge->twin].twin = -1;
  if (isTriangle) {
    if (prev->twin > -1)_halfEdges[prev->twin].twin = next->twin;
    if (next->twin > -1)_halfEdges[next->twin].twin = prev->twin;

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
  const HalfEdge* prev = &_halfEdges[edge->prev];
  if (prev->twin >= 0)
    return &_halfEdges[prev->twin];
  return NULL;
}

const HalfEdge*
HalfEdgeGraph::_GetPreviousAdjacentEdge(const HalfEdge* edge) const
{
  const HalfEdge* prev = &_halfEdges[edge->prev];
  if (prev->twin >= 0)
    return &_halfEdges[prev->twin];
  return NULL;
}

HalfEdge* 
HalfEdgeGraph::_GetNextAdjacentEdge(const HalfEdge* edge)
{
  if (edge->twin >= 0)
    return &_halfEdges[_halfEdges[edge->twin].next];
    
  return NULL;

}

const HalfEdge*
HalfEdgeGraph::_GetNextAdjacentEdge(const HalfEdge* edge) const
{
  if (edge->twin >= 0)
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


void
HalfEdgeGraph::ComputeNeighbors(const HalfEdge* edge, pxr::VtArray<int>& neighbors)
{
  _ComputeVertexNeighbors(edge, neighbors);
}

void
HalfEdgeGraph::_ComputeVertexNeighbors(const HalfEdge* edge, pxr::VtArray<int>& neighbors)
{
  const HalfEdge* current = edge;
  do {
    neighbors.push_back(_halfEdges[current->next].vertex);
    current = _GetNextAdjacentEdge(current);
  } while(current && (current != edge));

  if (!current) {
    const HalfEdge* current = edge;
    do {
      neighbors.push_back(_halfEdges[current->next].vertex);
      current = _GetPreviousAdjacentEdge(current);
    } while (current);
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
  REMOVE_EDGE_AVG_T.Start();
  RemoveEdge(edge, &modified);
  REMOVE_EDGE_AVG_T.End();

  if (twin) {
    REMOVE_EDGE_AVG_T.Start();
    RemoveEdge(twin, &modified);
    REMOVE_EDGE_AVG_T.End();
  }
  if (!modified)return false;

  if (p1 > p2) {
    REMOVE_POINT_AVG_T.Start();
    RemovePoint(p1, p2);
    REMOVE_POINT_AVG_T.End();
  }
  else {
    REMOVE_POINT_AVG_T.Start();
    RemovePoint(p2, p1);
    REMOVE_POINT_AVG_T.End();
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
  GetEdgeFromFace(edge, indices);



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

JVR_NAMESPACE_CLOSE_SCOPE