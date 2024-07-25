#ifndef JVR_PBD_UTILS_H
#define JVR_PBD_UTILS_H



#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/matrix3d.h>
#include <pxr/base/gf/quatd.h>
#include <pxr/base/gf/transform.h>

#include "../common.h"


JVR_NAMESPACE_OPEN_SCOPE

/*
void ExtractRotation(const pxr::GfMatrix3d &A, pxr::GfQuaternion &q,
  const unsigned int maxIter)
{
  for (unsigned int iter = 0; iter < maxIter; ++iter)
  {
    pxr::GfMatrix3d R = q.Matrix();
    pxr::GfVec3d omega = 
      (R.GetColumn(0) ^ A.GetColumn(0) + 
       R.GetColumn(1) ^ A.GetColumn(1) + 
       R.GetColumn(2) ^ A.GetColumn(2) 
      ) * (
        1.0 / pxr::GfAbs(
          pxr::GfDot(R.GetColumn(0), A.GetColumn(0)) + 
          pxr::GfDot(R.GetColumn(1), A.GetColumn(1)) + 
          pxr::GfDot(R.GetColumn(2), A.GetColumn(2))) + 1.0e-9);
    double w = omega.GetLength();
    if (w < 1.0e-9) break;
    q = pxr::GfQuaterniond(AngleAxisd(w, (1.0 / w) * omega)) * q;
    q.Normalize();
  }
}
*/

pxr::GfMatrix4d InterpolateMatrices(const pxr::GfMatrix4d& m0, const pxr::GfMatrix4d& m1, float t)
{
  
  pxr::GfVec3f s0(
    pxr::GfVec3f(m0[0][0], m0[1][0], m0[2][0]).GetLength(),
    pxr::GfVec3f(m0[0][1], m0[1][1], m0[2][1]).GetLength(),
    pxr::GfVec3f(m0[0][2], m0[1][2], m0[2][2]).GetLength());

  pxr::GfVec3f s1(
    pxr::GfVec3f(m1[0][0], m1[1][0], m1[2][0]).GetLength(),
    pxr::GfVec3f(m1[0][1], m1[1][1], m1[2][1]).GetLength(),
    pxr::GfVec3f(m1[0][2], m1[1][2], m1[2][2]).GetLength());

  const pxr::GfTransform interpolated(
    s0 * t + s1 * (1 - t),                                                // interpolated scale
    pxr::GfRotation(),                                                    // pivot orientation
    m0.ExtractRotation() * t * m1.ExtractRotation() * (1 - t),            // interpolated rotation
    pxr::GfVec3d(0.f),                                                    // pivot position
    m0.ExtractTranslation() * t + m1.ExtractTranslation() * (1 - t)       // interpolated translation
  );
  return interpolated.GetMatrix();
}

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_UTILS_H