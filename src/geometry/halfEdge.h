#ifndef JVR_GEOMETRY_HALFEDGE_H
#define JVR_GEOMETRY_HALFEDGE_H

#include <queue>
#include <iterator>
#include <pxr/base/vt/array.h>
#include <pxr/base/tf/hashmap.h>

#include "../common.h"
#include "../acceleration/morton.h"

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
  void ComputeNeighbors();
  void ComputeNeighbors(const HalfEdge* edge, pxr::VtArray<int>& neighbors);
  void ComputeTopology(pxr::VtArray<int>& faceCounts, pxr::VtArray<int>& faceConnects) const;

  void AllocateEdges(size_t num);
  bool FlipEdge(HalfEdge* edge);
  bool SplitEdge(HalfEdge* edge, size_t numPoints);
  bool CollapseEdge(HalfEdge* edge);
  bool CollapseFace(HalfEdge* edge, pxr::VtArray<int>& vertices);
  bool CollapseStar(HalfEdge* edge, pxr::VtArray<int>& neighbors);
  void RemoveEdge(HalfEdge* edge, bool* removed);
  void RemovePoint(size_t index, size_t replace);
  
  bool IsCollapsable(const HalfEdge* edge);
  bool IsUnique(const HalfEdge* edge) const;
  bool IsUsed(const HalfEdge* edge) const;

  size_t GetNumRawEdges() const;
  size_t GetNumEdges() const;
  HalfEdge* GetEdge(int index);
  HalfEdge* GetAvailableEdge();
  pxr::VtArray<HalfEdge>& GetEdges();
  pxr::VtArray<pxr::VtArray<int>>& GetNeighbors();
  HalfEdge* GetEdgeFromVertex(size_t vertex);
  HalfEdge* GetEdgeFromVertices(size_t start, size_t end);
  const HalfEdge* GetEdgeFromVertices(size_t start, size_t end) const;
  void GetEdgeFromFace(const HalfEdge* edge, pxr::VtArray<int>& indices);

  size_t GetLongestEdgeInTriangle(const pxr::GfVec3i& vertices, const pxr::GfVec3f* positions) const;
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
  const HalfEdge* _GetPreviousAdjacentEdge(const HalfEdge* edge) const;
  HalfEdge* _GetNextAdjacentEdge(const HalfEdge* edge);
  const HalfEdge* _GetNextAdjacentEdge(const HalfEdge* edge) const;
  HalfEdge* _GetNextEdge(const HalfEdge* edge);
  const HalfEdge* _GetNextEdge(const HalfEdge* edge) const;
  HalfEdge* _GetPreviousEdge(const HalfEdge* edge);
  const HalfEdge* _GetPreviousEdge(const HalfEdge* edge) const;
  HalfEdge* _FindInAdjacentEdges(const HalfEdge* edge, size_t endVertex);
  bool _IsTriangle(const HalfEdge* edge) const;
  void _TriangulateFace(const HalfEdge* edge);
  void _UpdatePoint(size_t startIndex, size_t endIndex, size_t oldIndex, size_t replaceIdx);
  
  void _RemoveOneEdge(const HalfEdge* edge, bool* modified);
  void _ComputeVertexNeighbors(const HalfEdge* edge, pxr::VtArray<int>& neighbors);
  size_t _GetEdgeIndex(const HalfEdge* edge) const;
  size_t _GetFaceVerticesCount(const HalfEdge* edge);


private:
  // half-edge data
  pxr::VtArray<bool>                   _halfEdgeUsed;
  pxr::VtArray<HalfEdge>               _halfEdges;
  std::queue<int>                      _availableEdges;

  // vertex data
  pxr::VtArray<int>                    _vertexHalfEdge;
  pxr::VtArray<bool>                   _boundary;
  pxr::VtArray<int>                    _shell;
  pxr::VtArray<pxr::VtArray<int>>      _neighbors;

  friend Mesh;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif
