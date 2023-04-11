// HalfEdge
//----------------------------------------------
#include "../geometry/halfEdge.h"
#include "../geometry/mesh.h"


JVR_NAMESPACE_OPEN_SCOPE

HalfEdge* HalfEdgeGraph::Get(size_t index) const
{
  return index < _uniqueEdges.size() ? _uniqueEdges[index] : NULL;
}

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
  for (auto& halfEdge : _halfEdges) {
    halfEdge->latency = HalfEdge::REAL;
  }
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
HalfEdgeGraph::_RemoveOneEdge(HalfEdge* edge, bool* modified)
{
  for (size_t idx = 0; idx < _halfEdges.size(); ++idx) {
    if (_halfEdges[idx] == edge) {
      
      size_t edgeIdx = edge->index;
      for (auto& halfEdge : _halfEdges) {
        if (halfEdge->index > edgeIdx) halfEdge->index--;
      }
      
      _halfEdges.erase(_halfEdges.begin() + idx);
      *modified = true;
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
}

void
HalfEdgeGraph::RemovePoint(size_t index, size_t replace)
{
  for (auto& halfEdge : _halfEdges) {
    if (halfEdge->vertex == index)halfEdge->vertex = replace;
    else if (halfEdge->vertex > index)halfEdge->vertex--;
  }
}

void 
HalfEdgeGraph::ComputeGraph(Mesh* mesh)
{
  size_t numTriangles = mesh->GetNumTriangles();
  _rawHalfEdges.resize(numTriangles * 3);
  _halfEdges.resize(numTriangles * 3);

  pxr::TfHashMap<uint64_t, HalfEdge*, pxr::TfHash> halfEdgesMap;

  size_t halfEdgeIdx = 0;
  size_t numFaceTriangles;
  size_t faceIdx = 0;
  size_t offsetIdx = 0;
  size_t triangleIdx = 0;
  size_t faceTriangleIdx = 0;

  HalfEdge* halfEdge = &_rawHalfEdges[0];
  pxr::VtArray<Triangle>& triangles = mesh->GetTriangles();

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
      halfEdgesMap[v1 | (v0 << 32)] = halfEdge;
      halfEdge->index = triangleIdx * 3;
      halfEdge->vertex = v0;
      halfEdge->next = halfEdge + 1;
      _SetHalfEdgeLatency(halfEdge, numFaceTriangles, faceTriangleIdx, 0);
      _halfEdges[halfEdge->index] = halfEdge;
      halfEdge++;

      // half-edge that goes from B to C:
      halfEdgesMap[v2 | (v1 << 32)] = halfEdge;
      halfEdge->index = triangleIdx * 3 + 1;
      halfEdge->vertex = v1;
      halfEdge->next = halfEdge + 1;
      _SetHalfEdgeLatency(halfEdge, numFaceTriangles, faceTriangleIdx, 1);
      _halfEdges[halfEdge->index] = halfEdge;
      halfEdge++;

      // half-edge that goes from C to A:
      halfEdgesMap[v0 | (v2 << 32)] = halfEdge;
      halfEdge->index = triangleIdx * 3 + 2;
      halfEdge->vertex = v2;
      halfEdge->next = halfEdge - 2;
      _SetHalfEdgeLatency(halfEdge, numFaceTriangles, faceTriangleIdx, 2);
      _halfEdges[halfEdge->index] = halfEdge;
      halfEdge++;

      triangleIdx++;
      faceTriangleIdx++;
    }
  }

  // verify that the mesh is clean:
  size_t numEntries = halfEdgesMap.size();
  bool problematic = false;
  if(numEntries != (size_t)(numTriangles * 3))problematic = true;

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
HalfEdgeGraph::ComputeUniqueEdges()
{
  _uniqueEdges.clear();
  _uniqueEdges.reserve(_halfEdges.size());
  size_t halfEdgeIndex = 0;
  for (HalfEdge* halfEdge : _halfEdges) {
    if (halfEdge->twin) {
      if (halfEdge->vertex < halfEdge->twin->vertex)
        _uniqueEdges.push_back(halfEdge);
    }
    else {
      _uniqueEdges.push_back(halfEdge);
    }
  }
}

HalfEdge* 
HalfEdgeGraph::GetLongestEdge(const pxr::GfVec3f* positions)
{
  float maxLength = 0.f;
  HalfEdge* longestEdge = _uniqueEdges[0];
  for (size_t uniqueEdgeIdx = 1; uniqueEdgeIdx < _uniqueEdges.size(); ++uniqueEdgeIdx) {
    HalfEdge* unique = _uniqueEdges[uniqueEdgeIdx];
    if (unique->latency != HalfEdge::REAL)continue;
    HalfEdge* next = unique->next;
    float edgeLength = (positions[next->vertex] - positions[unique->vertex]).GetLength();
    if (edgeLength > maxLength) {
      longestEdge = unique;
      maxLength = edgeLength;
    }
  }
  return longestEdge;
}

HalfEdge* 
HalfEdgeGraph::GetShortestEdge(const pxr::GfVec3f* positions)
{
  float minLength = std::numeric_limits<float>::max();
  HalfEdge* shortestEdge = _uniqueEdges[0];
  for (size_t uniqueEdgeIdx = 0; uniqueEdgeIdx < _uniqueEdges.size(); ++uniqueEdgeIdx) {
    HalfEdge* unique = _uniqueEdges[uniqueEdgeIdx];
    if (unique->latency != HalfEdge::REAL)continue;
    HalfEdge* next = unique->next;
    float edgeLength = (positions[next->vertex] - positions[unique->vertex]).GetLength();
    if (edgeLength < minLength) {
      shortestEdge = unique;
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

  for (HalfEdge* halfEdge : _halfEdges) {
    if (used[halfEdge->GetTriangleIndex()])continue;
    const HalfEdge* longest = GetLongestEdgeInTriangle(halfEdge, positions);
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

  for (HalfEdge* halfEdge : _halfEdges) {
    int edgeIndex = halfEdge->index;
    int vertexIndex = halfEdge->vertex;
    HalfEdge* startEdge = halfEdge;
    HalfEdge* currentEdge = startEdge;
    HalfEdge* nextEdge = NULL;
    pxr::VtArray<int>& neighbors = _neighbors[vertexIndex];
    if (!_IsNeighborRegistered(neighbors, startEdge->next->vertex)) {
      neighbors.push_back(startEdge->next->vertex);
    }

    short state = 1;
    while (state == 1) {
      nextEdge = _GetNextjacentEdge(currentEdge);
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
        previousEdge = _GetPreviousAdjacentEdge(currentEdge);
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
HalfEdgeGraph::SplitEdge(HalfEdge* edge, size_t numPoints)
{
  size_t numEdges = _halfEdges.size();
  if (edge->latency != HalfEdge::REAL)return false;

  HalfEdge* twin = edge->twin;
  if(twin) _halfEdges.resize(numEdges + 6);
  else _halfEdges.resize(numEdges + 3);

  HalfEdge* next = _GetNextEdge(edge, HalfEdge::REAL);
  HalfEdge* previous = _GetPreviousEdge(edge, HalfEdge::REAL);

  HalfEdge* halfEdge = _halfEdges[numEdges];
  halfEdge->index = numEdges;
  halfEdge->vertex = numPoints;
  halfEdge->next = next;
  halfEdge->latency = HalfEdge::REAL;
  HalfEdge* n1 = halfEdge;
  halfEdge++;

  halfEdge->index = numEdges + 1;
  halfEdge->vertex = numPoints;
  halfEdge->next = previous;
  halfEdge->latency = HalfEdge::IMPLICIT;
  HalfEdge* n2 = halfEdge;
  halfEdge++;

  halfEdge->index = numEdges + 2;
  halfEdge->vertex = numPoints;
  halfEdge->next = n2;
  halfEdge->latency = HalfEdge::IMPLICIT;
  HalfEdge* n3 = halfEdge;

  next->next = n3;
  edge->next = n1;

  numPoints++;
  
  if (twin) {
    HalfEdge* twinNext = _GetNextEdge(twin, HalfEdge::REAL);
    HalfEdge* twinPrevious = _GetPreviousEdge(twin, HalfEdge::REAL);

    halfEdge->index = numEdges + 3;
    halfEdge->vertex = numPoints;
    halfEdge->next = twinNext;
    halfEdge->latency = HalfEdge::REAL;
    HalfEdge* n4 = halfEdge;
    halfEdge++;

    halfEdge->index = numEdges + 4;
    halfEdge->vertex = numPoints;
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

    numPoints++;
  }
  return true;
}


bool 
HalfEdgeGraph::CollapseEdge(HalfEdge* edge)
{
  HalfEdge* twin = edge->twin;

  if (edge->latency != HalfEdge::REAL)return false;
  size_t p1 = edge->vertex;
  size_t p2 = edge->next->vertex;

  RemoveEdge(edge);
  if (twin) RemoveEdge(twin);

  if (p1 > p2) {
    RemovePoint(p1, p2);
  } else {
    RemovePoint(p2, p1);
  }

  ComputeUniqueEdges();
  return true;
}

void 
HalfEdgeGraph::UpdateTopologyFromEdges(Mesh* mesh)
{
  
  pxr::VtArray<int> faceCounts;
  pxr::VtArray<int> faceConnects;
  std::vector<int> visited(_rawHalfEdges.size(), 0);
  std::cout << "num edges : " << visited.size() << std::endl;
  HalfEdge* startEdge = NULL;
  for (auto& halfEdge: _halfEdges) {
    std::cout << halfEdge->index << ":" << visited[halfEdge->index] << std::endl;
    if (halfEdge->latency != HalfEdge::REAL || visited[halfEdge->index])continue;

    visited[halfEdge->index] = true;
    const HalfEdge* startEdge = halfEdge;
    std::cout << "start edge : " << startEdge << std::endl;
    HalfEdge* currentEdge = _GetNextEdge(startEdge, HalfEdge::REAL);
    std::cout << "current edge : " << currentEdge << std::endl;
    faceConnects.push_back(startEdge->vertex);
    size_t faceVertexCount = 1;
    while (currentEdge != startEdge) {
      visited[currentEdge->index] = true;
      faceConnects.push_back(currentEdge->vertex);
      faceVertexCount++;
      if (faceVertexCount > 6)break;
      currentEdge = _GetNextEdge(currentEdge, HalfEdge::REAL);
      std::cout << "current edge : " << currentEdge << std::endl;
    }
    std::cout << "push face " << faceVertexCount << std::endl;
    faceCounts.push_back(faceVertexCount);
  }
  mesh->SetTopology(faceCounts, faceConnects, false);
}




JVR_NAMESPACE_CLOSE_SCOPE