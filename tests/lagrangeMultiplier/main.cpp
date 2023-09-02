#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/quatf.h>
#include <pxr/base/gf/matrix3f.h>
#include <pxr/base/gf/matrix4f.h>
#include <iostream>

#include <Eigen/Core>

using VectorXf = Eigen::Matrix<float, Eigen::Dynamic, 1>;

#include "../../src/utils/timer.h"

#include "../../src/geometry/matrix.h"

PXR_NAMESPACE_USING_DIRECTIVE

const size_t N = 6;

float LagrangeMultiplierCustom(size_t N, pxr::GfVec3f* grad, float* w)
{
  float result = 0.f;
  for(size_t n = 0; n < N; ++n) {
    const float m = w[n];
    result += 
      grad[n][0] * m * grad[n][0] +
      grad[n][1] * m * grad[n][1] +
      grad[n][2] * m * grad[n][2];
  }
  return result;
}

float LagrangeMultiplierEigen(size_t N, const VectorXf& gradient, float* w)
{
  VectorXf inv_M(N * 3);
  for (size_t n = 0; n < N; ++n)
  {
    inv_M(n * 3 + 0) = w[n];
    inv_M(n * 3 + 1) = w[n];
    inv_M(n * 3 + 2) = w[n];
  }

  return gradient.transpose() * inv_M.asDiagonal() * gradient;
}

int main (int argc, char *argv[])
{
  int64_t sT, eT;
  float eR, cR;
  for(size_t n = 16; n <= 4096; n <<= 1) {
    pxr::GfVec3f grad[n];
    float w[n];
    for(size_t i = 0; i < n; ++i) {
      grad[i] = pxr::GfVec3f(RANDOM_LO_HI(-1, 1), RANDOM_LO_HI(-1, 1), RANDOM_LO_HI(-1, 1));
      w[i] = RANDOM_0_1;
    }

    VectorXf gradient(n * 3);
    for (unsigned int m = 0; m < n; ++m)
    {
      gradient(m * 3 + 0) = grad[m][0];
      gradient(m * 3 + 1) = grad[m][1];
      gradient(m * 3 + 2) = grad[m][2];
    }

    sT = CurrentTime();
    eR = LagrangeMultiplierEigen(n, gradient, w);
    eT = CurrentTime() - sT;
    std::cout << "eigen  (" << n << ") : " << eR << " took " << eT << std::endl;
   
    sT = CurrentTime();
    cR = LagrangeMultiplierCustom(n, grad, w);
    eT = CurrentTime() - sT;
    std::cout << "custom (" << n << ") : " << cR << " took " << eT << std::endl;

  }

  return 0;
  
}