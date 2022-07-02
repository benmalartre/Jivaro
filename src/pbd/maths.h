#ifndef JVR_PBD_MATH_FUNCTIONS_H
#define JVR_PBD_MATH_FUNCTIONS_H

#include "../common.h"
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/quatf.h>
#include <pxr/base/gf/matrix3f.h>

JVR_NAMESPACE_OPEN_SCOPE
class PBDMath
{
private:
  static void JacobiRotate(pxr::GfMatrix3f&A, pxr::GfMatrix3f& R, int p, int q);

public:
static float InfNorm(const pxr::GfMatrix3f& A);
static float OneNorm(const pxr::GfMatrix3f& A);

static void EigenDecomposition(const pxr::GfMatrix3f& A,
  pxr::GfMatrix3f& eigenVecs, pxr::GfVec3f& eigenVals);

static void PolarDecomposition(const pxr::GfMatrix3f& A,
  pxr::GfMatrix3f& R, pxr::GfMatrix3f& U, pxr::GfMatrix3f& D);

static void PolarDecompositionStable(const pxr::GfMatrix3f &M,
  const float tolerance, pxr::GfMatrix3f& R);

static void SvdWithInversionHandling(const pxr::GfMatrix3f& A,
  pxr::GfVec3f& sigma, pxr::GfMatrix3f& U, pxr::GfMatrix3f& VT);

static float CotTheta(const pxr::GfVec3f& v, const pxr::GfVec3f& w);

static void CrossProductMatrix(const pxr::GfVec3f& v, pxr::GfMatrix3f& v_hat);

static void ExtractRotation(const pxr::GfMatrix3f& A, pxr::GfQuatf& q, 
  const unsigned int maxIter);

template<typename T>
static void CholeskyDecomposition(const T* matrix, T* result, int n);

};

<<<<<<< HEAD
JVR_NAMESPACE_CLOSE_SCOPE
=======
template<typename T>
void PBDMath::CholeskyDecomposition(const T* matrix, T* result, int n)
{
  std::cout << "CHOLESKY DECOMPOSITION" << std::endl;
  memset(result, 0, n * n * sizeof(T));

  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j <= i; j++) {
      T sum = 0;
      for (size_t k = 0; k < j; k++)
        sum += result[i*n+k] * result[j*n+k];

      if (i == j)
        result[i*n+j] = pxr::GfSqr(matrix[i*n+i] - sum);
      else
        result[i*n+j] = (1.0 / result[j*n+j] * (matrix[i*n+j] - sum));
    }
  }
}

PXR_NAMESPACE_CLOSE_SCOPE
>>>>>>> 61b4014121619805d24d40f056c78b6a159056db

#endif