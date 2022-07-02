#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/quatf.h>
#include <pxr/base/gf/matrix3f.h>
#include <pxr/base/gf/matrix4f.h>
#include <iostream>

#include <Eigen/Dense>

using Vector2r = Eigen::Matrix<float, 2, 1, Eigen::DontAlign>;
using Vector3r = Eigen::Matrix<float, 3, 1, Eigen::DontAlign>;
using Vector4r = Eigen::Matrix<float, 4, 1, Eigen::DontAlign>;
using Vector5r = Eigen::Matrix<float, 5, 1, Eigen::DontAlign>;
using Vector6r = Eigen::Matrix<float, 6, 1, Eigen::DontAlign>;
using Matrix2r = Eigen::Matrix<float, 2, 2, Eigen::DontAlign>;
using Matrix3r = Eigen::Matrix<float, 3, 3, Eigen::DontAlign>;
using Matrix4r = Eigen::Matrix<float, 4, 4, Eigen::DontAlign>;

#include "../../src/pbd/eigenSolver.h"
#include "../../src/utils/timer.h"

#include "../../src/pbd/matrix.h"

PXR_NAMESPACE_USING_DIRECTIVE



pxr::GfQuatf _RandomQuaternion() {
  float x,y,z, u,v,w, s;
  do { 
    x = RANDOM_LO_HI(-1.0,1.0); 
    y = RANDOM_LO_HI(-1.0,1.0); 
    z = x*x + y*y; 
  } while (z > 1);

  do { 
    u = RANDOM_LO_HI(-1.0,1.0); 
    v = RANDOM_LO_HI(-1.0,1.0); 
    w = u*u + v*v; 
  } while (w > 1);

  s = sqrt((1-z) / w);
  return pxr::GfQuatf(x, y, s*u, s*v).GetNormalized();
}

pxr::GfVec3f _RandomScale()
{
  return pxr::GfVec3f(
    RANDOM_LO_HI(0.5,1.5), RANDOM_LO_HI(0.5,1.5), RANDOM_LO_HI(0.5,1.5)
  );
}

void _PrintResult(std::string title, const float* values, const float* vectors) {
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
  std::cout << ": " << (t * 1e-9) << " seconds" << std::endl;
}

void _RandomMatrices(std::vector<pxr::GfMatrix4f>& matrices)
{
  for(auto& matrix: matrices) {
    matrix = pxr::GfMatrix4f(1.0).SetRotate(_RandomQuaternion()) * 
     pxr::GfMatrix4f(1.f).SetScale(_RandomScale());
  }
}

/*
void _Jacobi3(const pxr::GfMatrix4f& m, pxr::GfVec3f* eigenvalues, 
  pxr::GfVec3f eigenvectors[3])
{
   // This was adapted from the (open source) Open Inventor
    // SbMatrix::Jacobi3().

    // Initialize eigenvalues to the diagonal of the 3x3 matrix and
    // eigenvectors to the principal axes.
    eigenvalues->Set(m[0][0], m[1][1], m[2][2]);
    eigenvectors[0] = GfVec3f::XAxis();
    eigenvectors[1] = GfVec3f::YAxis();
    eigenvectors[2] = GfVec3f::ZAxis();

    GfMatrix4f a(m);
    GfVec3f   b = *eigenvalues;
    GfVec3f   z = GfVec3f(0);

    for (int i = 0; i < 50; i++) {
	float sm = 0.f;
	for (int p = 0; p < 2; p++)
	    for (int q = p+1; q < 3; q++)
		sm += GfAbs(a[p][q]);
	
	if (sm == 0.0)
	    return;
	
	float thresh = (i < 3 ? (.2 * sm / (3 * 3)) : 0.0);
	
	for (int p = 0; p < 3; p++) {
	    for (int q = p+1; q < 3; q++) {
		
		float g = 100.f * GfAbs(a[p][q]);
		
		if (i > 3 &&
		    (GfAbs((*eigenvalues)[p]) + g ==
		     GfAbs((*eigenvalues)[p])) && 
		    (GfAbs((*eigenvalues)[q]) + g ==
		     GfAbs((*eigenvalues)[q])))
		    a[p][q] = 0.f;
		
		else if (GfAbs(a[p][q]) > thresh) {
		    float h = (*eigenvalues)[q] - (*eigenvalues)[p];
		    float t;

		    if (GfAbs(h) + g == GfAbs(h)) {
			t = a[p][q] / h;
		    } else {
			float theta = 0.5f * h / a[p][q];
			t = 1.f / (GfAbs(theta) + sqrt(1.f + theta * theta));
			if (theta < 0.f)
			    t = -t;
		    }

		    // End of computing tangent of rotation angle
		    
		    float c = 1.f / sqrt(1.f + t*t);
		    float s = t * c;
		    float tau = s / (1.f + c);
		    h = t * a[p][q];
		    z[p]    -= h;
		    z[q]    += h;
		    (*eigenvalues)[p] -= h;
		    (*eigenvalues)[q] += h;
		    a[p][q] = 0.0;
		    
		    for (int j = 0; j < p; j++) {
			g = a[j][p];
			h = a[j][q];
			a[j][p] = g - s * (h + g * tau);
			a[j][q] = h + s * (g - h * tau);
		    }
		    
		    for (int j = p+1; j < q; j++) {
			g = a[p][j];
			h = a[j][q];
			a[p][j] = g - s * (h + g * tau);
			a[j][q] = h + s * (g - h * tau);
		    }
		    
		    for (int j = q+1; j < 3; j++) {
			g = a[p][j];
			h = a[q][j];
			a[p][j] = g - s * (h + g * tau);
			a[q][j] = h + s * (g - h * tau);
		    }
		    
		    for (int j = 0; j < 3; j++) {
			g = eigenvectors[j][p];
			h = eigenvectors[j][q];
			eigenvectors[j][p] = g - s * (h + g * tau);
			eigenvectors[j][q] = h + s * (g - h * tau);
		    }
		}
	    }
	}
	for (int p = 0; p < 3; p++) {
	    (*eigenvalues)[p] = b[p] += z[p];
	    z[p] = 0;
	}
    }
}
*/
void _Jacobi3(const pxr::GfMatrix4f& m, pxr::GfVec3f* eigenvalues, 
  pxr::GfVec3f eigenvectors[3])
{
  // This was adapted from the (open source) Open Inventor
  // SbMatrix::Jacobi3().

  // Initialize eigenvalues to the diagonal of the 3x3 matrix and
  // eigenvectors to the principal axes.
  eigenvalues->Set(m[0][0], m[1][1], m[2][2]);
  eigenvectors[0] = GfVec3f::XAxis();
  eigenvectors[1] = GfVec3f::YAxis();
  eigenvectors[2] = GfVec3f::ZAxis();

  GfMatrix4f a = m;
  GfVec3f   b = *eigenvalues;
  GfVec3f   z = GfVec3f(0);

  for (int i = 0; i < 50; i++) {
    float sm = 0.0;
    for (int p = 0; p < 2; p++)
      for (int q = p+1; q < 3; q++)
        sm += GfAbs(a[p][q]);
    
    if (sm == 0.0)
        return;
    
    double thresh = (i < 3 ? (.2 * sm / (3 * 3)) : 0.0);
    
    for (int p = 0; p < 3; p++) {
      for (int q = p+1; q < 3; q++) {
      
        double g = 100.0 * GfAbs(a[p][q]);
      
        if (i > 3 &&
            (GfAbs((*eigenvalues)[p]) + g ==
            GfAbs((*eigenvalues)[p])) && 
            (GfAbs((*eigenvalues)[q]) + g ==
            GfAbs((*eigenvalues)[q])))
            a[p][q] = 0.0;
      
        else if (GfAbs(a[p][q]) > thresh) {
          double h = (*eigenvalues)[q] - (*eigenvalues)[p];
          double t;

          if (GfAbs(h) + g == GfAbs(h)) {
            t = a[p][q] / h;
          } else {
            double theta = 0.5 * h / a[p][q];
            t = 1.0 / (GfAbs(theta) + sqrt(1.0 + theta * theta));
            if (theta < 0.0)
              t = -t;
          }

          // End of computing tangent of rotation angle
          
          double c = 1.0 / sqrt(1.0 + t*t);
          double s = t * c;
          double tau = s / (1.0 + c);
          h = t * a[p][q];
          z[p]    -= h;
          z[q]    += h;
          (*eigenvalues)[p] -= h;
          (*eigenvalues)[q] += h;
          a[p][q] = 0.0;
          
          for (int j = 0; j < p; j++) {
            g = a[j][p];
            h = a[j][q];
            a[j][p] = g - s * (h + g * tau);
            a[j][q] = h + s * (g - h * tau);
          }
          
          for (int j = p+1; j < q; j++) {
            g = a[p][j];
            h = a[j][q];
            a[p][j] = g - s * (h + g * tau);
            a[j][q] = h + s * (g - h * tau);
          }
          
          for (int j = q+1; j < 3; j++) {
            g = a[p][j];
            h = a[q][j];
            a[p][j] = g - s * (h + g * tau);
            a[q][j] = h + s * (g - h * tau);
          }
          
          for (int j = 0; j < 3; j++) {
            g = eigenvectors[j][p];
            h = eigenvectors[j][q];
            eigenvectors[j][p] = g - s * (h + g * tau);
            eigenvectors[j][q] = h + s * (g - h * tau);
          }
        }
      }
    }
    for (int p = 0; p < 3; p++) {
      (*eigenvalues)[p] = b[p] += z[p];
      z[p] = 0;
    }
  }
}

void _TestOne() {
  std:: cout << "\n\n############  TEST ONE   ##########################" << std::endl;

  pxr::GfMatrix4f m(1.0);
  m.SetRotate(_RandomQuaternion());

  std::cout << "MATRIX : " << m << std::endl;

  // pixar jacobi
  {
    pxr::GfVec3f eigenValues;
    pxr::GfVec3f eigenVectors[3];
    _Jacobi3(m, &eigenValues, eigenVectors);
    _PrintResult("Jacobi", &eigenValues[0], &eigenVectors[0][0]);
  }

  // eigen 
  {
    Matrix3r em3;

    em3 << m[0][0], m[1][0], m[2][0],
           m[0][1], m[1][1], m[2][1],
           m[0][2], m[1][2], m[2][2];
    Eigen::SelfAdjointEigenSolver<Matrix3r> es(em3);
    //Eigen::EigenSolver<Matrix3r> es(em3);

    _PrintResult("Eigen", es.eigenvalues().data(), 
      es.eigenvectors().data());
  
  }

  // custom iterative
  {
    pxr::GfVec3f values;
    pxr::GfMatrix3f vectors;

    pxr::SymmetricEigensolver3x3 solver;
    solver(m[0][0], m[0][1], m[0][2], m[1][1], m[1][2], m[2][2], true, 1, values, vectors);
    _PrintResult("Custom Iterative", &values[0], &vectors[0][0]);
  }

  // custom non-iterative
  {
    pxr::GfVec3f values;
    pxr::GfMatrix3f vectors;

    pxr::NISymmetricEigensolver3x3 solver;
    solver(m[0][0], m[0][1], m[0][2], m[1][1], m[1][2], m[2][2], 1, values, vectors);
    _PrintResult("Custom Non-Iterative", &values[0], &vectors[0][0]);
  }

  // matrix class
  {
    PBDMatrix<float> matrix(4, 4);
    
    for(size_t column = 0; column < 4; ++column) {
      for(size_t row = 0; row < 4; ++row) {
        matrix.SetValue(row, column, m[row][column]);
      }
    }
    /*
    std::vector<int> pivot(4);
    if(M.LUDecomposition(pivot) == PBDMatrix<float>::MATRIX_VALID) {
      std::cout << "LU DECOMPOSITION SUCCEED" << std::endl;
      std::cout << pivot[0] << "," << pivot[1] << "," << pivot[2] << "," << pivot[3]  << std::endl;
      M.SolveLU();
      
    } else {
      std::cout << "LU DECOMPOSITION FAILED" << std::endl;
    }
    */
    matrix.Echo("Matrix");
    PBDMatrix<float> inverse(4, 4);
    matrix.Inverse(inverse);
    inverse.Echo("Inverse");
    matrix.InverseInPlace();
    matrix.Echo("InverseInPlace");

    std::cout << m << std::endl;
    std::cout << m.GetInverse() << std::endl;
    //PBDMatrix<float> inverse = matrix.Inverse();
    //inverse.Echo();
    
  }
}

void _BenchMark(size_t N) {
  std::vector<pxr::GfMatrix4f> matrices(N);
  _RandomMatrices(matrices);

  std::vector<float> values0(N * 3);
  std::vector<float> vectors0(N * 9);
  std::vector<float> values1(N * 3);
  std::vector<float> vectors1(N * 9);
  std::vector<float> values2(N * 3);
  std::vector<float> vectors2(N * 9);
  std::vector<float> values3(N * 3);
  std::vector<float> vectors3(N * 9);


  std:: cout << "\n\n############  BENCH MARK   ##########################" << std::endl;
  uint64_t startT; 
  // jacobi
  { 
    startT = CurrentTime();
    for(size_t i=0; i < N; ++i) {
      const pxr::GfMatrix4f& m = matrices[i];
      pxr::GfVec3f* eigenValues = (pxr::GfVec3f*)&values0[i * 3];
      pxr::GfVec3f* eigenVectors = (pxr::GfVec3f*)&vectors0[i * 9];

      _Jacobi3(m, eigenValues, eigenVectors);
    }
    uint64_t T0 = CurrentTime() - startT;
    _PrintBenchMark("Jacobi", CurrentTime() - startT);
  }

  // eigen
  {
    startT = CurrentTime();
    Matrix3r em3;
    for(size_t i=0; i < N; ++i) {
      const pxr::GfMatrix4f& m = matrices[i];
      
      em3 << m[0][0], m[0][1], m[0][2],
             m[1][0], m[1][1], m[1][2],
             m[2][0], m[2][1], m[2][2];
             
      Eigen::SelfAdjointEigenSolver<Matrix3r> es(em3);
      memcpy(&values1[i*3], &es.eigenvalues(), 3 * sizeof(float));
      memcpy(&vectors1[i*9], &es.eigenvectors(), 9 * sizeof(float));
    }
    _PrintBenchMark("Eigen", CurrentTime() - startT);
  }

  // custom iterative
  {
    startT = CurrentTime();
    for(size_t i=0; i < N; ++i) {
      const pxr::GfMatrix4f& m = matrices[i];
      pxr::GfVec3f& values = *(pxr::GfVec3f*)&values3[i * 3];
      pxr::GfMatrix3f& vectors = *(pxr::GfMatrix3f*)&vectors3[i * 9];;

      pxr::SymmetricEigensolver3x3 solver;
      solver(m[0][0], m[0][1], m[0][2], m[1][1], m[1][2], m[2][2], true, 1, values, vectors);
    }
    _PrintBenchMark("Custom iterative", CurrentTime() - startT);
  }

  // custom non-iterative
  {
    startT = CurrentTime();
    for(size_t i=0; i < N; ++i) {
      const pxr::GfMatrix4f& m = matrices[i];
      pxr::GfVec3f& values = *(pxr::GfVec3f*)&values3[i * 3];
      pxr::GfMatrix3f& vectors = *(pxr::GfMatrix3f*)&vectors3[i * 9];;

      pxr::NISymmetricEigensolver3x3 solver;
      solver(m[0][0], m[0][1], m[0][2], m[1][1], m[1][2], m[2][2], 1, values, vectors);
    }
    _PrintBenchMark("Custom non-iterative", CurrentTime() - startT);
  }
}

bool _CompareDatas(float* lhs, float* rhs, size_t N)
{
  for(size_t x = 0; x < N; ++x) {
    if(std::abs(lhs[x] - rhs[x]) > 0.000001f)return false;
  }
  return true;
}

// benchmark matrix class efficency (compared to pxr::GfMatrix4)
void _BenchMark2(size_t N) {
  std::vector<pxr::GfMatrix4f> matrices(N);
  _RandomMatrices(matrices);

  std::vector<PBDMatrix<float>> inverses0(N);
  std::vector<pxr::GfMatrix4f> inverses1(N);
  std::vector<Matrix4r> inverses2(N);

  std:: cout << "\n\n############  BENCH MARK 2  ##########################" << std::endl;
  uint64_t startT; 
  
  // custom
  {
    startT = CurrentTime();
    PBDMatrix<float> matrix(4, 4);
    for(size_t i=0; i < N; ++i) {
      memcpy(matrix.Data(), &matrices[i][0][0], 16 * sizeof(float));
      matrix.Inverse(inverses0[i]);
    }
    _PrintBenchMark("CUSTOM INVERSE", CurrentTime() - startT);
  }

  // pixar
  { 
    startT = CurrentTime();

    for(size_t i=0; i < N; ++i) {
      inverses1[i] = matrices[i].GetInverse();
    }
    uint64_t T0 = CurrentTime() - startT;
    _PrintBenchMark("PIXAR INVERSE", CurrentTime() - startT);
  }

  // eigen
  {
    
    startT = CurrentTime();
    Matrix4r em4;
    for(size_t i=0; i < N; ++i) {
      const pxr::GfMatrix4f& m = matrices[i];
      
      em4 << m[0][0], m[1][0], m[2][0], m[3][0],
             m[0][1], m[1][1], m[2][1], m[3][1],
             m[0][2], m[1][2], m[2][2], m[3][2],
             m[0][3], m[1][3], m[2][3], m[3][3];

      Eigen::FullPivLU<Matrix4r> lu(em4);
             
      inverses2[i] = lu.inverse();
    }
    _PrintBenchMark("EIGEN INVERSE", CurrentTime() - startT);
    
  }

  std::vector<float> D0(N * 16);
  std::vector<float> D1(N * 16);
  std::vector<float> D2(N * 16);

  for(size_t n = 0; n < N; ++n){
    
    memcpy(&D0[n * 16], inverses0[n].Data(), 16 * sizeof(float));
    memcpy(&D1[n * 16], inverses1[n].data(), 16 * sizeof(float));
    memcpy(&D2[n * 16], inverses2[n].data(), 16 * sizeof(float));
  }
  std::cout << "EQUALS ? " << _CompareDatas(&D0[0], &D1[0], N * 16) << std::endl;
  std::cout << "EQUALS ? " << _CompareDatas(&D0[0], &D2[0], N * 16) << std::endl;
  
}

void _BenchMark3(size_t N)
{
  std:: cout << "\n\n############  BENCH MARK 2  ##########################" << std::endl;

  std::vector<pxr::GfMatrix4f> matrices(1);
  _RandomMatrices(matrices);
  pxr::GfVec4f v(0.5, 0.5, 0.0, 1.0);
  pxr::GfVec4f r;

  Eigen::Matrix4f A;
  Eigen::Vector4f x;
  for(size_t idx = 0; idx < 16; ++idx) {
    A(idx/4, idx%4) = matrices[0].data()[idx] ;
  }
  for(size_t idx = 0; idx < 4; ++idx) {
    x(idx) = v[idx];
  }
  std::cout << "Here is the matrix A:\n" << A << std::endl;
  std::cout << "Computing LLT decomposition..." << std::endl;

  
  std::cout << "The solution is:\n" << A.llt().solve(x) << std::endl;

  Eigen::MatrixXf L = A.llt().matrixL(); // retrieve factor L  in the decomposition
  // The previous two lines can also be written as "L = A.llt().matrixL()"
 
std::cout << "The Cholesky factor L is" << std::endl << L << std::endl;
std::cout << "To check this, let us compute L * L.transpose()" << std::endl;
std::cout << L * L.transpose() << std::endl;
std::cout << "This should equal the matrix A" << std::endl;


  PBDMatrix<float> matrix(4, 4);
  memcpy(matrix.Data(), matrices[0].data(), 16 * sizeof(float));

  matrix.Solve(&v[0], &r[0]);
  std::cout << "The solution is:\n" << r[0] << "," << r[1] << "," << r[2] << "," << r[3]<< std::endl;

  std::cout << "The solution is:\n" << A.inverse() * x << std::endl;
}

int main (int argc, char *argv[])
{
  _TestOne();
  size_t N = 10000;
  _BenchMark(N);

  _BenchMark2(N);
  _BenchMark3(0);
  return 0;
}