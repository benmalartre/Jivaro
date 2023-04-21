#include "../geometry/tesselator.h"

JVR_NAMESPACE_OPEN_SCOPE

Tesselator::Tesselator(Mesh* mesh)
  : _input(mesh)
  , _queue(HalfEdgePriorityQueue(this))
{
  _InitVertices();
  _ComputeGraph();
}

void Tesselator::_InitVertices()
{
  size_t numPoints = _input->GetNumPoints();
  _vertices.resize(numPoints);
  _positions.resize(numPoints);
  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    _vertices[pointIdx].valence = 0;
    _vertices[pointIdx].edge = NULL;
    _positions[pointIdx] = _input->GetPosition(pointIdx);
  }
}

void Tesselator::_RemoveVertex(size_t index)
{
  _vertices.erase(_vertices.begin() + index);
  _positions.erase(_positions.begin() + index);
}

void Tesselator::_ComputeGraph()
{
  _graph.ComputeGraph(_input);
  HalfEdgeGraph::ItUniqueEdge it(&_graph);
  HalfEdge* edge = it.Next();
  while(edge) {
    _Vertex& vertex = _vertices[edge->vertex];
    vertex.valence++;
    if (!vertex.edge)vertex.edge = edge;
    _queue.push(edge);
    edge = it.Next();
  }
}

bool Tesselator::CompareEdgeLengthDivergence(const HalfEdge* lhs, const HalfEdge* rhs)
{
  return (_graph.GetLength(lhs, &_positions[0]) - _length) >
    (_graph.GetLength(rhs, &_positions[0]) - _length);
}

bool Tesselator::CompareEdgeShorter(const HalfEdge* lhs, const HalfEdge* rhs)
{
  return _graph.GetLength(lhs, &_positions[0]) <
    _graph.GetLength(rhs, &_positions[0]);
}

bool Tesselator::CompareEdgeLonger(const HalfEdge* lhs, const HalfEdge* rhs)
{
  return _graph.GetLength(lhs, &_positions[0]) >
    _graph.GetLength(rhs, &_positions[0]);
}

size_t Tesselator::GetNumEdges()
{
  return _graph.GetNumEdges();
}


void Tesselator::_BuildSplitEdgeQueue(float l)
{
  const float maxL = 4.f / 3.f * l;
  _ClearEdgeQueue();
  HalfEdgeGraph::ItUniqueEdge it(&_graph);
  HalfEdge* edge = it.Next();
  while (edge) {
    if (_graph.GetLength(edge, &_positions[0]) > maxL) _queue.push(edge);
    edge = it.Next();
  }
}

void Tesselator::_BuildCollapseEdgeQueue(float minL)
{
  _ClearEdgeQueue();
  HalfEdgeGraph::ItUniqueEdge it(&_graph);
  HalfEdge* edge = it.Next();
  std::cout << "build collapse edge queue : " << edge << std::endl;
  while (edge) {
    if (_graph.IsCollapsable(edge) && (_graph.GetLength(edge, &_positions[0]) < minL)) {
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
  float collapseLen = 4.f / 5.f * _length;
  _BuildCollapseEdgeQueue(collapseLen);
  if (!_queue.size())return true;

  std::cout << "queue size : " << _queue.size() << std::endl;

  while (!_queue.empty())
  {
    HalfEdge* edge = _queue.top();
    if (!_graph.IsUsed(edge) || _graph.GetLength(edge, &_positions[0]) > collapseLen || !_graph.IsCollapsable(edge)) {
      _queue.pop(); continue;
    }
    size_t p1 = edge->vertex;
    size_t p2 = _graph.GetEdge(edge->next)->vertex;
    if (_graph.CollapseEdge(edge)) {
      if (p1 > p2) {
        _positions[p2] = (_positions[p1] + _positions[p2]) * 0.5f;
        _RemoveVertex(p1);
      }
      else {
        _positions[p1] = (_positions[p1] + _positions[p2]) * 0.5f;
        _RemoveVertex(p2);
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
 
  _graph.UpdateTopologyFromEdges(_faceCounts, _faceConnects);
  
}

const pxr::VtArray<pxr::GfVec3f>& Tesselator::GetPositions() const
{
  return _positions;
}

const pxr::VtArray<int>& Tesselator::GetFaceCounts() const
{
  return _faceCounts;
}

const pxr::VtArray<int>& Tesselator::GetFaceConnects() const
{
  return _faceConnects;
}

JVR_NAMESPACE_CLOSE_SCOPE