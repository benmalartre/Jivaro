#include "../geometry/kabsch.h"


JVR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------
// setup binds (static) points
//-------------------------------------------------------
void 
KabschSolver::Bind(const VtArray<GfVec3f>& positions, 
  const GfMatrix4d& matrix, const VtArray<int>& indices) 
{
  int numBinds = indices.size();
  _binds.resize(numBinds);
  if(!numBinds)return;

  _bndCentroid = GfVec3f(0.f);

  // get world points
  for(int i = 0; i < numBinds; ++i)_binds[i] = matrix.Transform(positions[indices[i]]);

  // compute centroid
  for (int i = 0; i < numBinds; ++i) _bndCentroid += _binds[i];
  _bndCentroid /= numBinds;

  // shift binds to origin
  for (int i = 0; i < numBinds; ++i) _binds[i] -= _bndCentroid;

  // cache num points
  _numPoints = numBinds;
}

//-------------------------------------------------------
// setup input (deform) points
//-------------------------------------------------------
void 
KabschSolver::Update(const VtArray<GfVec3f>& positions, 
  const GfMatrix4d& matrix, const VtArray<int>& indices) 
{
  size_t numPoints = indices.size();
  _pntCentroid = GfVec3f(0.f);

  if(numPoints == _numPoints){
    _points.resize(_numPoints);

    // get world points
    for(int i=0;i<_numPoints;++i)_points[i] = matrix.Transform(positions[indices[i]]);

    // compute centroid
    for (int i = 0; i < _numPoints; i++) _pntCentroid += _points[i];
    _pntCentroid /= _numPoints;

    // shift binds to origin
    for (int i = 0; i < _numPoints; i++)_points[i] -= _pntCentroid;
  }
}

const GfMatrix4d&
KabschSolver::Solve(bool solveRotation, bool solveScale, bool firstSolve)
{
  static const GfMatrix4d _identity = GfMatrix4d(1.0);
  if(_points.size() != _binds.size())return _identity;
  size_t numPoints = _points.size();

  // calculate the scale ratio
  if (solveScale) {
    float pntScale = 0.0f, bndScale = 0.0f;
    for (int i = 0; i < numPoints; i++) {
      pntScale += _points[i].GetLength();
      bndScale += _binds[i].GetLength();
    }
    _scale = (bndScale / pntScale);
  }

  // calculate the 3x3 covariance matrix, and the optimal rotation
  if (solveRotation) {
    if(firstSolve) {
      VtArray<GfVec3f> randomizedPoints(numPoints);
      for(size_t i=0; i < numPoints; ++i) {
        randomizedPoints[i] = 
          _points[i] + GfVec3f(
            (float)rand() / (float)RAND_MAX,
            (float)rand() / (float)RAND_MAX,
            (float)rand() / (float)RAND_MAX);
      }
      GfMatrix4d covariantMatrix = 
        _ComputeCovarianceMatrix(randomizedPoints, _binds, _pntCentroid, _bndCentroid);
      _ExtractRotation(covariantMatrix, &_rotation, 16);
      covariantMatrix = 
        _ComputeCovarianceMatrix(_points, _binds, _pntCentroid, _bndCentroid);
      _ExtractRotation(covariantMatrix, &_rotation, 32);
    } else {
      GfMatrix4d covariantMatrix = 
        _ComputeCovarianceMatrix(_points, _binds, _pntCentroid, _bndCentroid);
      _ExtractRotation(covariantMatrix, &_rotation, 16);
    }
  }
  GfTransform S, R, T;
  GfVec3f scale(_scale);
  S.SetScale(scale);
  R.SetRotation(_rotation);
  T.SetTranslation(_pntCentroid);

  _solved = S.GetMatrix() * R.GetMatrix() * T.GetMatrix();
  return _solved;
}


void KabschSolver::_BuildMatrixFromQuaternion(const GfQuatd& quat, GfMatrix4d* matrix) {
  GfVec3f right(quat.Transform(GfVec3d(1,0,0)));
  GfVec3f up(quat.Transform(GfVec3d(0,1,0)));
  GfVec3f forward(quat.Transform(GfVec3d(0,0,1)));

  *matrix[0][0] = right[0];
  *matrix[0][1] = right[1];
  *matrix[0][2] = right[2];

  *matrix[1][0] = up[0];
  *matrix[1][1] = up[1];
  *matrix[1][2] = up[2];

  *matrix[2][0] = forward[0];
  *matrix[2][1] = forward[1];
  *matrix[2][2] = forward[2];
}

//https://animation.rwth-aachen.de/media/papers/2016-MIG-StableRotation.pdf
//Iteratively apply torque to the basis using Cross products (in place of SVD)
void 
KabschSolver::_ExtractRotation(const GfMatrix4d& matrix, GfQuatd* rotation, size_t iterations)
{
  GfQuatd rotation2;

  GfVec3f matrix0(matrix[0][0], matrix[0][1], matrix[0][2]);
  GfVec3f matrix1(matrix[1][0], matrix[1][1], matrix[1][2]);
  GfVec3f matrix2(matrix[2][0], matrix[2][1], matrix[2][2]);

  for(int iter = 0;iter < iterations; ++iter)
  {
    _BuildMatrixFromQuaternion(*rotation, &_basis);
    GfVec3f basis0(_basis[0][0], _basis[0][1], _basis[0][2]);
    GfVec3f basis1(_basis[1][0], _basis[1][1], _basis[1][2]);
    GfVec3f basis2(_basis[2][0], _basis[2][1], _basis[2][2]);

    GfVec3f omega = ((basis0 ^ matrix0) + (basis1 ^ matrix1) + (basis2 ^ matrix2)) *
    (1.0f / GfAbs((basis0 * matrix0) + (basis1 * matrix1) + (basis2 * matrix2) + KABSCH_EPSILON));

    float w = omega.GetLength();
    if(w < KABSCH_EPSILON) break;

    rotation2 = GfQuatd(w, omega / w);
    rotation2 *= *rotation;
    *rotation = rotation2.GetNormalized();
  }
}


const GfMatrix4d& 
KabschSolver::_ComputeCovarianceMatrix( const VtArray<GfVec3f>& vec1,
                                        const VtArray<GfVec3f>& vec2,
                                        const GfVec3f& centroid1,
                                        const GfVec3f& centroid2)
{
  _covariance = GfMatrix4d();

  for (int k = 0; k < vec1.size(); ++k) {//k is the column in this matrix
    GfVec3f lhs = vec1[k];// *vec2[k][3];
    GfVec3f rhs = vec2[k];// *GfAbs(vec2[k][3]);

    _covariance[0][0] += lhs[0]*rhs[0];
    _covariance[1][0] += lhs[1]*rhs[0];
    _covariance[2][0] += lhs[2]*rhs[0];
    _covariance[0][1] += lhs[0]*rhs[1];
    _covariance[1][1] += lhs[1]*rhs[1];
    _covariance[2][1] += lhs[2]*rhs[1];
    _covariance[0][2] += lhs[0]*rhs[2];
    _covariance[1][2] += lhs[1]*rhs[2];
    _covariance[2][2] += lhs[2]*rhs[2];
  }
  return _covariance;
}

JVR_NAMESPACE_CLOSE_SCOPE