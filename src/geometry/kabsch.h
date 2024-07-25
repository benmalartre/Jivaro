#ifndef JVR_GEOMETRY_KABSCH_H
#define JVR_GEOMETRY_KABSCH_H

#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/quatd.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/transform.h>
#include <pxr/base/vt/array.h>

#include "../common.h"


JVR_NAMESPACE_OPEN_SCOPE

#define KABSCH_EPSILON 0.0000001f


class KabschSolver{
private:
  pxr::GfMatrix4d             _solved;
  pxr::GfMatrix4d             _basis;
  pxr::GfMatrix4d             _covariance;
  pxr::GfQuatd                _rotation;
  float                       _scale;

  int                         _numPoints;
  pxr::VtArray<pxr::GfVec3f>  _points;
  pxr::VtArray<pxr::GfVec3f>  _binds;

  pxr::GfVec3f                _pntCentroid;
  pxr::GfVec3f                _bndCentroid;

public:
  // setup bind points (init)
  void Bind(const pxr::VtArray<pxr::GfVec3f>& positions, const pxr::GfMatrix4d& matrix, 
    const pxr::VtArray<int>& indices);

  // update deform points (update)
  void Update(const pxr::VtArray<pxr::GfVec3f>& positions, const pxr::GfMatrix4d& matrix, 
    const pxr::VtArray<int>& indices);

  // solve the the kabsch covariant transform
  const pxr::GfMatrix4d& Solve(bool solveRotation=true, bool solveScale=false, bool firstSolve=false);

private:
  //https://animation.rwth-aachen.de/media/papers/2016-MIG-StableRotation.pdf
  //Iteratively apply torque to the basis using Cross products (in place of SVD)
  void _ExtractRotation(const pxr::GfMatrix4d& matrix,  pxr::GfQuatd* rotation, size_t iterations);

  // calculate Covariance Matrix
  const pxr::GfMatrix4d& _ComputeCovarianceMatrix(const pxr::VtArray<pxr::GfVec3f>& vec1,
                                                  const pxr::VtArray<pxr::GfVec3f>& vec2,
                                                  const pxr::GfVec3f& centroid1,
                                                  const pxr::GfVec3f& centroid2);

  //Build Matrix from Quaternion
  void _BuildMatrixFromQuaternion(const pxr::GfQuatd& quat, pxr::GfMatrix4d* matrix);

};

struct KabschData {
  KabschSolver        solver;
  pxr::GfMatrix4d     matrix;
  pxr::GfMatrix4d     bindMatrix;
  pxr::GfMatrix4d     deformMatrix;
  pxr::VtArray<int>   indices;
  pxr::VtArray<float> weights;
  int                 flags;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_KABSCH_H