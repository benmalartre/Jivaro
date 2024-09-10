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
    if(std::abs(lhs[x] - rhs[x]) > 0.0001f)return false;
  }
  return true;
}

// benchmark matrix class efficency (compared to pxr::GfMatrix4)
void _BenchMark(size_t N) {
  

  SparseMatrix<Scalar>   inverseSparse(N, N);
  Matrix<Scalar>         inverseDense(N, N);
  for(size_t c = 0; c < N; ++c)
    for(size_t r = c; r < N; ++r)
      {
        Scalar value = RANDOM_0_1;
        inverseSparse.Set(r, c, value);
        inverseSparse.Set(c, r, value);
        inverseDense.Set(r, c, value);
        inverseDense.Set(c, r, value);
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

    std::cout << "compute : " << (SparseMatrix<Scalar>::solveT * 1e-6) << " seconds" << std::endl;
    std::cout << "identity : " << (SparseMatrix<Scalar>::identityT * 1e-6) << " seconds" << std::endl;
    std::cout << "solve : " << (SparseMatrix<Scalar>::solveT * 1e-6) << " seconds" << std::endl;
  }

  // eigen
  {

    startT = CurrentTime();
   
    inverseDense.InverseInPlace();
    
    _PrintBenchMark("dense (Custom): ", CurrentTime() - startT);
    
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
  for(size_t i = 2; i < 16; ++i)
    _BenchMark(1<<i);
  return 0;
}