
#include "../acceleration/bvh.h"
#include "../geometry/geodesic.h"
#include "../geometry/smooth.h"
#include "../geometry/mesh.h"


JVR_NAMESPACE_OPEN_SCOPE

// https://github.com/IasonManolas/Geodesics_in_heat/blob/master/src/geodesicdistance.hpp


Geodesic::Geodesic(Mesh* mesh) : _mesh(mesh) 
{
  // compute timestep
  _averageEdgeLength = mesh->ComputeAverageEdgeLength();


  double sum = 0.0;
  int n = 0;
  HalfEdgeGraph::ItUniqueEdge it(*mesh->GetEdgesGraph());
  HalfEdge* edge = it.Next();
  const GfVec3f* positions = mesh->GetPositionsCPtr();
  while(edge) {
    double d = (positions[edge->vertex] - positions[mesh->GetEdge(edge->next)->vertex]).GetLength();
    sum += d;
    n++;
   
  }
  _t = (sum /n ) * (sum /n);

  _Init();
  _Prefactor();
}

void 
Geodesic::SetMFactor(double m) 
{
  _t = _averageEdgeLength * _averageEdgeLength * m;
  _Prefactor();
}

void 
Geodesic::_Init() 
{

  std::cout << "Initializing.." << std::endl;
  // compute cotangent operator L_C
  std::cout << "Computing cotangent matrix.." << std::endl;

  
  _mesh->ComputeCotangentWeights(_cotangentWeights);

  SparseMatrixInfos<double> laplaceMatrixInfos = GetLaplacianMatrix<double>(_mesh, _cotangentWeights);

  _Lc.Set(laplaceMatrixInfos.keys.size(), &laplaceMatrixInfos.keys[0], &laplaceMatrixInfos.values[0]);

  // compute mass matrix A
  
  std::cout << "Computing mass matrix.." << std::endl;
  
  _mesh->ComputeAreas(_areas);
  SparseMatrixInfos<double> massMatrixInfos = GetMassMatrix<double>(_mesh, _areas);

  _A.Set(massMatrixInfos.keys.size(), &massMatrixInfos.keys[0], &massMatrixInfos.values[0]);

  _initialized = true;
}
	
void Geodesic::_Prefactor() 
{
  // factor first equation
  std::cout << "Prefactoring the heat equation.." << std::endl;
  SparseMatrix<double> B, B0;
  B0 = _Lc.Scale(_t);
  B = _A.Add(B0);

  B.Compute(_la);
  B.CheckSuccess(_la, "Prefactoring the heat equation failed.");

  // factor the poisson problem
  std::cout << "Prefactoring the poisson's equation.." << std::endl;
  _Lc.Compute(_laCotan);
  _Lc.CheckSuccess(_laCotan, "Prefactoring the poisson equation failed." );

}

void Geodesic::_UpdateKroneckerDelta(
    const Geodesic::VertexIndexSet &sourcesVi)
{
  Eigen::SparseVector<double> delta(_mesh->GetNumPoints());
  delta.reserve(sourcesVi.size());
  for (int vi : sourcesVi) {
    delta.insert(vi) = 1;
  }

  _kronecker = delta;
}

void Geodesic::_SolveCotanLaplace()
{
  _solvedU = _A.Solve(_la, _kronecker);
}

void Geodesic::_ComputeGradient() 
{
  size_t numFaces = _mesh->GetNumFaces();
  if (_gradient.empty()) {
    _gradient.resize(numFaces);
  }
  /*
  for (size_t fi = 0; fi < numFaces; ++fi) {
    VCGTriMesh::FaceType &f = m.face[fi];
    const double faceArea = vcg::DoubleArea(m.face[fi]) / 2;
    const VCGTriMesh::VertexType &v0 = *f.cV(0);
    const VCGTriMesh::VertexType &v1 = *f.cV(1);
    const VCGTriMesh::VertexType &v2 = *f.cV(2);
    double u0 = m_solved_u(m.getIndex(v0));
    double u1 = m_solved_u(m.getIndex(v1));
    double u2 = m_solved_u(m.getIndex(v2));

    double r_mag = 1. / std::max(std::max(u0, u1), u2);
    if (std::isfinite(r_mag)) {
      u0 *= r_mag;
      u1 *= r_mag;
      u2 *= r_mag;
    }
    auto vij = v1.cP() - v0.cP();
    auto pj = v1.cP();
    auto pk = v2.cP();
    VCGTriMesh::CoordType faceNormal = f.cN();
    VCGTriMesh::CoordType sum = (faceNormal ^ (v1.cP() - v0.cP())) * u2;
    sum += (faceNormal ^ (v2.cP() - v1.cP())) * u0;
    sum += (faceNormal ^ (v0.cP() - v2.cP())) * u1;
    sum /= faceArea;
    m_unitGradient[fi] = sum.Normalize();
  }
  */
}




JVR_NAMESPACE_CLOSE_SCOPE
