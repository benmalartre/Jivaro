#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/quatf.h>
#include <pxr/base/gf/matrix3f.h>
#include <pxr/base/gf/matrix4f.h>
#include <iostream>

#include <Eigen/Dense>
using Scalar = double;

using Vector2r = Eigen::Matrix<Scalar, 2, 1, Eigen::DontAlign>;
using Vector3r = Eigen::Matrix<Scalar, 3, 1, Eigen::DontAlign>;
using Vector4r = Eigen::Matrix<Scalar, 4, 1, Eigen::DontAlign>;
using Vector5r = Eigen::Matrix<Scalar, 5, 1, Eigen::DontAlign>;
using Vector6r = Eigen::Matrix<Scalar, 6, 1, Eigen::DontAlign>;
using Matrix2r = Eigen::Matrix<Scalar, 2, 2, Eigen::DontAlign>;
using Matrix3r = Eigen::Matrix<Scalar, 3, 3, Eigen::DontAlign>;
using Matrix4r = Eigen::Matrix<Scalar, 4, 4, Eigen::DontAlign>;

#include "../../src/utils/timer.h"
#include "../../src/geometry/matrix.h"
#include "../../src/geometry/mesh.h"
#include "../../src/geometry/scene.h"

PXR_NAMESPACE_USING_DIRECTIVE


JVR_NAMESPACE_OPEN_SCOPE



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
    if(std::abs(lhs[x] - rhs[x]) > 0.001f)return false;
  }
  return true;
}

// benchmark matrix class efficency (compared to pxr::GfMatrix4)
void _BenchMark(size_t N, const std::string &filename) {
  std::cout << "0 " << std::endl;
  SparseMatrix<Scalar>   inverseSparse(N, N);
  Matrix<Scalar>         inverseDense(N, N);
  std::cout << "1" << std::endl;
  pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(filename); 
  Scene scene;
  scene.Init(stage);

  std::cout << "2 " << std::endl;

  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());

  pxr::UsdPrim prim = _TraverseStageFindingMesh(stage);
  if(prim.IsValid()) {
    Mesh *mesh = new Mesh(pxr::UsdGeomMesh(prim), xformCache.GetLocalToWorldTransform(prim));
    scene.AddGeometry(prim.GetPath(), mesh);

    std::cout << "3 " << std::endl;

    N = mesh->GetNumPoints();
    inverseSparse.Resize(N, N);
    inverseDense.Resize(N, N);

    uint64_t startT = CurrentTime();
    HalfEdgeGraph* graph = mesh->GetEdgesGraph();
    const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
    mesh->ComputeCotangentWeights();
    HalfEdgeGraphSparseMatrix<Scalar> laplaceMatrixInfos = HalfEdgeGraphGetLaplacianMatrix<Scalar>(*graph);

    std::cout << "4 " << std::endl;

    inverseSparse.Set(laplaceMatrixInfos.values.size(), &laplaceMatrixInfos.rows[0],
      &laplaceMatrixInfos.columns[0], &laplaceMatrixInfos.values[0]);


    inverseSparse.Compress();

  } else {
    for(size_t c = 0; c < N; ++c)
      for(size_t r = c; r < N; ++r)
        {
          Scalar value = RANDOM_0_1;
          inverseSparse.Set(r, c, value);
          inverseSparse.Set(c, r, value);
          inverseDense.Set(r, c, value);
          inverseDense.Set(c, r, value);
        }

      inverseSparse.Compress();
  }

  /*
  std::cout << "sparse (Eigen): " << inverseSparse << std::endl;
  std::cout << "dense (Custom): " << inverseDense << std::endl;
  */
  std:: cout << "\n\n############  Benchmark Inverse Matrix (" << N << "*" << N << ") ##########################" << std::endl;
  uint64_t startT; 

  // custom
  {
    startT = CurrentTime();
    SparseMatrix<Scalar>::Solver solver;
    inverseSparse.InverseInPlace(solver);

    _PrintBenchMark("sparse (Eigen): ", CurrentTime() - startT);

    std::cout << "compute : " << (SparseMatrix<Scalar>::computeT * 1e-6) << " seconds" << std::endl;
    std::cout << "identity : " << (SparseMatrix<Scalar>::identityT * 1e-6) << " seconds" << std::endl;
    std::cout << "solve : " << (SparseMatrix<Scalar>::solveT * 1e-6) << " seconds" << std::endl;
  }

  // eigen
  {

    startT = CurrentTime();
   
    inverseDense.InverseInPlace();
    
    _PrintBenchMark("dense (Custom): ", CurrentTime() - startT);

    std::cout << "compute : " << (Matrix<Scalar>::computeT * 1e-6) << " seconds" << std::endl;
    std::cout << "identity : " << (Matrix<Scalar>::identityT * 1e-6) << " seconds" << std::endl;
    std::cout << "solve : " << (Matrix<Scalar>::solveT * 1e-6) << " seconds" << std::endl;
    
  }

  std::vector<Scalar> D0(N * N);
  std::vector<Scalar> D1(N * N);

  inverseSparse.GetDenseData(&D0[0], true);
  memcpy(&D1[0], (void*)inverseDense.GetData(), N * N * sizeof(Scalar));
  /*
  std::cout << "sparse : " << inverseSparse << std::endl;
  std::cout << "dense : " << inverseDense << std::endl;
  */
  std::cout << "Compare custom to eigen : " << _CompareDatas(&D0[0], &D1[0], N * N) << std::endl;
  
}


JVR_NAMESPACE_CLOSE_SCOPE

JVR_NAMESPACE_USING_DIRECTIVE

int main (int argc, char *argv[])
{

  std::string filename0 = "C:/Users/graph/Documents/bmal/src/USD_ASSETS/UVMapping/Kitty.usda";
  _BenchMark(32, filename0);

  std::cout << "bench 1 ok" << std::endl; 
  

  std::string filename1 = "C:/Users/graph/Documents/bmal/src/USD_ASSETS/UVMapping/Lezar.usda";
  _BenchMark(32, filename1);

  std::cout << "bench 2 ok" << std::endl; 
  return 0;
}