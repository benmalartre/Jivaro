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
  for (auto& edge : _graph.GetEdges()) {
    _Vertex& vertex = _vertices[edge->vertex];
    vertex.valence++;
    if (!vertex.edge)vertex.edge = edge;
    _queue.push(edge);
  }
}

bool Tesselator::CompareEdgeLengthDivergence(const HalfEdge* lhs, const HalfEdge* rhs)
{
  return (_graph.GetLength(lhs, &_positions[0]) - _length) <
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
  for (auto& edge : _graph.GetEdges())
    if (_graph.GetLength(edge, &_positions[0]) > maxL) _queue.push(edge);
}

void Tesselator::_BuildCollapseEdgeQueue(float l)
{
  const float minL = 4.f / 5.f * l;
  _ClearEdgeQueue();
  for (auto& edge : _graph.GetEdges()) {
    if (_graph.IsCollapsable(edge) && (_graph.GetLength(edge, &_positions[0]) < minL))
      _queue.push(edge);
  }
  
}

void Tesselator::_ClearEdgeQueue()
{
  _queue = HalfEdgePriorityQueue(this);
}

bool Tesselator::_CollapseEdges()
{
  std::cout << "collapse edges ..." << std::endl;
  float collapseLen = 4.f / 5.f * _length;

  _BuildCollapseEdgeQueue(_length);
  if (!_queue.size())return true;

  while (!_queue.empty())
  {
    HalfEdge* edge = _queue.top();
    if (_graph.IsUsed(edge) && _graph.GetLength(edge, &_positions[0]) < collapseLen) {
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

 
  std::cout << "num collapsable edges : " << _queue.size() << std::endl;
  bool done = false;
  while (!done) {
    done = _CollapseEdges();
    std::cout << "done ? " << done << std::endl;
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