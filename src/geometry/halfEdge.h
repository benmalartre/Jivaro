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
  uint32_t                index;
  uint32_t                vertex;    // vertex index
  HalfEdge*               twin;      // opposite half-edge
  HalfEdge*               prev;      // previous half-edge  
  HalfEdge*               next;      // next half-edge

  HalfEdge() : vertex(0), twin(NULL), prev(NULL), next(NULL){};

  const HalfEdge* GetLongestEdgeInTriangle(const pxr::GfVec3f* positions) const;

  inline size_t GetTriangleIndex() const { return 0; };

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
  
  HalfEdge* GetLongestEdge(const pxr::GfVec3f* positions);
  HalfEdge* GetShortestEdge(const pxr::GfVec3f* positions);
  HalfEdge* GetRandomEdge();

  bool FlipEdge(HalfEdge* edge);
  bool SplitEdge(HalfEdge* edge, size_t numPoints);
  bool CollapseEdge(HalfEdge* edge);
  void RemoveEdge(HalfEdge* edge, bool* removed);
  void RemovePoint(size_t index, size_t replace);
  void UpdateTopologyFromEdges(Mesh* mesh);

  size_t GetNumEdges() const;
  const pxr::VtArray<HalfEdge*>& GetEdges();

  const pxr::VtArray<int>& GetVertexNeighbors(const HalfEdge* edge);

protected:
  HalfEdge* _GetPreviousAdjacentEdge(const HalfEdge* edge);
  HalfEdge* _GetNextAdjacentEdge(const HalfEdge* edge);
  HalfEdge* _GetNextEdge(const HalfEdge* edge);
  HalfEdge* _GetPreviousEdge(const HalfEdge* edge);
  HalfEdge* _FindInAdjacentEdges(const HalfEdge* edge, size_t endVertex);
  bool _IsTriangle(const HalfEdge* edge);
  bool _IsNeighborRegistered(const pxr::VtArray<int>& neighbors, int idx);
  void _RemoveOneEdge(HalfEdge* edge, bool* modified);
  void _ComputeVertexNeighbors(HalfEdge* edge, pxr::VtArray<int>& neighbors);

private:
  // half-edge data
  pxr::VtArray<HalfEdge>               _halfEdges;
  pxr::VtArray<HalfEdge*>              _usedEdges;

  pxr::VtArray<int>                    _triangleHalfEdge;
  pxr::VtArray<int>                    _faceHalfEdge;

  // vertex data
  pxr::VtArray<int>                    _vertexUsed;
  pxr::VtArray<HalfEdge*>              _vertexHalfEdge;
  pxr::VtArray<bool>                   _boundary;
  pxr::VtArray<int>                    _shell;
  pxr::VtArray< pxr::VtArray<int>>     _neighbors;

  HalfEdge*                            _shortest;
  HalfEdge*                            _longest;

  friend Mesh;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif
