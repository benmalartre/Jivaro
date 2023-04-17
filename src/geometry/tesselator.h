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
  struct _Vertex {
    size_t    valence;
    HalfEdge* edge;
  };

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

  size_t GetNumEdges();
  void Update(float l);

  const pxr::VtArray<pxr::GfVec3f>& GetPositions() const;
  const pxr::VtArray<int>& GetFaceCounts() const;
  const pxr::VtArray<int>& GetFaceConnects() const;
  const float DesiredLength() { return _length; };

  bool CompareEdgeLengthDivergence(const HalfEdge* lhs, const HalfEdge* rhs);
  bool CompareEdgeShorter(const HalfEdge* lhs, const HalfEdge* rhs);
  bool CompareEdgeLonger(const HalfEdge* lhs, const HalfEdge* rhs);

protected:
  void                       _BuildSplitEdgeQueue(float l);
  void                       _BuildCollapseEdgeQueue(float l);
  void                       _ClearEdgeQueue();

  void                       _InitVertices();
  void                       _RemoveVertex(size_t index);
  void                       _ComputeGraph();
  bool                       _CollapseEdges();            

private:
  HalfEdgePriorityQueue       _queue;
  Mesh*                       _input;
  HalfEdgeGraph               _graph;
  std::vector<_Vertex>        _vertices;

  pxr::VtArray<int>           _faceCounts;
  pxr::VtArray<int>           _faceConnects;
  pxr::VtArray<pxr::GfVec3f>  _positions;

  float                       _length;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_SUBDIV_H
