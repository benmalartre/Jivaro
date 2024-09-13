#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/quatf.h>
#include <pxr/base/gf/matrix3f.h>
#include <pxr/base/gf/matrix4f.h>
#include <iostream>

#include <Eigen/Sparse>

#include "../../src/utils/timer.h"
#include "../../src/geometry/matrix.h"
#include "../../src/geometry/mesh.h"
#include "../../src/geometry/scene.h"

PXR_NAMESPACE_USING_DIRECTIVE


JVR_NAMESPACE_OPEN_SCOPE

using Scalar = double;



void _PrintResult(std::string title, const Scalar* values, const Scalar* vectors) {
  std::cout << "######   " << title << "   ######" << std::endl;
  std::cout << "#" << std::endl;
  std::cout << "#   Values : " << values[0] << ", " << 
    values[1] << ", " << values[2] << std::endl;
  std::cout << "#" << std::endl;
  for(size_t i=0; i<3; ++i) {
    std::cout << "#   Vector : (" << vectors[i*3] << ", " << 
    vectors[i*3+1] << ", " << vectors[i*3+2] << ")" << std::endl;
  }
  std::cout << "#" << std::endl;
}


pxr::UsdPrim _TraverseStageFindingMesh(pxr::UsdStageRefPtr& stage)
{
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  for (pxr::UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<pxr::UsdGeomMesh>())
      return prim;
  
  return pxr::UsdPrim();
}

void _PrintBenchMark(const std::string& title, uint64_t t)
{
  static size_t titleMax = 32;
  size_t titleSpace = titleMax - title.length();
  std::cout << "# " << title;
  for(size_t i=0; i< titleSpace; ++i) std::cout << " ";
  std::cout << ": " << (t * 1e-6) << " seconds" << std::endl;
}


bool _CompareDatas(Scalar* lhs, Scalar* rhs, size_t N)
{
  for(size_t x = 0; x < N; ++x) {
    if(std::abs(lhs[x]) - std::abs(rhs[x]) > 0.001f)return false;
  }
  return true;
}

void _PrintTs()
{
  // custom
  {
    uint64_t totalT = SparseMatrix<Scalar>::feedT + SparseMatrix<Scalar>::computeT + SparseMatrix<Scalar>::solveT;
    _PrintBenchMark("sparse (Eigen): ", totalT);

    std::cout << " - feed    : " << (SparseMatrix<Scalar>::feedT * 1e-6) << " seconds" << std::endl;
    std::cout << " - compute : " << (SparseMatrix<Scalar>::computeT * 1e-6) << " seconds" << std::endl;
    std::cout << " - solve   : " << (SparseMatrix<Scalar>::solveT * 1e-6) << " seconds" << std::endl;
  }

  // eigen
  {

    uint64_t totalT = Matrix<Scalar>::feedT + Matrix<Scalar>::computeT + Matrix<Scalar>::solveT;
    _PrintBenchMark("dense (Custom): ", totalT );

    std::cout << " - feed    : " << (Matrix<Scalar>::feedT * 1e-6) << " seconds" << std::endl;
    std::cout << " - compute : " << (Matrix<Scalar>::computeT * 1e-6) << " seconds" << std::endl;
    std::cout << " - solve   : " << (Matrix<Scalar>::solveT * 1e-6) << " seconds" << std::endl;
  }

  Matrix<Scalar>::ResetTs();
  SparseMatrix<Scalar>::ResetTs();
}

bool _CompareMatrices(Matrix<Scalar>& dense, SparseMatrix<Scalar>& sparse)
{
  size_t rows = dense.NumRows();
  size_t columns = dense.NumColumns();

  std::vector<Scalar> D0(rows * columns);
  std::vector<Scalar> D1(rows * columns);

  sparse.GetDenseData(&D0[0], true);
  memcpy(&D1[0], (void*)dense.GetData(), rows * columns * sizeof(Scalar));

  return _CompareDatas(&D0[0], &D1[0], rows * columns);
}

void _BenchMark1(size_t N)
{
  std::cout << "\n\n############  Benchmark Inverse Matrix (" << N << " * " << N << ")" << std::endl;

  SparseMatrix<Scalar>   sparse(N, N);
  Matrix<Scalar>         dense(N, N);

  SparseMatrixInfos<Scalar> infos(0);
  std::vector<bool> skip(N, false);
  std::vector<Scalar> sum(N, 0.0);
  for(size_t n = 0; n < N; ++n) {
    for(size_t m = 0; m < 4; ++m) {
      size_t o = RANDOM_LO_HI(0, N-1);
      if(o != n && !skip[o]) {
        Scalar value = RANDOM_0_1;
        skip[n] = true;
        skip[o] = true;
        infos.AddEntry(n, o, value);
        infos.AddEntry(o, n, value);
        sum[n] += value;
        sum[o] += value;
      }
    }
  }
  for(size_t n = 0; n < N; ++n)
    infos.AddEntry(n, n, -sum[n]);

  sparse.Set(infos.keys.size(), &infos.keys[0], &infos.values[0]);
  dense.Set(infos.keys.size(), &infos.keys[0], &infos.values[0]);
  
  Matrix<Scalar> invDense = dense.Inverse();

  sparse.Compress();

  SparseMatrix<Scalar>::Solver solver;
  SparseMatrix<Scalar> invSparse = sparse.Inverse(solver);

  _PrintTs();

  std::cout<< "initial equals :" << _CompareMatrices(dense, sparse) << std::endl;;
  std::cout<< "inverse equals :" << _CompareMatrices(invDense, invSparse) << std::endl;
  std::cout<< "final equals   :" << _CompareMatrices(invDense.Multiply(invDense.Transpose()) , invSparse.Multiply(invSparse.Transpose())) << std::endl;
  std::cout << std::endl;

}


template <typename T>
SparseMatrixInfos<T> GetLaplacianMatrix(Mesh* mesh)
{
  size_t numVertices = mesh->GetNumPoints();
  size_t numEntries = mesh->GetTotalNumAdjacents() + numVertices;

  MeshCotangentWeights cotanWeights;
  mesh->ComputeCotangentWeights(cotanWeights);

  SparseMatrixInfos<T> infos(numEntries);
  size_t entryIdx = 0;
  for(size_t v = 0; v < numVertices; ++v) {
    size_t numAdjacents = mesh->GetNumAdjacents(v);
    for(size_t n = 0; n < numAdjacents; ++n) {
      size_t adjacent = mesh->GetAdjacent(v, n);
      if(v < adjacent) {
        T weight = cotanWeights.Get(v, n);
        infos.keys[entryIdx] = std::make_pair(v, adjacent);
        infos.values[entryIdx] = weight ;
        entryIdx++;

        infos.keys[entryIdx] = std::make_pair(adjacent, v);
        size_t o = mesh->GetAdjacentIndex(adjacent, v);
        infos.values[entryIdx] = cotanWeights.Get(adjacent, o);

        entryIdx++;
      }
    }
    infos.keys[entryIdx] = { v, v };
    infos.values[entryIdx] = cotanWeights.Get(v, numAdjacents);
    entryIdx++;
  }
  return infos;
}

template <typename T>
SparseMatrixInfos<T> GetMassMatrix(Mesh* mesh)
{
  size_t numEntries = mesh->GetNumPoints();
  SparseMatrixInfos<T> infos(numEntries);

  MeshAreas areas;
  mesh->ComputeAreas(areas);

  for(size_t v = 0; v < numEntries; ++v) {
    infos.keys[v] = {v, v};
    infos.values[v] = areas.vertex[v];
  }

  return infos;

}

// benchmark matrix class efficency (compared to pxr::GfMatrix4)
void _BenchMark2( const std::string &filename) {
  std::cout << "\n\n############  Benchmark Inverse Matrix (" << filename << ")" << std::endl;
  Matrix<Scalar>::ResetTs();
  SparseMatrix<Scalar>::ResetTs();
  SparseMatrix<Scalar>   sparse;
  Matrix<Scalar>         dense;

  pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(filename); 
  Scene scene;
  scene.Init(stage);

  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());

  pxr::UsdPrim prim = _TraverseStageFindingMesh(stage);
  if(!prim.IsValid()) {
    std::cerr << "filename invalid: no mesh found!" << std::endl;
    return;
  }
  Mesh *mesh = new Mesh(pxr::UsdGeomMesh(prim), xformCache.GetLocalToWorldTransform(prim));
  //mesh->Triangulate();
  scene.AddGeometry(prim.GetPath(), mesh);

  size_t N = mesh->GetNumPoints();
  sparse.Resize(N, N);
  dense.Resize(N, N);

  uint64_t startT = CurrentTime();
  
  SparseMatrixInfos<Scalar> infos = GetLaplacianMatrix<Scalar>(mesh);
  std::cout << "compute infos took : " << ((CurrentTime() - startT) * 1e-6) << std::endl;

  sparse.Set(infos.keys.size(), &infos.keys[0], &infos.values[0]);
  dense.Set(infos.keys.size(), &infos.keys[0], &infos.values[0]);

  //std::cout << "dense : " << dense << std::endl;
  //std::cout << "sparse : " << sparse << std::endl;

  Matrix<Scalar> invDense = dense.Inverse();

  sparse.Compress();

  SparseMatrix<Scalar>::Solver solver;
  SparseMatrix<Scalar> invSparse = sparse.Inverse(solver);

  //std::cout << "inverse dense : " << invDense << std::endl;
  //std::cout << "inverse sparse : " << invSparse << std::endl;

 _PrintTs();

  std::cout<< "initial equals :" << _CompareMatrices(dense, sparse) << std::endl;;
  std::cout<< "inverse equals :" << _CompareMatrices(invDense, invSparse) << std::endl;

  startT = CurrentTime();
  SparseMatrix<Scalar> finalSparse = invSparse.Multiply(invSparse.Transpose());
  std::cout<< "multiply sparse took :" << ((CurrentTime() - startT) * 1e-6) << std::endl;

  startT = CurrentTime();
  Matrix<Scalar> finalDense = invDense.Multiply(invDense.Transpose());
  std::cout<< "multiply dense took :" << ((CurrentTime() - startT) * 1e-6) << std::endl;


  std::cout<< "final equals   :" << _CompareMatrices(finalDense , finalSparse)<< std::endl;
  std::cout << std::endl;
  
  
}


JVR_NAMESPACE_CLOSE_SCOPE

JVR_NAMESPACE_USING_DIRECTIVE

int main (int argc, char *argv[])
{

  for (size_t N = 2; N < 5; ++N)
    _BenchMark1(1<<N);


  std::cout << "bench 1 ok" << std::endl;

  std::string filename0 = "C:/Users/graph/Documents/bmal/src/USD_ASSETS/UVMapping/Cubo.usda";
  _BenchMark2(filename0);


  std::string filename1 = "C:/Users/graph/Documents/bmal/src/USD_ASSETS/UVMapping/Lezar.usda";
  _BenchMark2(filename1);

  std::cout << "bench 2 ok" << std::endl; 

  //std::string filename2 = "C:/Users/graph/Documents/bmal/src/USD_ASSETS/UVMapping/cow.usda";
  //_BenchMark(32, filename2);

  //std::cout << "bench 3 ok" << std::endl; 
  return 0;
}