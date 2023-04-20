#ifndef JVR_GEOMETRY_HALFEDGE_H
#define JVR_GEOMETRY_HALFEDGE_H

#include <queue>
#include <iterator>
#include <pxr/base/vt/array.h>
#include <pxr/base/tf/hashMap.h>

#include "../common.h"
#include "../acceleration/mortom.h"

JVR_NAMESPACE_OPEN_SCOPE

class Mesh;
struct HalfEdge
{
  int vertex;    // vertex index
  int twin;      // opposite half-edge
  int prev;      // previous half-edge  
  int next;      // next half-edge

  HalfEdge() : vertex(-1), twin(-1), prev(-1), next(-1){};
};

class HalfEdgeGraph {
public:
 
  struct ItUniqueEdge {
    HalfEdgeGraph*      edges;
    int                 index;
    
    ItUniqueEdge(HalfEdgeGraph* graph);
    HalfEdge* Next();
  };

  void ComputeGraph(Mesh* mesh);
  void ComputeNeighbors(Mesh* mesh);

  void AllocateEdges(size_t num);
  bool FlipEdge(HalfEdge* edge);
  bool SplitEdge(HalfEdge* edge, size_t numPoints);
  bool CollapseEdge(HalfEdge* edge);
  void RemoveEdge(HalfEdge* edge, bool* removed);
  void RemovePoint(size_t index, size_t replace);
  void UpdateTopologyFromEdges(pxr::VtArray<int>& faceCounts, pxr::VtArray<int>& faceConnects);
  bool IsCollapsable(const HalfEdge* edge);
  bool IsUnique(const HalfEdge* edge) const;
  bool IsUsed(const HalfEdge* edge) const;

  size_t GetNumEdges() const;
  HalfEdge* GetEdge(int index);
  HalfEdge* GetAvailableEdge();
  pxr::VtArray<HalfEdge>& GetEdges();
  HalfEdge* GetEdgeFromVertices(size_t start, size_t end);
  const HalfEdge* GetEdgeFromVertices(size_t start, size_t end) const;

  const pxr::VtArray<int>& GetVertexNeighbors(const HalfEdge* edge);

  const HalfEdge* GetLongestEdgeInTriangle(const HalfEdge* edge, const pxr::GfVec3f* positions) const;
  float GetLength(const HalfEdge* edge, const pxr::GfVec3f* positions) const;
  float GetLengthSq(const HalfEdge* edge, const pxr::GfVec3f* positions) const;

  //HalfEdge* GetLongest(const pxr::GfVec3f* positions);
  /*
  short GetFlags(const pxr::GfVec3f* positions, const pxr::GfVec3f* normals,
    const pxr::GfVec3f& v, float creaseValue) const;
  float GetWeight(const pxr::GfVec3f* positions, const pxr::GfVec3f* normals,
    const pxr::GfVec3f& v) const;
  */

protected:
  HalfEdge* _GetPreviousAdjacentEdge(const HalfEdge* edge);
  HalfEdge* _GetNextAdjacentEdge(const HalfEdge* edge);
  HalfEdge* _GetNextEdge(const HalfEdge* edge);
  HalfEdge* _GetPreviousEdge(const HalfEdge* edge);
  HalfEdge* _FindInAdjacentEdges(const HalfEdge* edge, size_t endVertex);
  bool _FindInAvailableEdges(const HalfEdge* edge);
  bool _IsTriangle(const HalfEdge* edge);
  void _TriangulateFace(const HalfEdge* edge);
  
  bool _IsNeighborRegistered(const pxr::VtArray<int>& neighbors, int idx);
  void _RemoveOneEdge(const HalfEdge* edge, bool* modified);
  void _ComputeVertexNeighbors(const HalfEdge* edge, pxr::VtArray<int>& neighbors);
  size_t _GetEdgeIndex(const HalfEdge* edge) const;
  size_t _GetFaceVerticesCount(const HalfEdge* edge);


private:
  // half-edge data
  pxr::VtArray<bool>                   _halfEdgeUsed;
  pxr::VtArray<HalfEdge>               _halfEdges;
  pxr::VtArray<HalfEdge*>              _availableEdges;

  // vertex data
  pxr::VtArray<HalfEdge*>              _vertexHalfEdge;
  pxr::VtArray<bool>                   _boundary;
  pxr::VtArray<int>                    _shell;
  pxr::VtArray< pxr::VtArray<int>>     _neighbors;

  friend Mesh;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif
