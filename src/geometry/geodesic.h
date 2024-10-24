#ifndef JVR_GEOMETRY_GEODESIC_H
#define JVR_GEOMETRY_GEODESIC_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include <unordered_set>
#include "../geometry/matrix.h"
#include "../geometry/mesh.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
class Mesh;
class HalfEdgeGraph;

class Geodesic
{
  using VertexIndexSet = std::unordered_set<int>;
  using VertexIndexMap = std::unordered_map<int, double>;
  using SimplicialSolver = Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>>;
public:
  Geodesic(Mesh* mesh);

  void SetHalfEdgeGraph(Mesh* mesh);
  void SetMFactor(double m);

  Eigen::VectorXd Compute(const VertexIndexSet &vertices, VertexIndexMap &distances);

protected:
  void _Init();
  void _Prefactor();
  void _UpdateKroneckerDelta(const VertexIndexSet &sourcesVi);
  void _SolveCotanLaplace();
  void _ComputeGradient();
  void _ComputeDivergence();

  Eigen::VectorXd _SolvePhi(const VertexIndexSet &sourcesVi);

private:

  bool                          _initialized{false};
  Mesh*                         _mesh;
  double                        _t;
  SparseMatrix<double>::Vector  _kronecker;
  Eigen::VectorXd               _solvedU;
  VtArray<GfVec3d>    _gradient;
  Eigen::VectorXd               _divergence;

  SimplicialSolver              _la;
  SimplicialSolver              _laCotan;

  SparseMatrix<double>          _Lc;
  SparseMatrix<double>          _A;

  double                        _averageEdgeLength;
  MeshCotangentWeights          _cotangentWeights;
  MeshAreas                     _areas;
}; 


template <typename T>
SparseMatrixInfos<T> GetLaplacianMatrix(const Mesh* mesh, const MeshCotangentWeights& weights)
{
  size_t numVertices = mesh->GetNumPoints();
  size_t numEntries = mesh->GetTotalNumAdjacents() + numVertices;

  SparseMatrixInfos<T> infos(numEntries);
  size_t entryIdx = 0;
  for(size_t v = 0; v < numVertices; ++v) {
    size_t numAdjacents = mesh->GetNumAdjacents(v);
    for(size_t n = 0; n < numAdjacents; ++n) {
      size_t adjacent = mesh->GetAdjacent(v, n);
      if(v < adjacent) {
        T weight = weights.Get(v, n);
        infos.keys[entryIdx] = std::make_pair(v, adjacent);
        infos.values[entryIdx] = weight ;
        entryIdx++;

        infos.keys[entryIdx] = std::make_pair(adjacent, v);
        size_t o = mesh->GetAdjacentIndex(adjacent, v);
        infos.values[entryIdx] = weights.Get(adjacent, o);

        entryIdx++;
      }
    }
    infos.keys[entryIdx] = { v, v };
    infos.values[entryIdx] = weights.Get(v, numAdjacents);
    entryIdx++;
  }
  return infos;
}

template <typename T>
SparseMatrixInfos<T> GetMassMatrix(const Mesh* mesh, const MeshAreas& areas)
{
  size_t numEntries = mesh->GetNumPoints();
  SparseMatrixInfos<T> infos(numEntries);

  for(size_t v = 0; v < numEntries; ++v) {
    infos.keys[v] = {v, v};
    infos.values[v] = areas.vertex[v];
  }

  return infos;

}

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_GEODESIC_H
