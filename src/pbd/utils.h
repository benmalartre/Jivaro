#ifndef JVR_PBD_UTILS_H
#define JVR_PBD_UTILS_H



#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/matrix3d.h>
#include <pxr/base/gf/quatd.h>
#include <pxr/base/gf/transform.h>

#include "../common.h"


JVR_NAMESPACE_OPEN_SCOPE

/*
void ExtractRotation(const GfMatrix3d &A, GfQuaternion &q,
  const unsigned int maxIter)
{
  for (unsigned int iter = 0; iter < maxIter; ++iter)
  {
    GfMatrix3d R = q.Matrix();
    GfVec3d omega = 
      (R.GetColumn(0) ^ A.GetColumn(0) + 
       R.GetColumn(1) ^ A.GetColumn(1) + 
       R.GetColumn(2) ^ A.GetColumn(2) 
      ) * (
        1.0 / GfAbs(
          GfDot(R.GetColumn(0), A.GetColumn(0)) + 
          GfDot(R.GetColumn(1), A.GetColumn(1)) + 
          GfDot(R.GetColumn(2), A.GetColumn(2))) + 1.0e-9);
    double w = omega.GetLength();
    if (w < 1.0e-9) break;
    q = GfQuaterniond(AngleAxisd(w, (1.0 / w) * omega)) * q;
    q.Normalize();
  }
}
*/

GfMatrix4d InterpolateMatrices(const GfMatrix4d& m0, const GfMatrix4d& m1, float t)
{
  
  GfVec3f s0(
    GfVec3f(m0[0][0], m0[1][0], m0[2][0]).GetLength(),
    GfVec3f(m0[0][1], m0[1][1], m0[2][1]).GetLength(),
    GfVec3f(m0[0][2], m0[1][2], m0[2][2]).GetLength());

  GfVec3f s1(
    GfVec3f(m1[0][0], m1[1][0], m1[2][0]).GetLength(),
    GfVec3f(m1[0][1], m1[1][1], m1[2][1]).GetLength(),
    GfVec3f(m1[0][2], m1[1][2], m1[2][2]).GetLength());

  const GfTransform interpolated(
    s0 * t + s1 * (1 - t),                                                // interpolated scale
    GfRotation(),                                                    // pivot orientation
    m0.ExtractRotation() * t * m1.ExtractRotation() * (1 - t),            // interpolated rotation
    GfVec3d(0.f),                                                    // pivot position
    m0.ExtractTranslation() * t + m1.ExtractTranslation() * (1 - t)       // interpolated translation
  );
  return interpolated.GetMatrix();
}

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_UTILS_H