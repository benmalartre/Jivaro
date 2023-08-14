#ifndef JVR_PBD_UTILS_H
#define JVR_PBD_UTILS_H

#include <pxr/usd/usd/stage.h>
#include "../common.h"


#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/matrix3d.h>
#include <pxr/base/gf/quatd.h>



JVR_NAMESPACE_OPEN_SCOPE

void ExtractRotation(const pxr::GfMatrix3d &A, pxr::GfQuaternion &q,
  const unsigned int maxIter)
{
  for (unsigned int iter = 0; iter < maxIter; ++iter)
  {
    pxr::GfMatrix3 R = q.Matrix();
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

#endif // JVR_PBD_UTILS_H