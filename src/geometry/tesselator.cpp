#include <pxr/base/work/loops.h>
#include "../geometry/tesselator.h"

JVR_NAMESPACE_OPEN_SCOPE

Tesselator::Tesselator(Mesh* mesh)
  : _input(mesh)
  , _queue(HalfEdgePriorityQueue(this))
{
  _InitVertices();
  _ComputeGraph();
}

void 
Tesselator::_InitVertices()
{
  size_t numPoints = _input->GetNumPoints();
  _vertices.resize(numPoints);
  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    _Vertex& vertex = _vertices[pointIdx];
    vertex.valence = 0;
    vertex.position = _input->GetPosition(pointIdx);
  }
}

void
Tesselator::_RemoveVertexRange(size_t startIdx, size_t endIdx, size_t index, size_t replace)
{
  for (size_t vertexIdx = startIdx; vertexIdx < endIdx; ++vertexIdx) {
    _Vertex& vertex = _vertices[vertexIdx];
    pxr::VtArray<int>& neighbors = vertex.neighbors;
    for (size_t neighborIdx = 0; neighborIdx < neighbors.size() ; ++neighborIdx) {
      if (neighbors[neighborIdx] == index) neighbors[neighborIdx] = replace;
      else if (neighbors[neighborIdx] > index)neighbors[neighborIdx]--;
    }
  }
}

void 
Tesselator::_RemoveVertex(size_t index, size_t replace)
{
  pxr::WorkParallelForN(
    _vertices.size(),
    std::bind(&Tesselator::_RemoveVertexRange, this,
      std::placeholders::_1, std::placeholders::_2, index, replace)
  );
  _vertices.erase(_vertices.begin() + index);
}

void 
Tesselator::_ComputeGraph()
{
  size_t numVertices = _vertices.size();
  _graph.ComputeGraph(_input);
  for (size_t vertexIdx = 0; vertexIdx < numVertices; ++vertexIdx) {
    _Vertex* vertex = &_vertices[vertexIdx];
    std::cout << "compute graph " << vertexIdx << std::endl;
    HalfEdge* edge = _graph.GetEdgeFromVertex(vertexIdx);
    std::cout << "edge from vertex : " << edge << std::endl;
    _graph.ComputeNeighbors(edge, vertex->neighbors);
    std::cout << "vertex neighbors : " << vertex->neighbors << std::endl;
    vertex->valence = vertex->neighbors.size();
  }
}

float 
Tesselator::_GetLengthSq(const HalfEdge* edge)
{
  return (_vertices[_graph.GetEdge(edge->next)->vertex].position - 
  _vertices[edge->vertex].position).GetLengthSq();
}

float 
Tesselator::_GetLength(const HalfEdge* edge)
{
  return (_vertices[_graph.GetEdge(edge->next)->vertex].position - 
    _vertices[edge->vertex].position).GetLength();
}

bool 
Tesselator::CompareEdgeLengthDivergence(const HalfEdge* lhs, const HalfEdge* rhs)
{
  return (_GetLengthSq(lhs) - (_length * _length)) >=
    (_GetLengthSq(rhs) - (_length * _length));
}

bool 
Tesselator::CompareEdgeShorter(const HalfEdge* lhs, const HalfEdge* rhs)
{
  return _GetLengthSq(lhs) < _GetLengthSq(rhs);
}

bool
Tesselator::CompareEdgeLonger(const HalfEdge* lhs, const HalfEdge* rhs)
{
  return _GetLengthSq(lhs) > _GetLengthSq(rhs);
}

bool
Tesselator::_IsCollapsable(const HalfEdge* edge)
{
  if (edge->twin >= 0) {
    const HalfEdge* twin = _graph.GetEdge(edge->twin);
    const pxr::VtArray<int>& edgeNeighbors = _vertices[edge->vertex].neighbors;
    const pxr::VtArray<int>& twinNeighbors = _vertices[twin->vertex].neighbors;
    size_t commonNeighbors = 0;

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
Tesselator::_IsStarCollapsable(const HalfEdge* edge, float squaredLen)
{
  pxr::VtArray<int> neighbors;
  _graph.ComputeNeighbors(edge, neighbors);
std::cout << "neighbors : " << neighbors << std::endl;
  for (auto& neighbor : neighbors) {
    std::cout << squaredLen << " vs " << (_vertices[edge->vertex].position - _vertices[neighbor].position).GetLengthSq() << std::endl;
    if ((_vertices[edge->vertex].position - _vertices[neighbor].position).GetLengthSq() > squaredLen)
      return false;
  }
  return true;
}


void Tesselator::_BuildSplitEdgeQueue(float l)
{
  const float maxL = 4.f / 3.f * (l * l);
  _ClearEdgeQueue();
  HalfEdgeGraph::ItUniqueEdge it(&_graph);
  HalfEdge* edge = it.Next();
  while (edge) {
    if (_GetLengthSq(edge) > maxL) _queue.push(edge);
    edge = it.Next();
  }
}

void Tesselator::_BuildCollapseEdgeQueue(float minL)
{
  _ClearEdgeQueue();
  HalfEdgeGraph::ItUniqueEdge it(&_graph);
  HalfEdge* edge = it.Next();
  while (edge) {
    if (_graph.IsCollapsable(edge) && (_GetLengthSq(edge) < minL)) {
      _queue.push(edge);
    }
    edge = it.Next();
  }
}

void Tesselator::_ClearEdgeQueue()
{
  _queue = HalfEdgePriorityQueue(this);
}

bool Tesselator::_CollapseEdges()
{
  float collapseLen = 4.f / 5.f * (_length * _length);
  _BuildCollapseEdgeQueue(collapseLen);
  if (!_queue.size())return true;

  while (!_queue.empty())
  {
    HalfEdge* edge = _queue.top();
    if (!_graph.IsUsed(edge) || _GetLengthSq(edge) > collapseLen || !_graph.IsCollapsable(edge)) {
      _queue.pop(); continue;
    }
    if (_IsStarCollapsable(edge, collapseLen)) {
      std::cout << "star collapsable " << edge << std::endl;
      
      size_t vertex = edge->vertex;
      pxr::VtArray<int> neighbors;
      _graph.CollapseStar(edge, neighbors);

      pxr::GfVec3f average(0.f);
      for (auto& neighbor : neighbors) 
        average += _vertices[neighbor].position;
      average /= neighbors.size();
      _vertices[vertex].position = average;

      for (auto& neighbor : neighbors)
        _RemoveVertex(neighbor, vertex);
      
    } else {
      size_t p1 = edge->vertex;
      size_t p2 = _graph.GetEdge(edge->next)->vertex;
      if (_graph.CollapseEdge(edge)) {
        if (p1 > p2) {
          _vertices[p2].position = (_vertices[p1].position + _vertices[p2].position) * 0.5f;
          _RemoveVertex(p1, p2);
        }
        else {
          _vertices[p1].position = (_vertices[p1].position + _vertices[p2].position) * 0.5f;
          _RemoveVertex(p2, p1);
        }
      }
    }
    _queue.pop();
  }
  return false;
}


/* recepy
  1. Split all edges at their midpoint that are longer then 4/3 * l
  2. Collapse all edges shorter than 4/5 * l into their midpoint.
  3. Flip edges in order to minimize the deviation from valence 6 (or 4 on boundaries).
  4. Relocate vertices on the surface by tangential smoothing
*/
void Tesselator::Update(float l)
{
  _length = l;


  bool done = false;
  while (!done) {
    done = _CollapseEdges();
  }
  
}

void Tesselator::GetPositions(pxr::VtArray<pxr::GfVec3f>& positions) const
{
  size_t numVertices = _vertices.size();
  positions.resize(numVertices);
  for (size_t vertexIdx = 0; vertexIdx < numVertices; ++vertexIdx) {
    positions[vertexIdx] = _vertices[vertexIdx].position;
  }

}

void Tesselator::GetTopology(pxr::VtArray<int>& faceCounts, pxr::VtArray<int>& faceConnects) const
{
  _graph.ComputeTopology(faceCounts, faceConnects);
}


JVR_NAMESPACE_CLOSE_SCOPE