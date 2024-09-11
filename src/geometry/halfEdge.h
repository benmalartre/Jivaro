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
  int twin;      // opposite half-edge
  int prev;      // previous half-edge  
  int next;      // next half-edge

  HalfEdge() : vertex(INVALID_INDEX), twin(INVALID_INDEX), prev(INVALID_INDEX), next(INVALID_INDEX){};
};

static std::ostream& operator<<(std::ostream& os, const HalfEdge *edge) {

  return os << edge->vertex << " " << edge->prev << " " << 
    edge->next << " " << edge->twin << std::endl;
}

template <typename T>
struct HalfEdgeGraphSparseMatrix 
{
  pxr::VtArray<int> rows;
  pxr::VtArray<int> columns;
  pxr::VtArray<T>   values;

  HalfEdgeGraphSparseMatrix(size_t size) {
    rows.resize(size);
    columns.resize(size);
    values.resize(size);
  };
};

class HalfEdgeGraph {
public:
 
  static inline const float EPSILON = 1e-6f;

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
  void ComputeCotangentWeights(const pxr::GfVec3f *positions);

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

  size_t GetNumVertices() const;
  size_t GetNumRawEdges() const;
  size_t GetNumEdges() const;
  HalfEdge* GetEdge(int index);
  const HalfEdge* GetEdge(int index)const;
  size_t GetEdgeIndex(const HalfEdge* edge) const;
  HalfEdge* GetAvailableEdge();
  pxr::VtArray<HalfEdge>& GetEdges(){return _halfEdges;};
  const pxr::VtArray<HalfEdge>& GetEdges() const{return _halfEdges;};

  size_t GetNumNeighbors(size_t index) const;
  const int* GetNeighbors(size_t index)const;
  int GetNeighbor(size_t index, size_t neighbor)const;
  size_t GetTotalNumNeighbors()const;

  size_t GetNumAdjacents(size_t index)const;
  const int* GetAdjacents(size_t index)const;
  int GetAdjacent(size_t index, size_t adjacent)const;

  const float* GetCotangentWeights(size_t index) const;
  float GetCotangentWeight(size_t index, size_t neighbor) const;

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
  inline float _CotangentWeight(float x);

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
  pxr::VtArray<float>                  _cotangentWeights;

  friend Mesh;

};

inline float 
HalfEdgeGraph::_CotangentWeight(float x)
{
  const float cotanMax = pxr::GfCos( HalfEdgeGraph::EPSILON ) / pxr::GfSin( HalfEdgeGraph::EPSILON  );
  //return pxr::GfCos(x)/pxr::GfSin(x);
  float cotan = pxr::GfCos(x)/pxr::GfSin(x);
  return cotan < -cotanMax ? -cotanMax : cotan > cotanMax ? cotanMax : cotan;
}

using HalfEdgesKeys = std::vector<std::pair<uint64_t, HalfEdge*>>;
using HalfEdgeKey  = HalfEdgesKeys::value_type;

template <typename T>
HalfEdgeGraphSparseMatrix<T> HalfEdgeGraphGetLaplacianMatrix(const HalfEdgeGraph& graph)
{
  size_t numVertices = graph.GetNumVertices();
  size_t numEntries = graph.GetTotalNumNeighbors() * 2 + numVertices;

  std::cout << "num laplace entries : " << numEntries << std::endl;
  HalfEdgeGraphSparseMatrix<T> laplaceMatrixInfos(numEntries);
  size_t entryIdx = 0;
  for(size_t v = 0; v < graph.GetNumVertices(); ++v) {
    size_t numNeighbors = graph.GetNumNeighbors(v);
    for(size_t n = 0; n < numNeighbors; ++n) {
      size_t neighbor = graph.GetNeighbor(v, n);

      laplaceMatrixInfos.rows[entryIdx] = v;
      laplaceMatrixInfos.columns[entryIdx] = neighbor;
      laplaceMatrixInfos.values[entryIdx] = graph.GetCotangentWeight(v, n);
      entryIdx++;

      laplaceMatrixInfos.rows[entryIdx] = neighbor;
      laplaceMatrixInfos.columns[entryIdx] = v;
      laplaceMatrixInfos.values[entryIdx] = graph.GetCotangentWeight(n, v);
      entryIdx++;
    }
    laplaceMatrixInfos.rows[entryIdx] = v;
    laplaceMatrixInfos.columns[entryIdx] = v;
    laplaceMatrixInfos.values[entryIdx] = -1.0;//graph.GetCotangentWeight(n, v);
    entryIdx++;
    //std::cout << "entry idx " << entryIdx << std::endl;
  }

  std::cout << "last entry : " << entryIdx << std::endl;

  //std::cout << laplaceMatrixInfos.rows << std::endl;
  //std::cout << laplaceMatrixInfos.columns << std::endl;
  return laplaceMatrixInfos;
}

template <typename T>
HalfEdgeGraphSparseMatrix<T> HalfEdgeGraphGetMassMatrix(const HalfEdgeGraph& graph)
{
  pxr::VtArray<float> areas(graph.GetNumPoints());
  /*
  typename MeshType::template PerVertexAttributeHandle<ScalarType> h =
          tri::Allocator<MeshType>:: template GetPerVertexAttribute<ScalarType>(m, "area");
  for(int i=0;i<m.vn;++i) h[i]=0;

  for(FaceIterator fi=m.face.begin(); fi!=m.face.end();++fi)
  {
      ScalarType a = DoubleArea(*fi);
      for(int j=0;j<fi->VN();++j)
          h[tri::Index(m,fi->V(j))] += a;
  }
  ScalarType maxA=0;
  for(int i=0;i<m.vn;++i)
      maxA = std::max(maxA,h[i]);

  //store the index and the scalar for the sparse matrix
  for (size_t i=0;i<m.vert.size();i++)
  {
      if (vertexCoord)
      {
          for (size_t j=0;j<3;j++)
          {
              int currI=(i*3)+j;
              index.push_back(std::pair<int,int>(currI,currI));
              entry.push_back(h[i]/maxA);
          }
      }
      else
      {
          int currI=i;
          index.push_back(std::pair<int,int>(currI,currI));
          entry.push_back(h[i]/maxA);
      }
  }
  tri::Allocator<MeshType>::template DeletePerVertexAttribute<ScalarType>(m,h);
  */
  return HalfEdgeGraphSparseMatrix<T>;
}


JVR_NAMESPACE_CLOSE_SCOPE

#endif
