#ifndef JVR_GEOMETRY_HALFEDGE_H
#define JVR_GEOMETRY_HALFEDGE_H

#include <pxr/base/vt/array.h>
#include "../common.h"
#include "../geometry/triangle.h"

JVR_NAMESPACE_OPEN_SCOPE

class Mesh;
struct HalfEdge
{
  enum Latency {
    REAL,
    IMPLICIT,
    VIRTUAL,
    ANY
  };

  uint32_t                index;     // half edge index
  uint32_t                vertex;    // vertex index
  uint8_t                 latency;   // edge latency

  HalfEdge*               twin;      // opposite half-edge
  HalfEdge*               next;      // next half-edge

  HalfEdge():vertex(0), twin(NULL), next(NULL),latency(VIRTUAL){};
  inline size_t GetTriangleIndex() const {return index / 3;};

  //HalfEdge* GetLongest(const pxr::GfVec3f* positions);
  /*
  short GetFlags(const pxr::GfVec3f* positions, const pxr::GfVec3f* normals, 
    const pxr::GfVec3f& v, float creaseValue) const;
  float GetWeight(const pxr::GfVec3f* positions, const pxr::GfVec3f* normals,
    const pxr::GfVec3f& v) const;
  */
};

class HalfEdgeGraph {
  friend Mesh;
public:
  void ComputeGraph(Mesh* mesh);
  void ComputeNeighbors(Mesh* mesh);
  void ComputeTrianglePairs(Mesh* mesh);
  void ComputeUniqueEdges();
  void RemoveUniqueEdge(HalfEdge* edge);
  void AddUniqueEdge(HalfEdge* edge);

  const HalfEdge* GetLongestEdgeInTriangle(const HalfEdge* edge,
    const pxr::GfVec3f* positions);
  
  HalfEdge* GetLongestEdge(const pxr::GfVec3f* positions);
  HalfEdge* GetShortestEdge(const pxr::GfVec3f* positions);

  void SetAllEdgesLatencyReal();
  bool FlipEdge(HalfEdge* edge);
  bool SplitEdge(HalfEdge* edge, size_t numPoints);
  bool CollapseEdge(HalfEdge* edge);
  bool RemoveEdge(HalfEdge* edge);
  void RemovePoint(size_t index, size_t replace);
  void UpdateTopologyFromEdges(Mesh* mesh);

  HalfEdge* Get(size_t index) const;
  size_t GetNumEdges() const { return _uniqueEdges.size(); };
  size_t GetNumHalfEdges() const { return _halfEdges.size(); };
  pxr::VtArray<HalfEdge*>& GetUniqueEdges() { return _uniqueEdges; };
  pxr::VtArray<HalfEdge*>& GetEdges() { return _halfEdges; };

protected:
  HalfEdge* _GetPreviousAdjacentEdge(HalfEdge* edge);
  HalfEdge* _GetNextjacentEdge(HalfEdge* edge);
  HalfEdge* _GetNextEdge(const HalfEdge* edge, short latency = HalfEdge::ANY);
  HalfEdge* _GetPreviousEdge(const HalfEdge* edge, short latency = HalfEdge::ANY);
  void _SetHalfEdgeLatency(HalfEdge* halfEdge, int numFaceTriangles, 
    int faceTriangleIndex, int triangleEdgeIndex);
  bool _IsNeighborRegistered(const pxr::VtArray<int>& neighbors, int idx);
  void _RemoveOneEdge(HalfEdge* edge, bool* modified);

private:
  // half-edge data
  pxr::VtArray<HalfEdge>               _rawHalfEdges;
  pxr::VtArray<HalfEdge*>              _halfEdges;
  pxr::VtArray<HalfEdge*>              _uniqueEdges;
  pxr::VtArray<int>                    _vertexHalfEdge;
  pxr::VtArray<int>                    _triangleHalfEdge;
  pxr::VtArray<int>                    _faceHalfEdge;

  // vertex data
  pxr::VtArray<bool>                   _boundary;
  pxr::VtArray<int>                    _shell;
  pxr::VtArray< pxr::VtArray<int>>     _neighbors;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif
