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


const HalfEdge* 
HalfEdgeGraph::GetLongestEdgeInTriangle(const HalfEdge* edge, const pxr::GfVec3f* positions)
{
  const HalfEdge* next = edge->next;
  const HalfEdge* prev = next->next;
  const float edge0 = (positions[next->vertex] - positions[edge->vertex]).GetLength();
  const float edge1 = (positions[prev->vertex] - positions[next->vertex]).GetLength();
  const float edge2 = (positions[edge->vertex] - positions[prev->vertex]).GetLength();
  if (edge0 > edge1 && edge0 > edge2)return edge;
  else if (edge1 > edge0 && edge1 > edge2)return next;
  else return prev;
}

void HalfEdgeGraph::SetAllEdgesLatencyReal()
{
  for (auto& usedEdge : _usedEdges) {
    usedEdge->latency = HalfEdge::REAL;
  }
}


size_t 
HalfEdgeGraph::GetNumEdges(short latency) const
{
  size_t numEdges = 0;
  if (latency == HalfEdge::ANY)
    return _usedEdges.size();
  else
    for (auto& usedEdge : _usedEdges)
      if (usedEdge->latency == latency)numEdges++;

  return numEdges;
}

void 
HalfEdgeGraph::GetEdges(pxr::VtArray<HalfEdge*>& edges, short latency)
{
  edges.clear();
  edges.reserve(_usedEdges.size());
  if (latency == HalfEdge::ANY)
    for (auto& usedEdge : _usedEdges)
      edges.push_back(usedEdge);
  else
    for (auto& usedEdge : _usedEdges)
      if (usedEdge->latency == latency)edges.push_back(usedEdge);
}

void 
HalfEdgeGraph::_SetHalfEdgeLatency(HalfEdge* halfEdge, int numFaceTriangles, 
  int faceTriangleIndex, int triangleEdgeIndex)
{
  if (numFaceTriangles == 1) {
    halfEdge->latency = HalfEdge::REAL;
  } else if (faceTriangleIndex == 0) {
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

void 
HalfEdgeGraph::ComputeGraph(Mesh* mesh)
{
  const size_t numPoints = mesh->GetNumPoints();
  const size_t numTriangles = mesh->GetNumTriangles();
  const size_t numHalfEdges = numTriangles * 3;
  _halfEdges.resize(numHalfEdges);
  _usedEdges.resize(numHalfEdges);

  size_t numFaceTriangles;
  size_t faceIdx = 0;
  size_t offsetIdx = 0;
  size_t triangleIdx = 0;
  size_t faceTriangleIdx = 0;


  HalfEdge* halfEdge = &_halfEdges[0];
  pxr::VtArray<Triangle>& triangles = mesh->GetTriangles();
  pxr::TfHashMap<uint64_t, HalfEdge*, pxr::TfHash> halfEdgesMap;

  for (const auto& faceVertexCount: mesh->GetFaceCounts())
  { 
    numFaceTriangles = faceVertexCount - 2;
    faceTriangleIdx = 0;
    for (int faceTriangle = 0; faceTriangle < numFaceTriangles; ++faceTriangle)
    {
      const Triangle* tri = &triangles[triangleIdx];
      uint64_t v0 = tri->vertices[0];
      uint64_t v1 = tri->vertices[1];
      uint64_t v2 = tri->vertices[2];

      // half-edge that goes from A to B:
      HalfEdge* h0 = halfEdge;
      halfEdgesMap[v1 | (v0 << 32)] = h0;
      h0->index = triangleIdx * 3;
      h0->vertex = tri->vertices[0];
      h0->next = halfEdge + 1;
      _usedEdges[h0->index] = h0;
      _SetHalfEdgeLatency(h0, numFaceTriangles, faceTriangleIdx, 0);
      halfEdge++;

      // half-edge that goes from B to C:
      HalfEdge* h1 = halfEdge;
      halfEdgesMap[v2 | (v1 << 32)] = h1;
      h1->index = triangleIdx * 3 + 1;
      h1->vertex = tri->vertices[1];
      h1->next = halfEdge + 1;
      _usedEdges[h1->index] = h1;
      _SetHalfEdgeLatency(h1, numFaceTriangles, faceTriangleIdx, 1);
      halfEdge++;

      // half-edge that goes from C to A:
      HalfEdge* h2 = halfEdge;
      halfEdgesMap[v0 | (v2 << 32)] = h2;
      h2->index = triangleIdx * 3 + 2;
      h2->vertex = tri->vertices[2];
      h2->next = halfEdge - 2;
      _usedEdges[h2->index] = h2;
      _SetHalfEdgeLatency(h2, numFaceTriangles, faceTriangleIdx, 2);
      halfEdge++;

      triangleIdx++;
      faceTriangleIdx++;
    }
  }

  // verify that the mesh is clean:
  size_t numEntries = _usedEdges.size();
  bool problematic = false;
  if(numEntries != (size_t)(numTriangles * 3))problematic = true;

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

bool
HalfEdgeGraph::RemoveEdge(HalfEdge* edge)
{
  bool modified = false;
  HalfEdge* previous = _GetPreviousEdge(edge);
  HalfEdge* next = _GetNextEdge(edge);

  if (previous->twin) {
    previous->twin->twin = next->twin;
    previous->twin->latency = HalfEdge::REAL;
  }
  if (next->twin) {
    next->twin->twin = previous->twin;
    next->twin->latency = HalfEdge::REAL;
  }

  _RemoveOneEdge(edge, &modified);
  _RemoveOneEdge(previous, &modified);
  _RemoveOneEdge(next, &modified);
  return modified;
}

void
HalfEdgeGraph::RemovePoint(size_t index, size_t replace)
{
  for (auto& edge: _usedEdges) {
    if (edge->vertex == index)edge->vertex = replace;
    else if (edge->vertex > index)edge->vertex--;
  }
}

HalfEdge* 
HalfEdgeGraph::GetLongestEdge(const pxr::GfVec3f* positions, short latency)
{
  float maxLength = 0.f;
  HalfEdge* longestEdge = NULL;
  for (auto& used: _usedEdges) {
    if (latency != HalfEdge::ANY && used->latency != latency) continue;
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
HalfEdgeGraph::GetShortestEdge(const pxr::GfVec3f* positions, short latency)
{
  float minLength = std::numeric_limits<float>::max();
  HalfEdge* shortestEdge = NULL;
  for (auto& used: _usedEdges) {
    if (latency != HalfEdge::ANY && used->latency != latency)continue;
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
    const HalfEdge* longest = GetLongestEdgeInTriangle(edge, positions);
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
HalfEdgeGraph::_GetPreviousAdjacentEdge(HalfEdge* edge)
{
  int vertex = edge->vertex;
  HalfEdge* current = edge->next;
  while (current->next->vertex != vertex) {
    current = current->next;
  }
  if (current->twin)
    return current->twin->next;
  return NULL;
}

HalfEdge* 
HalfEdgeGraph::_GetNextjacentEdge(HalfEdge* edge)
{
  if (edge->twin)
    return edge->twin->next;
  return NULL;

}

HalfEdge* 
HalfEdgeGraph::_GetNextEdge(const HalfEdge* edge, short latency)
{
  HalfEdge* next = edge->next;
  switch (latency) {
    case HalfEdge::REAL:
      if (next->latency == HalfEdge::REAL)
        return next;
      else if(next->twin) 
        return _GetNextEdge(next->twin, HalfEdge::REAL);
      else return next;
    case HalfEdge::IMPLICIT:
    case HalfEdge::VIRTUAL:
    case HalfEdge::ANY:
      return next;
    default:
      return next;
  }
}

HalfEdge* 
HalfEdgeGraph::_GetPreviousEdge(const HalfEdge* edge, short latency)
{
  int vertex = edge->vertex;
  HalfEdge* current = edge->next;
  HalfEdge* previous = NULL;
  switch (latency) {
    case HalfEdge::REAL:
      previous = _GetPreviousEdge(edge);
      if (previous->latency == HalfEdge::REAL)
        return previous;
      else if (previous->twin) 
        return _GetPreviousEdge(previous->twin, HalfEdge::REAL);
      else
        return previous;
    case HalfEdge::IMPLICIT:
    case HalfEdge::VIRTUAL:
    case HalfEdge::ANY:
      while (current->next->vertex != vertex)
        current = current->next;
      return current;
    default:
      while (current->next->vertex != vertex)
        current = current->next;
      return current;
  }
  
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

  for (auto& edge: _usedEdges) {
    HalfEdge* start = edge;
    HalfEdge* current = start;
    HalfEdge* next = NULL;
    pxr::VtArray<int>& neighbors = _neighbors[edge->vertex];
    if (!_IsNeighborRegistered(neighbors, start->next->vertex)) {
      neighbors.push_back(start->next->vertex);
    }

    short state = 1;
    while (state == 1) {
      next = _GetNextjacentEdge(current);
      if (!next) {
        state = 0;
      } else if (next == start) {
        state = -1;
      } else {
        if (!_IsNeighborRegistered(neighbors, next->next->vertex)) {
          neighbors.push_back(next->next->vertex);
        }
        current = next;
      }
    }
    
    if(state == 0) {
      HalfEdge* previous = NULL;
      current = start;
      previous = _GetPreviousEdge(current);
      if (!_IsNeighborRegistered(neighbors, previous->vertex)) {
        neighbors.push_back(previous->vertex);
      }
      state = 1;
      while (state == 1) {
        previous = _GetPreviousAdjacentEdge(current);
        if (!previous) {
          state = 0;
        }
        else if (previous == start) {
          state = -1;
        }
        else {
          if (!_IsNeighborRegistered(neighbors, previous->next->vertex)) {
            neighbors.push_back(previous->next->vertex);
          }
          current = previous;
        }
      }
    }
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

  if (edge->latency != HalfEdge::REAL)return false;
  size_t p1 = edge->vertex;
  size_t p2 = edge->next->vertex;

  REMOVE_EDGE_AVG_T.Start();
  RemoveEdge(edge);
  REMOVE_EDGE_AVG_T.End();

  if (twin) {
    REMOVE_EDGE_AVG_T.Start();
    RemoveEdge(twin);
    REMOVE_EDGE_AVG_T.End();
  }

  if (p1 > p2) {
    REMOVE_POINT_AVG_T.Start();
    RemovePoint(p1, p2);
    REMOVE_POINT_AVG_T.End();
  } else {
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
  std::vector<int> visited(_halfEdges.size(), 0);
  for (auto& edge: _usedEdges) {
    if (edge->latency != HalfEdge::REAL || visited[edge->index])continue;

    visited[edge->index] = true;
    const HalfEdge* start = edge;
    HalfEdge* current = _GetNextEdge(start, HalfEdge::REAL);
    faceConnects.push_back(start->vertex);
    size_t faceVertexCount = 1;
    while (current != start) {
      visited[current->index] = true;
      faceConnects.push_back(current->vertex);
      faceVertexCount++;
      current = _GetNextEdge(current, HalfEdge::REAL);
    }
    faceCounts.push_back(faceVertexCount);
  }
  mesh->SetTopology(faceCounts, faceConnects, false);

  std::cout << "remove edge time : " << REMOVE_EDGE_AVG_T.Elapsed() << std::endl;
  std::cout << "remove point time : " << REMOVE_POINT_AVG_T.Elapsed() << std::endl;

  REMOVE_EDGE_AVG_T.Reset();
  REMOVE_POINT_AVG_T.Reset();
}




JVR_NAMESPACE_CLOSE_SCOPE