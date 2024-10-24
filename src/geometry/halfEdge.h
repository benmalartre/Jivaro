#ifndef JVR_GEOMETRY_HALFEDGE_H
#define JVR_GEOMETRY_HALFEDGE_H

#include <queue>
#include <iterator>
#include <vector>
#include <iomanip>
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
  int face;      // face index
  int twin;      // opposite half-edge
  int prev;      // previous half-edge  
  int next;      // next half-edge

  HalfEdge() : vertex(INVALID_INDEX), twin(INVALID_INDEX), prev(INVALID_INDEX), next(INVALID_INDEX){};
};

static std::ostream& operator<<(std::ostream& os, const HalfEdge *edge) {

  return os << edge->vertex << " " << edge->prev << " " << 
    edge->next << " " << edge->twin << std::endl;
}

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
  void ComputeNeighbors(const HalfEdge* edge, VtArray<int>& neighbors);
  void ComputeAdjacents(const HalfEdge* edge, VtArray<int>& adjacents);
  void ComputeTopology(VtArray<int>& faceCounts, VtArray<int>& faceConnects) const;

  void AllocateEdges(size_t num);
  bool FlipEdge(HalfEdge* edge);
  bool SplitEdge(HalfEdge* edge, size_t numPoints);
  bool CollapseEdge(HalfEdge* edge);
  bool CollapseStar(HalfEdge* edge, VtArray<int>& neighbors);
  void RemoveEdge(HalfEdge* edge, bool* removed);
  void RemovePoint(size_t index, size_t replace);
  
  bool IsCollapsable(const HalfEdge* edge);
  bool IsUnique(const HalfEdge* edge) const;
  bool IsUsed(const HalfEdge* edge) const;

  size_t GetNumVertices() const;
  size_t GetNumRawEdges() const;
  size_t GetNumEdges() const;
  HalfEdge* GetEdge(int index);
  const HalfEdge* GetEdge(int index)const;
  size_t GetEdgeIndex(const HalfEdge* edge) const;
  HalfEdge* GetAvailableEdge();
  VtArray<HalfEdge>& GetEdges(){return _halfEdges;};
  const VtArray<HalfEdge>& GetEdges() const{return _halfEdges;};

  HalfEdge* GetPreviousAdjacentEdge(const HalfEdge* edge);
  const HalfEdge* GetPreviousAdjacentEdge(const HalfEdge* edge) const;
  HalfEdge* GetNextAdjacentEdge(const HalfEdge* edge);
  const HalfEdge* GetNextAdjacentEdge(const HalfEdge* edge) const;
  HalfEdge* GetNextEdge(const HalfEdge* edge);
  const HalfEdge* GetNextEdge(const HalfEdge* edge) const;
  HalfEdge* GetPreviousEdge(const HalfEdge* edge);
  const HalfEdge* GetPreviousEdge(const HalfEdge* edge) const;

  size_t GetNumNeighbors(size_t index) const;
  const int* GetNeighbors(size_t index)const;
  int GetNeighbor(size_t index, size_t neighbor)const;
  int GetNeighborIndex(size_t neighbor, size_t index)const;
  size_t GetTotalNumNeighbors()const;

  size_t GetNumAdjacents(size_t index)const;
  const int* GetAdjacents(size_t index)const;
  int GetAdjacent(size_t index, size_t adjacent)const;
  int GetAdjacentIndex(size_t neighbor, size_t index)const;
  size_t GetTotalNumAdjacents()const;

  HalfEdge* GetEdgeFromVertex(size_t vertex);
  HalfEdge* GetEdgeFromVertices(size_t start, size_t end);
  const HalfEdge* GetEdgeFromVertices(size_t start, size_t end) const;

  float GetLength(const HalfEdge* edge, const GfVec3f* positions) const;
  float GetLengthSq(const HalfEdge* edge, const GfVec3f* positions) const;

  const VtArray<bool>&  GetBoundaries() const {return _boundary;};

  //HalfEdge* GetLongest(const GfVec3f* positions);
  /*
  short GetFlags(const GfVec3f* positions, const GfVec3f* normals,
    const GfVec3f& v, float creaseValue) const;
  float GetWeight(const GfVec3f* positions, const GfVec3f* normals,
    const GfVec3f& v) const;
  */

protected:

  bool _IsTriangle(const HalfEdge* edge) const;
  void _TriangulateFace(const HalfEdge* edge);
  void _UpdatePoint(size_t startIndex, size_t endIndex, size_t oldIndex, size_t replaceIdx);
  
  void _RemoveOneEdge(const HalfEdge* edge, bool* modified);
  void _ComputeVertexNeighbors(const HalfEdge* edge, VtArray<int>& neighbors, bool connected=false);
  size_t _GetEdgeIndex(const HalfEdge* edge) const;
  size_t _GetFaceVerticesCount(const HalfEdge* edge);


private:
  // half-edge data
  VtArray<bool>                   _halfEdgeUsed;
  VtArray<HalfEdge>               _halfEdges;
  std::queue<int>                      _availableEdges;

  // vertex data
  VtArray<int>                    _vertexHalfEdge;
  VtArray<bool>                   _boundary;
  VtArray<int>                    _shell;
  VtArray<int>                    _adjacents; // connected
  VtArray<int>                    _adjacentsCount;
  VtArray<int>                    _adjacentsOffset;
  VtArray<int>                    _neighbors; // first ring
  VtArray<int>                    _neighborsCount;
  VtArray<int>                    _neighborsOffset;

  friend Mesh;

};

using HalfEdgesKeys = std::vector<std::pair<uint64_t, HalfEdge*>>;
using HalfEdgeKey  = HalfEdgesKeys::value_type;


JVR_NAMESPACE_CLOSE_SCOPE

#endif
