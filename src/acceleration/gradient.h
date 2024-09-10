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

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_GRADIENT_H
