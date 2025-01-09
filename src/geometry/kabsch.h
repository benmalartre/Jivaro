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
  GfMatrix4d             _solved;
  GfMatrix4d             _basis;
  GfMatrix4d             _covariance;
  GfQuatd                _rotation;
  float                       _scale;

  int                         _numPoints;
  VtArray<GfVec3f>  _points;
  VtArray<GfVec3f>  _binds;

  GfVec3f                _pntCentroid;
  GfVec3f                _bndCentroid;

public:
  // setup bind points (init)
  void Bind(const VtArray<GfVec3f>& positions, const GfMatrix4d& matrix, 
    const VtArray<int>& indices);

  // update deform points (update)
  void Update(const VtArray<GfVec3f>& positions, const GfMatrix4d& matrix, 
    const VtArray<int>& indices);

  // solve the the kabsch covariant transform
  const GfMatrix4d& Solve(bool solveRotation=true, bool solveScale=false, bool firstSolve=false);

private:
  //https://animation.rwth-aachen.de/media/papers/2016-MIG-StableRotation.pdf
  //Iteratively apply torque to the basis using Cross products (in place of SVD)
  void _ExtractRotation(const GfMatrix4d& matrix,  GfQuatd* rotation, size_t iterations);

  // calculate Covariance Matrix
  const GfMatrix4d& _ComputeCovarianceMatrix(const VtArray<GfVec3f>& vec1,
                                                  const VtArray<GfVec3f>& vec2,
                                                  const GfVec3f& centroid1,
                                                  const GfVec3f& centroid2);

  //Build Matrix from Quaternion
  void _BuildMatrixFromQuaternion(const GfQuatd& quat, GfMatrix4d* matrix);

};

struct KabschData {
  KabschSolver        solver;
  GfMatrix4d     matrix;
  GfMatrix4d     bindMatrix;
  GfMatrix4d     deformMatrix;
  VtArray<int>   indices;
  VtArray<float> weights;
  int                 flags;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_KABSCH_H