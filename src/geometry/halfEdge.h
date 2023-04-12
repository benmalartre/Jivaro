#ifndef JVR_GEOMETRY_HALFEDGE_H
#define JVR_GEOMETRY_HALFEDGE_H

#include <list>
#include <pxr/base/vt/array.h>
#include <pxr/base/tf/hashMap.h>

#include "../common.h"
#include "../geometry/triangle.h"
#include "../acceleration/mortom.h"

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

  const HalfEdge* GetLongestEdgeInTriangle(const pxr::GfVec3f* positions) const;

  inline size_t GetTriangleIndex() const {return index / 3;};

  bool operator <(const HalfEdge& other) const {
    if (twin && vertex > twin->vertex) {
      return twin->vertex < twin->next->vertex;
    }
    return vertex < next->vertex;
  }

  //HalfEdge* GetLongest(const pxr::GfVec3f* positions);
  /*
  short GetFlags(const pxr::GfVec3f* positions, const pxr::GfVec3f* normals, 
    const pxr::GfVec3f& v, float creaseValue) const;
  float GetWeight(const pxr::GfVec3f* positions, const pxr::GfVec3f* normals,
    const pxr::GfVec3f& v) const;
  */
};

class HalfEdgeGraph {
public:
  void ComputeGraph(Mesh* mesh);
  void ComputeNeighbors(Mesh* mesh);
  void ComputeTrianglePairs(Mesh* mesh);
  
  HalfEdge* GetLongestEdge(const pxr::GfVec3f* positions, short latency=HalfEdge::REAL);
  HalfEdge* GetShortestEdge(const pxr::GfVec3f* positions, short latency=HalfEdge::REAL);

  void SetAllEdgesLatencyReal();
  bool FlipEdge(HalfEdge* edge);
  bool SplitEdge(HalfEdge* edge, size_t numPoints);
  bool CollapseEdge(HalfEdge* edge);
  bool RemoveEdge(HalfEdge* edge);
  void RemovePoint(size_t index, size_t replace);
  void UpdateTopologyFromEdges(Mesh* mesh);

  size_t GetNumEdges(short latency=HalfEdge::REAL) const;
  void GetEdges(pxr::VtArray<HalfEdge*>&, short latency=HalfEdge::REAL);

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
  pxr::VtArray<HalfEdge>               _halfEdges;
  pxr::VtArray<HalfEdge*>              _usedEdges;

  pxr::VtArray<int>                    _vertexHalfEdge;
  pxr::VtArray<int>                    _triangleHalfEdge;
  pxr::VtArray<int>                    _faceHalfEdge;

  // vertex data
  pxr::VtArray<bool>                   _boundary;
  pxr::VtArray<int>                    _shell;
  pxr::VtArray< pxr::VtArray<int>>     _neighbors;

  HalfEdge*                            _shortest;
  HalfEdge*                            _longest;

  friend Mesh;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif
