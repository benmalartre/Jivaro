#ifndef JVR_GEOMETRY_RBF_H
#define JVR_GEOMETRY_RBF_H

#include <vector>
#include <pxr/base/gf/math.h>
#include "../common.h"
#include "../geometry/matrix.h"


JVR_NAMESPACE_OPEN_SCOPE

class RBF {

  enum Kernel {
    LINEAR,
    GAUSSIAN,
    CUBIC,
    QUINTIC,
    INVERSE,
    MULTIQUADRIC,
    THINPLATE
  };

  using Matrix = Matrix<float>;
  using Vector = std::vector<float>;

  typedef float (*PFNRBFKERNEL)(float r, float eps);

  RBF();
  ~RBF();
  void SetKernelType(int type);

  void Init(const Matrix& keys, const Matrix& values);
  void Interpolate(const Matrix& querys, Matrix* result);

private:
  void _ComputeEpsilon(const Matrix& matrix);
  bool          _initialized;
  float         _epsilon;
  int           _type;
  PFNRBFKERNEL  _kernel;
  Matrix        _A;       // column vector
  Matrix        _keys;    // keys
  Matrix        _values;  // values

  // Interpolator Kernels
  static float _MultiQuadricKernel(float r, float eps) {
    return GfSqrt(GfPow(r / eps, 2) + 1);
  }

  static float _InverseKernel(float r, float eps) {
    return 1.0 / GfSqrt(GfPow(r / eps, 2) + 1);
  }

  static float _GaussianKernel(float r, float eps) {
    return GfExp(-GfPow(r / eps, 2));
  }

  static float _LinearKernel(float r) {
    return r;
  }

  static float _CubicKernel(float r) {
    return GfPow(r, 3);
  }

  static float _QuinticKernel(float r) {
    return GfPow(r, 5);
  }

  static float _ThinPlateKernel(float r) {
    return (GfPow(r, 2) * GfLog(r)) * (r > 0);
  }
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_RBF_H