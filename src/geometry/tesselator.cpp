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
  _positions.resize(numPoints);
  _valences.resize(numPoints);
  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    _positions[pointIdx] = _input->GetPosition(pointIdx);
    _valences[pointIdx] = 0;
  }
}

void 
Tesselator::_RemoveVertex(size_t index, size_t replace)
{
  _positions.erase(_positions.begin() + index);
  _valences.erase(_valences.begin() + index);
}

void 
Tesselator::_ComputeGraph()
{
  size_t numVertices = _positions.size();
  _graph.ComputeGraph(_input);
  for (size_t vertexIdx = 0; vertexIdx < numVertices; ++vertexIdx) {
    _valences[vertexIdx]++;
  }
}

float 
Tesselator::_GetLengthSq(const HalfEdge* edge)
{
  return (_positions[_graph.GetEdge(edge->next)->vertex] - 
    _positions[edge->vertex]).GetLengthSq();
}

float 
Tesselator::_GetLength(const HalfEdge* edge)
{
  return (_positions[_graph.GetEdge(edge->next)->vertex] - 
    _positions[edge->vertex]).GetLength();
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
  /*
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
  */
  return true;
}

bool
Tesselator::_IsStarCollapsable(const HalfEdge* edge, float squaredLen)
{
  if (!_graph.IsUsed(edge))return false;
  pxr::VtArray<int> neighbors;
  _graph.ComputeNeighbors(edge, neighbors);
  for (auto& neighbor : neighbors) {
    if ((_positions[edge->vertex] - _positions[neighbor]).GetLengthSq() > squaredLen)
      return false;
  }
  return true;
}

bool
Tesselator::_IsFaceCollapsable(const HalfEdge* edge, float squaredLen)
{
  if (!_graph.IsUsed(edge))return false;
  pxr::VtArray<int> neighbors;
  pxr::GfVec3f start = _positions[edge->vertex];
  const HalfEdge* current = _graph.GetEdge(edge->next);
  while (current != edge) {
    pxr::GfVec3f end = _positions[current->vertex];
    if ((end - start).GetLengthSq() > squaredLen)return false;
    start = end;
    current = _graph.GetEdge(current->next);
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
    
    //if (_IsStarCollapsable(edge, collapseLen)) {
    if (_IsFaceCollapsable(edge, collapseLen)) {
      std::cout << "face collapsable " << edge << std::endl;
      
      size_t vertex = edge->vertex;
      pxr::VtArray<int> neighbors;
      if (_graph.CollapseFace(edge, neighbors)) {

        std::cout << "num neighbors : " << neighbors.size() << std::endl;
        pxr::GfVec3f average(0.f);
        for (auto& neighbor : neighbors)
          average += _positions[neighbor];
        average += _positions[vertex];
        average /= (neighbors.size() + 1);
        std::cout << "average position : " << average << std::endl;


        size_t lowest = neighbors.back();
        if (vertex > lowest) {
          std::cout << "keep vertex : " << lowest << std::endl;
          std::cout << "remove vertices : ";
          for (size_t neighborIdx = 0; neighborIdx < neighbors.size() - 1; ++neighborIdx) {
            _RemoveVertex(neighbors[neighborIdx], lowest);
            std::cout << neighbors[neighborIdx] << ",";
          }
          std::cout << std::endl;
          _RemoveVertex(vertex, lowest);
          _positions[lowest] = average;
        }
        
        else {
          std::cout << "keep vertex : " << vertex << std::endl;
          std::cout << "remove vertices : " << neighbors << std::endl;
          _positions[vertex] = average;
          for (auto& neighbor : neighbors)
            _RemoveVertex(neighbor, vertex);
        }
        std::cout << "vertices removed" << std::endl;
      }
    } else {
    
      size_t p1 = edge->vertex;
      size_t p2 = _graph.GetEdge(edge->next)->vertex;
      if (_graph.CollapseEdge(edge)) {
        if (p1 > p2) {
          _positions[p2] = (_positions[p1] + _positions[p2]) * 0.5f;
          _RemoveVertex(p1, p2);
        }
        else {
          _positions[p1] = (_positions[p1] + _positions[p2]) * 0.5f;
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
  size_t numVertices = _positions.size();
  positions.resize(numVertices);
  for (size_t vertexIdx = 0; vertexIdx < numVertices; ++vertexIdx) {
    positions[vertexIdx] = _positions[vertexIdx];
  }

}

void Tesselator::GetTopology(pxr::VtArray<int>& faceCounts, pxr::VtArray<int>& faceConnects) const
{
  _graph.ComputeTopology(faceCounts, faceConnects);
}


JVR_NAMESPACE_CLOSE_SCOPE