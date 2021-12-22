#ifndef JVR_PBD_MATH_FUNCTIONS_H
#define JVR_PBD_MATH_FUNCTIONS_H

#include "../common.h"
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/quatf.h>
#include <pxr/base/gf/matrix3f.h>

PXR_NAMESPACE_OPEN_SCOPE
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
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif