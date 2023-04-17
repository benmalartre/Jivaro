// HalfEdge
//----------------------------------------------
#include "../geometry/halfEdge.h"
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

/*
// Using a custom function object to compare elements.
struct
{
  bool operator() (const int l, const int r) const { return l > r; }
} customLess;
std::priority_queue minq3(data.begin(), data.end(), customLess);
*/

const HalfEdge*
HalfEdgeGraph::GetLongestEdgeInTriangle(const HalfEdge* edge, const pxr::GfVec3f* positions) const
{
  const HalfEdge* next = &_halfEdges[edge->next];
  const HalfEdge* prev = &_halfEdges[edge->prev];
  const float edge0 = (positions[next->vertex] - positions[edge->vertex]).GetLength();
  const float edge1 = (positions[prev->vertex] - positions[next->vertex]).GetLength();
  const float edge2 = (positions[edge->vertex] - positions[prev->vertex]).GetLength();
  if (edge0 > edge1 && edge0 > edge2)return edge;
  else if (edge1 > edge0 && edge1 > edge2)return next;
  else return prev;
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

size_t 
HalfEdgeGraph::GetCapacityEdges() const
{
  return _halfEdges.size();
}

size_t 
HalfEdgeGraph::GetNumEdges() const
{
  size_t numEdges = 0;
  for (auto& usedEdge : _usedEdges)
    if(usedEdge->twin < 0 || usedEdge->vertex < _halfEdges[usedEdge->next].vertex)
      numEdges++;

  return numEdges;
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

const pxr::VtArray<HalfEdge*>& 
HalfEdgeGraph::GetEdges()
{
  return _usedEdges;
}

void
HalfEdgeGraph::SortEdgesByLength(const pxr::GfVec3f* positions)
{
  std::sort(_usedEdges.begin(), _usedEdges.end(),
    [this, positions](const HalfEdge* a, const HalfEdge* b) {
    return GetLengthSq(a, positions) < GetLengthSq(b, positions);
  });
}

bool
HalfEdgeGraph::IsCollapsable(const HalfEdge* edge)
{
  if(!IsUnique(edge))return false;
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
HalfEdgeGraph::IsAvailable(const HalfEdge* edge)
{
  return _FindInAvailableEdges(edge);
}

bool
HalfEdgeGraph::IsUnique(const HalfEdge* edge)
{
  if (edge->twin < 0)return true;
  else return edge->vertex < _halfEdges[edge->next].vertex;
}

bool
HalfEdgeGraph::IsUsed(const HalfEdge* edge)
{
  return _halfEdgeUsed[_GetEdgeIndex(edge)];
}

void 
HalfEdgeGraph::ComputeGraph(Mesh* mesh)
{
  const size_t numPoints = mesh->GetNumPoints();
  const pxr::VtArray<int>& faceConnects = mesh->GetFaceConnects();
  size_t numHalfEdges = 0;
  for (const auto& faceVertexCount : mesh->GetFaceCounts())
    numHalfEdges += faceVertexCount;

  _halfEdges.resize(numHalfEdges);
  _usedEdges.resize(numHalfEdges);
  _halfEdgeUsed.resize(numHalfEdges);

  size_t faceOffsetIdx = 0;
  size_t halfEdgeIdx = 0;

  HalfEdge* halfEdge = &_halfEdges[0];

  pxr::TfHashMap<uint64_t, HalfEdge*, pxr::TfHash> halfEdgesMap;
  _vertexHalfEdge.resize(numPoints);

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    _vertexHalfEdge[pointIdx] = NULL;
  }

  for (const auto& faceVertexCount: mesh->GetFaceCounts())
  { 
    for (size_t faceEdgeIdx = 0; faceEdgeIdx < faceVertexCount; ++faceEdgeIdx) {
      // get vertices
      size_t v0 = faceConnects[faceOffsetIdx + faceEdgeIdx];
      size_t v1 = faceConnects[faceOffsetIdx + ((faceEdgeIdx + 1) % faceVertexCount)];
      if (!_vertexHalfEdge[v0])_vertexHalfEdge[v0] = halfEdge;
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
      _usedEdges[faceOffsetIdx + faceEdgeIdx] = halfEdge;
      halfEdge++;
    }
    faceOffsetIdx += faceVertexCount;
  }

  // sort edges by length
  /*const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
  std::sort(_usedEdges.begin(), _usedEdges.end(), 
    [positions](HalfEdge* a, HalfEdge* b) {
      return a->GetLengthSq(positions) < b->GetLengthSq(positions);
    });
  
  std::sort(_usedEdges.begin(), _usedEdges.end(),
    [](HalfEdge* a, HalfEdge* b) {if (a->twin && a->vertex > a->twin->vertex) {
                                    return a->twin->vertex < b->vertex;
                                  }else return a->vertex < b->vertex; });*/

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
      ++boundaryCount;
    }
  }

  for (auto& used : _usedEdges)_GetEdgeIndex(used);
}


const pxr::VtArray<int>&
HalfEdgeGraph::GetVertexNeighbors(const HalfEdge* edge)
{
  return _neighbors[edge->vertex];

}

size_t 
HalfEdgeGraph::_GetEdgeIndex(const HalfEdge* edge)
{
  return ((intptr_t)edge - (intptr_t)&_halfEdges[0]) / sizeof(HalfEdge);
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

bool
HalfEdgeGraph::_FindInAvailableEdges(const HalfEdge* edge)
{
  for (auto& availableEdge : _availableEdges) {
    if (availableEdge == edge)return true;
  }
  return false;
}

void
HalfEdgeGraph::_RemoveOneEdge(const HalfEdge* edge, bool* modified)
{
  for (size_t currentIdx = 0; currentIdx < _usedEdges.size(); ++currentIdx) {
    if (_usedEdges[currentIdx] == edge) {
      _availableEdges.push_back(_usedEdges[currentIdx]);
      _halfEdgeUsed[_GetEdgeIndex(edge)] = false;
      _usedEdges[currentIdx] = std::move(_usedEdges.back());
      _usedEdges.pop_back();
      *modified = true;
      break;
    }
  }
}

void
HalfEdgeGraph::RemoveEdge(HalfEdge* edge, bool* modified)
{
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
HalfEdgeGraph::RemovePoint(size_t index, size_t replace)
{
  _vertexHalfEdge.erase(_vertexHalfEdge.begin() + index);
  for (auto& edge: _usedEdges) {
    if (edge->vertex == index) edge->vertex = replace;
    else if (edge->vertex > index) edge->vertex--;
  }
}


void 
HalfEdgeGraph::ComputeTrianglePairs(Mesh* mesh)
{
  std::vector<bool> used;
  used.assign(mesh->GetNumTriangles(), false);
  pxr::VtArray<Triangle>& triangles = mesh->GetTriangles();
  pxr::VtArray<TrianglePair>& trianglePairs = mesh->GetTrianglePairs();
  trianglePairs.clear();
  const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();

  for (auto& edge: _usedEdges) {
    if (used[GetTriangleIndex(edge)])continue;
    const HalfEdge* longest = GetLongestEdgeInTriangle(edge, positions);
    size_t triPairIdx = trianglePairs.size();
    if (longest->twin > -1) {
      HalfEdge* twin = &_halfEdges[longest->twin];
      trianglePairs.push_back(TrianglePair(
        triPairIdx,
        &triangles[GetTriangleIndex(longest)],
        &triangles[GetTriangleIndex(twin)]
      ));
      used[GetTriangleIndex(longest)] = true;
      used[GetTriangleIndex(twin)] = true;
    }
    else {
      trianglePairs.push_back(TrianglePair(
        triPairIdx,
        &triangles[GetTriangleIndex(longest)],
        NULL
      ));
      used[GetTriangleIndex(longest)] = true;
    }
  }
}

HalfEdge* 
HalfEdgeGraph::_GetPreviousAdjacentEdge(const HalfEdge* edge)
{
  if (edge->twin >= 0)
    return &_halfEdges[_halfEdges[edge->twin].prev];
  return NULL;
}

HalfEdge* 
HalfEdgeGraph::_GetNextAdjacentEdge(const HalfEdge* edge)
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

HalfEdge* 
HalfEdgeGraph::_GetPreviousEdge(const HalfEdge* edge)
{
  return &_halfEdges[edge->prev];
}

bool
HalfEdgeGraph::_IsTriangle(const HalfEdge* edge)
{
  int vertex = edge->vertex;
  int cnt = 1;
  HalfEdge* current = &_halfEdges[edge->next];
  while (current->vertex != vertex) {
    cnt++;
    current = &_halfEdges[current->next];
  }
  return (cnt == 3);
}


bool 
HalfEdgeGraph::_IsNeighborRegistered(const pxr::VtArray<int>& neighbors, int idx)
{
  for (int neighbor: neighbors) {
    if (neighbor == idx)return true;
  }
  return false;
}

void 
HalfEdgeGraph::ComputeNeighbors(Mesh* mesh)
{
  size_t numPoints = mesh->GetNumPoints();
  _neighbors.clear();
  _neighbors.resize(numPoints);
  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    _ComputeVertexNeighbors(_vertexHalfEdge[pointIdx], _neighbors[pointIdx]);
  }
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
      neighbors.push_back(_halfEdges[current->prev].vertex);
      current = _GetPreviousAdjacentEdge(current);
    } while (current && (current != edge));
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

  HalfEdge* twin = &_halfEdges[edge->twin];

  HalfEdge* edges[6] = {
    edge, &_halfEdges[edge->next], &_halfEdges[edge->prev],
    twin, &_halfEdges[twin->next], &_halfEdges[twin->prev]
  };

  uint32_t vertices[4] = {
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
  slip edge will create 3 new half edges for the edge
  if there is a twin edge it will also create 3 new half edges for it
                                  
               2              2  
              /|             /|  
             / |          3 / |  
            /  |         3  \ |  
           /   |         /\  \|  
          /    |        /      1 
         /_ _ _|       /_ _ \     
        0       1     0      1    
                                  
  */
  /*
  size_t numEdges = _halfEdges.size();
  if (edge->latency != HalfEdge::REAL)return false;

  HalfEdge* twin = edge->twin;
  if(twin) _halfEdges.resize(numEdges + 6);
  else _halfEdges.resize(numEdges + 3);

  HalfEdge* next = _GetNextEdge(edge, HalfEdge::REAL);
  HalfEdge* previous = _GetPreviousEdge(edge, HalfEdge::REAL);

  HalfEdge* halfEdge = _halfEdges[numEdges];
  halfEdge->index = numEdges;
  halfEdge->vertex = newPoint;
  halfEdge->next = next;
  halfEdge->latency = HalfEdge::REAL;
  HalfEdge* n1 = halfEdge;
  halfEdge++;

  halfEdge->index = numEdges + 1;
  halfEdge->vertex = newPoint;
  halfEdge->next = previous;
  halfEdge->latency = HalfEdge::IMPLICIT;
  HalfEdge* n2 = halfEdge;
  halfEdge++;

  halfEdge->index = numEdges + 2;
  halfEdge->vertex = newPoint;
  halfEdge->next = n2;
  halfEdge->latency = HalfEdge::IMPLICIT;
  HalfEdge* n3 = halfEdge;

  next->next = n3;
  edge->next = n1;
  
  if (twin) {
    HalfEdge* twinNext = _GetNextEdge(twin, HalfEdge::REAL);
    HalfEdge* twinPrevious = _GetPreviousEdge(twin, HalfEdge::REAL);

    halfEdge->index = numEdges + 3;
    halfEdge->vertex = newPoint;
    halfEdge->next = twinNext;
    halfEdge->latency = HalfEdge::REAL;
    HalfEdge* n4 = halfEdge;
    halfEdge++;

    halfEdge->index = numEdges + 4;
    halfEdge->vertex = newPoint;
    halfEdge->next = twinPrevious;
    halfEdge->latency = HalfEdge::IMPLICIT;
    HalfEdge* n5 = halfEdge;
    halfEdge++;

    halfEdge->index = numEdges + 5;
    halfEdge->vertex = twin->vertex;
    halfEdge->next = n4;
    halfEdge->latency = HalfEdge::IMPLICIT;
    HalfEdge* n6 = halfEdge;

    twinNext->next = n3;
    twin->next = n1;
  }
  return true;
  */
  return false;
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

void 
HalfEdgeGraph::UpdateTopologyFromEdges(Mesh* mesh)
{
  pxr::VtArray<int> faceCounts;
  pxr::VtArray<int> faceConnects;
  
  std::map<HalfEdge*, bool> visited;
  for (auto& edge : _usedEdges) 
    visited.insert({ edge, false });

  for (auto& edge: _usedEdges) {
    if (visited[edge])continue;
    visited[edge] = true;
    const HalfEdge* start = edge;
    HalfEdge* current = _GetNextEdge(start);
    faceConnects.push_back(start->vertex);
    size_t faceVertexCount = 1;
    while (current != start) {
      visited[current] = true;
      faceConnects.push_back(current->vertex);
      faceVertexCount++;
      current = _GetNextEdge(current);
    }
    faceCounts.push_back(faceVertexCount);
  }
  mesh->SetTopology(faceCounts, faceConnects, false);
}

void
HalfEdgeGraph::UpdateTopologyFromEdges(pxr::VtArray<int>& faceCounts, pxr::VtArray<int>& faceConnects)
{
  faceCounts.clear();
  faceConnects.clear();
  std::map<HalfEdge*, bool> visited;
  for (auto& edge : _usedEdges)
    visited.insert({ edge, false });

  for (auto& edge : _usedEdges) {
    if (visited[edge])continue;
    visited[edge] = true;
    const HalfEdge* start = edge;
    HalfEdge* current = _GetNextEdge(start);
    faceConnects.push_back(start->vertex);
    size_t faceVertexCount = 1;
    while (current != start) {
      visited[current] = true;
      faceConnects.push_back(current->vertex);
      faceVertexCount++;
      current = _GetNextEdge(current);
    }
    faceCounts.push_back(faceVertexCount);
  }
}

JVR_NAMESPACE_CLOSE_SCOPE