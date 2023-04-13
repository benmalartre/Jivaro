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
HalfEdge::GetLongestEdgeInTriangle(const pxr::GfVec3f* positions) const
{
  const HalfEdge* next = this->next;
  const HalfEdge* prev = next->next;
  const float edge0 = (positions[next->vertex] - positions[vertex]).GetLength();
  const float edge1 = (positions[prev->vertex] - positions[next->vertex]).GetLength();
  const float edge2 = (positions[vertex] - positions[prev->vertex]).GetLength();
  if (edge0 > edge1 && edge0 > edge2)return this;
  else if (edge1 > edge0 && edge1 > edge2)return next;
  else return prev;
}

size_t 
HalfEdgeGraph::GetNumEdges() const
{
  size_t numEdges = 0;
  for (auto& usedEdge : _usedEdges)
    if(!usedEdge->twin || usedEdge->vertex < usedEdge->next->vertex)
      numEdges++;

  return numEdges;
}


const pxr::VtArray<HalfEdge*>& 
HalfEdgeGraph::GetEdges()
{
  return _usedEdges;
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
      halfEdge->index = halfEdgeIdx++;
      if (!faceEdgeIdx) {
        halfEdge->prev = halfEdge + last;
        halfEdge->next = halfEdge + 1;
      } else if (faceEdgeIdx == last) {
        halfEdge->prev = halfEdge - 1;
        halfEdge->next = halfEdge - last;
      } else {
        halfEdge->prev = halfEdge - 1;
        halfEdge->next = halfEdge + 1;
      }

      _usedEdges[faceOffsetIdx + faceEdgeIdx] = halfEdge;
      halfEdge++;
    }
    faceOffsetIdx += faceVertexCount;
  }

  /*
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
      twinEdge->twin = halfEdge.second;
      halfEdge.second->twin = twinEdge;
    }
    else {
      _boundary[halfEdge.second->vertex] = true;
      ++boundaryCount;
    }
  }
}


const pxr::VtArray<int>&
HalfEdgeGraph::GetVertexNeighbors(const HalfEdge* edge)
{
  return _neighbors[edge->vertex];

}


HalfEdge*
HalfEdgeGraph::_FindInAdjacentEdges(const HalfEdge* edge, size_t endVertex)
{
  HalfEdge* current = _GetNextAdjacentEdge(edge);
  while (current && current != edge) {
    if (current->next->vertex == endVertex)return current;
    current = _GetNextAdjacentEdge(current);
  }
  if (!current) {
    current = _GetPreviousAdjacentEdge(edge);
    while (current && current != edge) {
      if (current->next->vertex == endVertex)return current;
      current = _GetPreviousAdjacentEdge(current);
    }
  }
  return NULL;
}

void
HalfEdgeGraph::_RemoveOneEdge(HalfEdge* edge, bool* modified)
{
  for (size_t currentIdx = 0; currentIdx < _usedEdges.size(); ++currentIdx) {
    if (_usedEdges[currentIdx] == edge) {
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

  
  if (edge->twin) {
    pxr::VtArray<int> edgeNeighbors;
    pxr::VtArray<int> twinNeighbors;
    size_t commonNeighbors = 0;
    _ComputeVertexNeighbors(edge, edgeNeighbors);
    _ComputeVertexNeighbors(edge->twin, twinNeighbors);

    for (auto& edgeNeighbor : edgeNeighbors)
      for (auto& twinNeighbor : twinNeighbors)
        commonNeighbors += (edgeNeighbor == twinNeighbor);

    if (commonNeighbors > 2) {
      std::cout << "problematic neighborhood : " << commonNeighbors << std::endl;
      return;
    }
  }
  
  bool isTriangle = _IsTriangle(edge);
  if (edge->twin) edge->twin->twin = NULL;
  if (isTriangle) {
    if (prev->twin)prev->twin->twin = next->twin;
    if (next->twin)next->twin->twin = prev->twin;

    _RemoveOneEdge(edge, modified);
    _RemoveOneEdge(prev, modified);
    _RemoveOneEdge(next, modified);
  }
  else {
    next->prev = prev;
    prev->next = next;
   
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

HalfEdge* 
HalfEdgeGraph::GetLongestEdge(const pxr::GfVec3f* positions)
{
  float maxLength = 0.f;
  HalfEdge* longestEdge = NULL;
  for (auto& used: _usedEdges) {
    if (used->vertex > used->next->vertex)continue;
    if (used->twin && used->vertex > used->twin->vertex)continue;
    HalfEdge* next = used->next;
    float edgeLength = (positions[next->vertex] - positions[used->vertex]).GetLengthSq();
    if (edgeLength > maxLength) {
      longestEdge = used;
      maxLength = edgeLength;
    }
  }
  return longestEdge;
}


HalfEdge* 
HalfEdgeGraph::GetShortestEdge(const pxr::GfVec3f* positions)
{
  float minLength = std::numeric_limits<float>::max();
  HalfEdge* shortestEdge = NULL;
  for (auto& used: _usedEdges) {
    if (used->twin && used->vertex > used->twin->vertex)continue;
    if (used->vertex > used->next->vertex)continue;
    HalfEdge* next = used->next;
    float edgeLength = (positions[next->vertex] - positions[used->vertex]).GetLengthSq();
    if (edgeLength < minLength) {
      shortestEdge = used;
      minLength = edgeLength;
    }
  }
  return shortestEdge;
}

HalfEdge*
HalfEdgeGraph::GetRandomEdge()
{
  return _usedEdges[RANDOM_0_X(_usedEdges.size() - 1)];
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
    if (used[edge->GetTriangleIndex()])continue;
    const HalfEdge* longest = edge->GetLongestEdgeInTriangle(positions);
    size_t triPairIdx = trianglePairs.size();
    if (longest->twin) {
      HalfEdge* twin = longest->twin;
      trianglePairs.push_back(TrianglePair(
        triPairIdx,
        &triangles[longest->GetTriangleIndex()],
        &triangles[twin->GetTriangleIndex()]
      ));
      used[longest->GetTriangleIndex()] = true;
      used[twin->GetTriangleIndex()] = true;
    }
    else {
      trianglePairs.push_back(TrianglePair(
        triPairIdx,
        &triangles[longest->GetTriangleIndex()],
        NULL
      ));
      used[longest->GetTriangleIndex()] = true;
    }
  }
}

HalfEdge* 
HalfEdgeGraph::_GetPreviousAdjacentEdge(const HalfEdge* edge)
{
  if (edge->twin)
    return edge->twin->prev;
  return NULL;
}

HalfEdge* 
HalfEdgeGraph::_GetNextAdjacentEdge(const HalfEdge* edge)
{
  if (edge->twin)
    return edge->twin->next;
    
  return NULL;

}

HalfEdge* 
HalfEdgeGraph::_GetNextEdge(const HalfEdge* edge)
{
  return edge->next;
}

HalfEdge* 
HalfEdgeGraph::_GetPreviousEdge(const HalfEdge* edge)
{
  return edge->prev;
}

bool
HalfEdgeGraph::_IsTriangle(const HalfEdge* edge)
{
  int vertex = edge->vertex;
  int cnt = 1;
  HalfEdge* current = edge->next;
  while (current->vertex != vertex) {
    cnt++;
    current = current->next;
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
HalfEdgeGraph::_ComputeVertexNeighbors(HalfEdge* edge, pxr::VtArray<int>& neighbors)
{
  HalfEdge* current = edge;
  do {
    neighbors.push_back(current->next->vertex);
    current = _GetNextAdjacentEdge(current);
  } while(current && (current != edge));

  if (!current) {
    HalfEdge* current = edge;
    do {
      neighbors.push_back(current->prev->vertex);
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
  if (!edge->twin) {
    std::cout << "flip edge : no twin edge ! aborted " << std::endl;
    return false;
  }

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

  edges[0]->vertex = vertices[3];
  edges[3]->vertex = vertices[1];
  edges[0]->next = edges[2];
  edges[1]->next = edges[0];
  edges[2]->next = edges[1];
  edges[3]->next = edges[5];
  edges[4]->next = edges[3];
  edges[5]->next = edges[4];

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
  
  HalfEdge* twin = edge->twin;

  size_t p1 = edge->vertex;
  size_t p2 = edge->next->vertex;

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
  
  std::cout << "Update Topology From Edges " << std::endl;
  std::cout << "Num Raw Half Edges : " << _halfEdges.size() << std::endl;
  std::cout << "Num Used HalfEdges : " << _usedEdges.size() << std::endl;

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

  REMOVE_EDGE_AVG_T.Reset();
  REMOVE_POINT_AVG_T.Reset();
}




JVR_NAMESPACE_CLOSE_SCOPE