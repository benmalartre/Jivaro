#ifndef JVR_GEOMETRY_TESSELATOR_H
#define JVR_GEOMETRY_TESSELATOR_H

#include <vector>
#include <utility>
#include <queue>
#include "../common.h"
#include "../geometry/halfEdge.h"
#include "../geometry/mesh.h"

JVR_NAMESPACE_OPEN_SCOPE


class Tesselator {
public:

  class _HalfEdgeLengthDivergence {
  public:
    _HalfEdgeLengthDivergence(Tesselator* tesselator)
      : _tesselator(tesselator) {}

    bool operator()(const HalfEdge* lhs, const HalfEdge* rhs) const
    {
      return _tesselator->CompareEdgeLengthDivergence(lhs, rhs);
    }

  private:
    Tesselator* _tesselator;
  };

  class _HalfEdgeLengthShorter {
  public:
    _HalfEdgeLengthShorter(Tesselator* tesselator)
      : _tesselator(tesselator) {}

    bool operator()(const HalfEdge* lhs, const HalfEdge* rhs) const
    {
      return _tesselator->CompareEdgeShorter(lhs, rhs);
    }

  private:
    Tesselator* _tesselator;
  };

  class _HalfEdgeLengthLonger {
  public:
    _HalfEdgeLengthLonger(Tesselator* tesselator)
      : _tesselator(tesselator) {}

    bool operator()(const HalfEdge* lhs, const HalfEdge* rhs) const
    {
      return _tesselator->CompareEdgeLonger(lhs, rhs);
    }

  private:
    Tesselator* _tesselator;
  };

  typedef std::priority_queue<HalfEdge*, std::vector<HalfEdge*>, _HalfEdgeLengthDivergence> HalfEdgePriorityQueue;

  Tesselator(Mesh* mesh);

  void Update(float l);

  void GetPositions(VtArray<GfVec3f>&) const;
  void GetTopology(VtArray<int>& faceCounts, VtArray<int>& faceConnects) const;
  const float DesiredLength() { return _length; };

  bool CompareEdgeLengthDivergence(const HalfEdge* lhs, const HalfEdge* rhs);
  bool CompareEdgeShorter(const HalfEdge* lhs, const HalfEdge* rhs);
  bool CompareEdgeLonger(const HalfEdge* lhs, const HalfEdge* rhs);

protected:
  float                      _GetLengthSq(const HalfEdge* edge);
  float                      _GetLength(const HalfEdge* edge);
  void                       _BuildSplitEdgeQueue(float l);
  void                       _BuildCollapseEdgeQueue(float l);
  void                       _ClearEdgeQueue();

  void                       _InitVertices();
  void                       _RemoveVertex(size_t index, size_t replace);
  void                       _ComputeGraph();
  bool                       _CollapseEdges();     
  bool                       _IsCollapsable(const HalfEdge* edge);
  bool                       _IsStarCollapsable(const HalfEdge* edge, float length);
  bool                       _IsFaceCollapsable(const HalfEdge* edge, float length);

private:
  HalfEdgePriorityQueue       _queue;
  Mesh*                       _input;
  HalfEdgeGraph               _graph;
  VtArray<GfVec3f>  _positions;
  VtArray<int>           _valences;

  float                       _length;


  friend                      HalfEdgeGraph;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_SUBDIV_H
