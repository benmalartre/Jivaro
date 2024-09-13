#ifndef JVR_ACCELERATION_GRADIENT_H
#define JVR_ACCELERATION_GRADIENT_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include <unordered_set>

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
class Mesh;
class HalfEdgeGraph;

class Gradient
{
  using VertexIndexSet = std::unordered_set<int>;
  using VertexIndexMap = std::unordered_map<int, double>;
  using SimplicialSolver = Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>>;
public:
  void Prefactor();
  void Compute(const VertexIndexSet &vertices, VertexIndexMap &distances);

protected:
  void _UpdateKroneckerDelta(const VertexIndexSet &sourcesVi);
  void _SolveCotanLaplace();
  void _ComputeUnitGradient();
  void _ComputeDivergence();

  Eigen::VectorXd _SolvePhi(const VertexIndexSet &sourcesVi);

private:
  HalfEdgeGraph*              _graph;
  double                      _t;
  Eigen::SparseVector<double> _kronecker;
  Eigen::VectorXd             _solved_u;
  pxr::VtArray<pxr::GfVec3d>  _gradient;
  Eigen::VectorXd             _divergence;

  SimplicialSolver            _la;
  SimplicialSolver            _la_cotan;


  Eigen::SparseMatrix<double> _Lc;
  Eigen::SparseMatrix<double> _A;

  double                      _averageEdgeLength;
}; 


template <typename T>
SparseMatrixInfos<T> GetLaplacianMatrix()
{
  size_t numVertices = _graph.GetNumVertices();
  size_t numEntries = _graph.GetTotalNumAdjacents() + numVertices;

  SparseMatrixInfos<T> infos(numEntries);
  size_t entryIdx = 0;
  for(size_t v = 0; v < _graph.GetNumVertices(); ++v) {
    size_t numAdjacents = _graph.GetNumAdjacents(v);
    for(size_t n = 0; n < numAdjacents; ++n) {
      size_t adjacent = _graph.GetAdjacent(v, n);
      if(v < adjacent) {
        T weight = graph.GetCotangentWeight(v, n);
        infos.keys[entryIdx] = std::make_pair(v, adjacent);
        infos.values[entryIdx] = weight ;
        entryIdx++;

        infos.keys[entryIdx] = std::make_pair(adjacent, v);
        size_t o = graph.GetAdjacentIndex(adjacent, v);
        infos.values[entryIdx] = graph.GetCotangentWeight(adjacent, o);

        entryIdx++;
      }
    }
    infos.keys[entryIdx] = { v, v };
    infos.values[entryIdx] = graph.GetCotangentWeight(v, numAdjacents);
    entryIdx++;
  }
  return infos;
}

template <typename T>
SparseMatrixInfos<T> GetMassMatrix()
{
  size_t numEntries = graph.GetNumVertices();
  SparseMatrixInfos<T> infos(numEntries);

  for(size_t v = 0; v < numEntries; ++v) {
    infos.keys[v] = {v, v};
    infos.values[v] = graph.GetMass(v);
  }

  return infos;

}

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_GRADIENT_H
