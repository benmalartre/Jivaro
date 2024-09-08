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
  static const int INVALID_INDEX = -1;
  int vertex;    // vertex index
  int twin;      // opposite half-edge
  int prev;      // previous half-edge  
  int next;      // next half-edge

  HalfEdge() : vertex(INVALID_INDEX), twin(INVALID_INDEX), prev(INVALID_INDEX), next(INVALID_INDEX){};
};

class HalfEdgeGraph {
public:
 
  struct ItUniqueEdge {
    const HalfEdgeGraph&      graph;
    int                       index;
    
    ItUniqueEdge(const HalfEdgeGraph& graph);
    HalfEdge* Next();
  };

  void ComputeGraph(Mesh* mesh);
  void ComputeNeighbors();
  void ComputeAdjacents();
  void ComputeNeighbors(const HalfEdge* edge, pxr::VtArray<int>& neighbors);
  void ComputeAdjacents(const HalfEdge* edge, pxr::VtArray<int>& adjacents);
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
  const HalfEdge* GetEdge(int index)const;
  size_t GetEdgeIndex(const HalfEdge* edge) const;
  HalfEdge* GetAvailableEdge();
  pxr::VtArray<HalfEdge>& GetEdges(){return _halfEdges;};
  const pxr::VtArray<HalfEdge>& GetEdges() const{return _halfEdges;};

  size_t GetNumNeighbors(size_t index);
  const int* GetNeighbors(size_t index);
  int GetNeighbor(size_t index, size_t neighbor);

  size_t GetNumAdjacents(size_t index);
  const int* GetAdjacents(size_t index);
  int GetAdjacent(size_t index, size_t adjacent);

  HalfEdge* GetEdgeFromVertex(size_t vertex);
  HalfEdge* GetEdgeFromVertices(size_t start, size_t end);
  const HalfEdge* GetEdgeFromVertices(size_t start, size_t end) const;
  void GetEdgesFromFace(const HalfEdge* edge, pxr::VtArray<int>& indices);
  size_t GetFaceFromEdge(const HalfEdge* edge);

  float GetLength(const HalfEdge* edge, const pxr::GfVec3f* positions) const;
  float GetLengthSq(const HalfEdge* edge, const pxr::GfVec3f* positions) const;

  const pxr::VtArray<bool>&  GetBoundaries() const {return _boundary;};

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
  void _ComputeVertexNeighbors(const HalfEdge* edge, pxr::VtArray<int>& neighbors, bool connected=false);
  size_t _GetEdgeIndex(const HalfEdge* edge) const;
  size_t _GetFaceVerticesCount(const HalfEdge* edge);


private:
  // half-edge data
  pxr::VtArray<bool>                   _halfEdgeUsed;
  pxr::VtArray<HalfEdge>               _halfEdges;
  std::queue<int>                      _availableEdges;

  // vertex data
  pxr::VtArray<int>                    _vertexHalfEdge;
  pxr::VtArray<int>                    _halfEdgeFace;
  pxr::VtArray<bool>                   _boundary;
  pxr::VtArray<int>                    _shell;
  pxr::VtArray<int>                    _adjacents; // connected
  pxr::VtArray<int>                    _adjacentsCount;
  pxr::VtArray<int>                    _adjacentsOffset;
  pxr::VtArray<int>                    _neighbors; // first ring
  pxr::VtArray<int>                    _neighborsCount;
  pxr::VtArray<int>                    _neighborsOffset;

  friend Mesh;

};

using HalfEdgesKeys = std::vector<std::pair<uint64_t, HalfEdge*>>;
using HalfEdgeKey  = HalfEdgesKeys::value_type;

JVR_NAMESPACE_CLOSE_SCOPE

#endif
