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
    matrix = pxr::GfMatrix4f(1.0).SetRotate(_RandomQuaternion());
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

int main (int argc, char *argv[])
{
  _TestOne();
  size_t N = 10000;
  _BenchMark(N);
  return 0;
}