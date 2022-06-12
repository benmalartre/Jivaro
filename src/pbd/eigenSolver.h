// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.16

#ifndef JVR_PBD_EIGENSOLVER_H
#define JVR_PBD_EIGENSOLVER_H

#include "../common.h"
#include <algorithm>
#include <array>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix3f.h>



// The document
// https://www.geometrictools.com/Documentation/RobustEigenSymmetric3x3.pdf
// describes algorithms for solving the eigensystem associated with a 3x3
// symmetric real-valued matrix.  The iterative algorithm is implemented
// by class SymmmetricEigensolver3x3.  The noniterative algorithm is
// implemented by class NISymmetricEigensolver3x3.  The code does not use
// GTEngine objects.

PXR_NAMESPACE_OPEN_SCOPE

class SortEigenstuff
{
public:
  void operator()(int sortType, bool isRotation,
    pxr::GfVec3f& values, pxr::GfMatrix3f& vectors)
  {
    if (sortType != 0) {
      // Sort the eigenvalues to values[0] <= values[1] <= values[2].
      std::array<size_t, 3> index;
      if (values[0] < values[1]) {
        if (values[2] < values[0]) {
          // even permutation
          index[0] = 2;
          index[1] = 0;
          index[2] = 1;
        } else if (values[2] < values[1]) {
          // odd permutation
          index[0] = 0;
          index[1] = 2;
          index[2] = 1;
          isRotation = !isRotation;
        } else {
          // even permutation
          index[0] = 0;
          index[1] = 1;
          index[2] = 2;
        }
      } else {
        if (values[2] < values[1]) {
          // odd permutation
          index[0] = 2;
          index[1] = 1;
          index[2] = 0;
          isRotation = !isRotation;
        } else if (values[2] < values[0]) {
          // even permutation
          index[0] = 1;
          index[1] = 2;
          index[2] = 0;
        } else {
          // odd permutation
          index[0] = 1;
          index[1] = 0;
          index[2] = 2;
          isRotation = !isRotation;
        }
      }

      if (sortType == -1) {
        // The request is for values[0] >= values[1] >= values[2]. This
        // requires an odd permutation, (i0,i1,i2) -> (i2,i1,i0).
        std::swap(index[0], index[2]);
        isRotation = !isRotation;
      }

      pxr::GfVec3f unorderedValues(values);
      pxr::GfVec3f unorderedVectors[3] = {vectors.GetRow(0), vectors.GetRow(1), vectors.GetRow(2)};
      for (size_t j = 0; j < 3; ++j) {
          size_t i = index[j];
          values[j] = unorderedValues[i];
          vectors.SetRow(j, unorderedVectors[i]);
      }
    }

    // Ensure the ordered eigenvectors form a right-handed basis.
    if (!isRotation) {
      for (size_t j = 0; j < 3; ++j) {
          vectors[2][j] = -vectors[2][j];
      }
    }
  }
};

class SymmetricEigensolver3x3
{
public:
  // The input matrix must be symmetric, so only the unique elements
  // must be specified: a00, a01, a02, a11, a12, and a22.
  //
  // If 'aggressive' is 'true', the iterations occur until a
  // superdiagonal entry is exactly zero.  If 'aggressive' is 'false',
  // the iterations occur until a superdiagonal entry is effectively
  // zero compared to the/ sum of magnitudes of its diagonal neighbors.
  // Generally, the nonaggressive convergence is acceptable.
  //
  // The order of the eigenvalues is specified by sortType:
  // -1 (decreasing), 0 (no sorting) or +1 (increasing).  When sorted,
  // the eigenvectors are ordered accordingly, and
  // {vectors[0], vectors[1], vectors[2]} is guaranteed to/ be a right-handed
  // orthonormal set.  The return value is the number of iterations
  // used by the algorithm.

  int operator()(float a00, float a01, float a02, float a11, float a12, 
    float a22, bool aggressive, int sortType, pxr::GfVec3f& values,
    pxr::GfMatrix3f& vectors) const
  {
    // Compute the Householder reflection H and B = H*A*H, where
    // b02 = 0.
    float const zero = (float)0, one = (float)1, half = (float)0.5;
    bool isRotation = false;
    float c, s;
    GetCosSin(a12, -a02, c, s);
    float Q[3][3] = { { c, s, zero }, { s, -c, zero }, { zero, zero, one } };
    float term0 = c * a00 + s * a01;
    float term1 = c * a01 + s * a11;
    float b00 = c * term0 + s * term1;
    float b01 = s * term0 - c * term1;
    term0 = s * a00 - c * a01;
    term1 = s * a01 - c * a11;
    float b11 = s * term0 - c * term1;
    float b12 = s * a02 - c * a12;
    float b22 = a22;

    // Givens reflections, B' = G^T*B*G, preserve tridiagonal
    // matrices.
    int const maxIteration = 2 * (1 + std::numeric_limits<float>::digits -
      std::numeric_limits<float>::min_exponent);
    int iteration;
    float c2, s2;

    if (std::fabs(b12) <= std::fabs(b01)) {
      float saveB00, saveB01, saveB11;
      for (iteration = 0; iteration < maxIteration; ++iteration)
      {
        // Compute the Givens reflection.
        GetCosSin(half * (b00 - b11), b01, c2, s2);
        s = std::sqrt(half * (one - c2));  // >= 1/sqrt(2)
        c = half * s2 / s;

        // Update Q by the Givens reflection.
        Update0(Q, c, s);
        isRotation = !isRotation;

        // Update B <- Q^T*B*Q, ensuring that b02 is zero and
        // |b12| has strictly decreased.
        saveB00 = b00;
        saveB01 = b01;
        saveB11 = b11;
        term0 = c * saveB00 + s * saveB01;
        term1 = c * saveB01 + s * saveB11;
        b00 = c * term0 + s * term1;
        b11 = b22;
        term0 = c * saveB01 - s * saveB00;
        term1 = c * saveB11 - s * saveB01;
        b22 = c * term1 - s * term0;
        b01 = s * b12;
        b12 = c * b12;

        if (Converged(aggressive, b00, b11, b01))
        {
          // Compute the Householder reflection.
          GetCosSin(half * (b00 - b11), b01, c2, s2);
          s = std::sqrt(half * (one - c2));
          c = half * s2 / s;  // >= 1/sqrt(2)

          // Update Q by the Householder reflection.
          Update2(Q, c, s);
          isRotation = !isRotation;

          // Update D = Q^T*B*Q.
          saveB00 = b00;
          saveB01 = b01;
          saveB11 = b11;
          term0 = c * saveB00 + s * saveB01;
          term1 = c * saveB01 + s * saveB11;
          b00 = c * term0 + s * term1;
          term0 = s * saveB00 - c * saveB01;
          term1 = s * saveB01 - c * saveB11;
          b11 = s * term0 - c * term1;
          break;
        }
      }
    } else {
      float saveB11, saveB12, saveB22;
      for (iteration = 0; iteration < maxIteration; ++iteration)
      {
        // Compute the Givens reflection.
        GetCosSin(half * (b22 - b11), b12, c2, s2);
        s = std::sqrt(half * (one - c2));  // >= 1/sqrt(2)
        c = half * s2 / s;

        // Update Q by the Givens reflection.
        Update1(Q, c, s);
        isRotation = !isRotation;

        // Update B <- Q^T*B*Q, ensuring that b02 is zero and
        // |b12| has strictly decreased.  MODIFY...
        saveB11 = b11;
        saveB12 = b12;
        saveB22 = b22;
        term0 = c * saveB22 + s * saveB12;
        term1 = c * saveB12 + s * saveB11;
        b22 = c * term0 + s * term1;
        b11 = b00;
        term0 = c * saveB12 - s * saveB22;
        term1 = c * saveB11 - s * saveB12;
        b00 = c * term1 - s * term0;
        b12 = s * b01;
        b01 = c * b01;

        if (Converged(aggressive, b11, b22, b12)) {
          // Compute the Householder reflection.
          GetCosSin(half * (b11 - b22), b12, c2, s2);
          s = std::sqrt(half * (one - c2));
          c = half * s2 / s;  // >= 1/sqrt(2)

          // Update Q by the Householder reflection.
          Update3(Q, c, s);
          isRotation = !isRotation;

          // Update D = Q^T*B*Q.
          saveB11 = b11;
          saveB12 = b12;
          saveB22 = b22;
          term0 = c * saveB11 + s * saveB12;
          term1 = c * saveB12 + s * saveB22;
          b11 = c * term0 + s * term1;
          term0 = s * saveB11 - c * saveB12;
          term1 = s * saveB12 - c * saveB22;
          b22 = s * term0 - c * term1;
          break;
        }
      }
    }

    values = { b00, b11, b22 };
    for (size_t row = 0; row < 3; ++row) {
      for (size_t col = 0; col < 3; ++col) {
          vectors[row][col] = Q[col][row];
      }
    }

    SortEigenstuff()(sortType, isRotation, values, vectors);
    return iteration;
  }

private:
  // Update Q = Q*G in-place using G = {{c,0,-s},{s,0,c},{0,0,1}}.
  void Update0(float Q[3][3], float c, float s) const
  {
    for (int r = 0; r < 3; ++r) {
      float tmp0 = c * Q[r][0] + s * Q[r][1];
      float tmp1 = Q[r][2];
      float tmp2 = c * Q[r][1] - s * Q[r][0];
      Q[r][0] = tmp0;
      Q[r][1] = tmp1;
      Q[r][2] = tmp2;
    }
  }

    // Update Q = Q*G in-place using G = {{0,1,0},{c,0,s},{-s,0,c}}.
    void Update1(float Q[3][3], float c, float s) const
    {
      for (int r = 0; r < 3; ++r)
      {
        float tmp0 = c * Q[r][1] - s * Q[r][2];
        float tmp1 = Q[r][0];
        float tmp2 = c * Q[r][2] + s * Q[r][1];
        Q[r][0] = tmp0;
        Q[r][1] = tmp1;
        Q[r][2] = tmp2;
      }
    }

    // Update Q = Q*H in-place using H = {{c,s,0},{s,-c,0},{0,0,1}}.
    void Update2(float Q[3][3], float c, float s) const
    {
      for (int r = 0; r < 3; ++r)
      {
        float tmp0 = c * Q[r][0] + s * Q[r][1];
        float tmp1 = s * Q[r][0] - c * Q[r][1];
        Q[r][0] = tmp0;
        Q[r][1] = tmp1;
      }
    }

    // Update Q = Q*H in-place using H = {{1,0,0},{0,c,s},{0,s,-c}}.
    void Update3(float Q[3][3], float c, float s) const
    {
      for (int r = 0; r < 3; ++r)
      {
        float tmp0 = c * Q[r][1] + s * Q[r][2];
        float tmp1 = s * Q[r][1] - c * Q[r][2];
        Q[r][1] = tmp0;
        Q[r][2] = tmp1;
      }
    }

    // Normalize (u,v) robustly, avoiding floating-point overflow in the
    // sqrt call.  The normalized pair is (cs,sn) with cs <= 0.  If
    // (u,v) = (0,0), the function returns (cs,sn) = (-1,0).  When used
    // to generate a Householder reflection, it does not matter whether
    // (cs,sn) or (-cs,-sn) is used.  When generating a Givens reflection,
    // cs = cos(2*theta) and sn = sin(2*theta).  Having a negative cosine
    // for the float-angle term ensures that the single-angle terms
    // c = cos(theta) and s = sin(theta) satisfy |c| <= |s|.
    void GetCosSin(float u, float v, float& cs, float& sn) const
    {
      float maxAbsComp = std::max(std::fabs(u), std::fabs(v));
      if (maxAbsComp > (float)0) {
        u /= maxAbsComp;  // in [-1,1]
        v /= maxAbsComp;  // in [-1,1]
        float length = std::sqrt(u * u + v * v);
        cs = u / length;
        sn = v / length;
        if (cs > (float)0) {
          cs = -cs;
          sn = -sn;
        }
      } else {
        cs = (float)-1;
        sn = (float)0;
      }
    }

    // The convergence test.  When 'aggressive' is 'true', the
    // superdiagonal test is "bSuper == 0".  When 'aggressive' is 'false',
    // the superdiagonal test is
    //   |bDiag0| + |bDiag1| + |bSuper| == |bDiag0| + |bDiag1|
    // which means bSuper is effectively zero compared to the sizes of the
    // diagonal entries.
    bool Converged(bool aggressive, float bDiag0, float bDiag1, float bSuper) const
    {
      if (aggressive) {
        return bSuper == 0.0;
      } else {
        float sum = std::fabs(bDiag0) + std::fabs(bDiag1);
        return sum + std::fabs(bSuper) == sum;
      }
    }
};


class NISymmetricEigensolver3x3
{
public:
  // The input matrix must be symmetric, so only the unique elements
  // must be specified: a00, a01, a02, a11, a12, and a22.  The
  // eigenvalues are sorted in ascending order: eval0 <= eval1 <= eval2.

  void operator()(float a00, float a01, float a02, float a11, float a12, 
    float a22, int sortType, pxr::GfVec3f& values, 
    pxr::GfMatrix3f& vectors) const
  {
    // Precondition the matrix by factoring out the maximum absolute
    // value of the components.  This guards against floating-point
    // overflow when computing the eigenvalues.
    float max0 = std::max(std::fabs(a00), std::fabs(a01));
    float max1 = std::max(std::fabs(a02), std::fabs(a11));
    float max2 = std::max(std::fabs(a12), std::fabs(a22));
    float maxAbsElement = std::max(std::max(max0, max1), max2);
    if (maxAbsElement == (float)0)
    {
      // A is the zero matrix.
      values[0] = (float)0;
      values[1] = (float)0;
      values[2] = (float)0;
      vectors.SetRow(0, pxr::GfVec3f(1.f, 0.f, 0.f));
      vectors.SetRow(1, pxr::GfVec3f(0.f, 1.f, 0.f));
      vectors.SetRow(2, pxr::GfVec3f(0.f, 0.f, 1.f));
      return;
    }

    float invMaxAbsElement = (float)1 / maxAbsElement;
    a00 *= invMaxAbsElement;
    a01 *= invMaxAbsElement;
    a02 *= invMaxAbsElement;
    a11 *= invMaxAbsElement;
    a12 *= invMaxAbsElement;
    a22 *= invMaxAbsElement;

    float norm = a01 * a01 + a02 * a02 + a12 * a12;
    if (norm > (float)0)
    {
      // Compute the eigenvalues of A.

      // In the PDF mentioned previously, B = (A - q*I)/p, where
      // q = tr(A)/3 with tr(A) the trace of A (sum of the diagonal
      // entries of A) and where p = sqrt(tr((A - q*I)^2)/6).
      float q = (a00 + a11 + a22) / (float)3;

      // The matrix A - q*I is represented by the following, where
      // b00, b11 and b22 are computed after these comments,
      //   +-           -+
      //   | b00 a01 a02 |
      //   | a01 b11 a12 |
      //   | a02 a12 b22 |
      //   +-           -+
      float b00 = a00 - q;
      float b11 = a11 - q;
      float b22 = a22 - q;

      // The is the variable p mentioned in the PDF.
      float p = std::sqrt((b00 * b00 + b11 * b11 + b22 * b22 + norm * (float)2) / (float)6);

      // We need det(B) = det((A - q*I)/p) = det(A - q*I)/p^3.  The
      // value det(A - q*I) is computed using a cofactor expansion
      // by the first row of A - q*I.  The cofactors are c00, c01
      // and c02 and the determinant is b00*c00 - a01*c01 + a02*c02.
      // The det(B) is then computed finally by the division
      // with p^3.
      float c00 = b11 * b22 - a12 * a12;
      float c01 = a01 * b22 - a12 * a02;
      float c02 = a01 * a12 - b11 * a02;
      float det = (b00 * c00 - a01 * c01 + a02 * c02) / (p * p * p);

      // The halfDet value is cos(3*theta) mentioned in the PDF. The
      // acos(z) function requires |z| <= 1, but will fail silently
      // and return NaN if the input is larger than 1 in magnitude.
      // To avoid this problem due to rounding errors, the halfDet
      // value is clamped to [-1,1].
      float halfDet = det * (float)0.5;
      halfDet = std::min(std::max(halfDet, (float)-1), (float)1);

      // The eigenvalues of B are ordered as
      // beta0 <= beta1 <= beta2.  The number of digits in
      // twoThirdsPi is chosen so that, whether float or float,
      // the floating-point number is the closest to theoretical
      // 2*pi/3.
      float angle = std::acos(halfDet) / (float)3;
      float const twoThirdsPi = (float)2.09439510239319549;
      float beta2 = std::cos(angle) * (float)2;
      float beta0 = std::cos(angle + twoThirdsPi) * (float)2;
      float beta1 = -(beta0 + beta2);

      // The eigenvalues of A are ordered as
      // alpha0 <= alpha1 <= alpha2.
      values[0] = q + p * beta0;
      values[1] = q + p * beta1;
      values[2] = q + p * beta2;

      // Compute the eigenvectors so that the set
      // {vectors[0], vectors[1], vectors[2]} is right handed and
      // orthonormal.
      if (halfDet >= 0.0)
      {
        ComputeEigenvector0(a00, a01, a02, a11, a12, a22, values[2], (pxr::GfVec3f&)*(&vectors[2][0]));
        ComputeEigenvector1(a00, a01, a02, a11, a12, a22, vectors.GetRow(2), values[1], *(pxr::GfVec3f*)&vectors[1][0]);
        vectors.SetRow(0, vectors.GetRow(1) ^ vectors.GetRow(2));
      }
      else
      {
        ComputeEigenvector0(a00, a01, a02, a11, a12, a22, values[0], (pxr::GfVec3f&)*(&vectors[0][0]));
        ComputeEigenvector1(a00, a01, a02, a11, a12, a22, vectors.GetRow(0), values[1], *(pxr::GfVec3f*)&vectors[1][0]);
        vectors.SetRow(2, vectors.GetRow(0) ^ vectors.GetRow(1));
      }
    } else {
      // The matrix is diagonal.
      values[0] = a00;
      values[1] = a11;
      values[2] = a22;
      vectors.SetRow(0, pxr::GfVec3f(1.f, 0.f, 0.f));
      vectors.SetRow(1, pxr::GfVec3f(0.f, 1.f, 0.f));
      vectors.SetRow(2, pxr::GfVec3f(0.f, 0.f, 1.f));
    }

    // The preconditioning scaled the matrix A, which scales the
    // eigenvalues.  Revert the scaling.
    values[0] *= maxAbsElement;
    values[1] *= maxAbsElement;
    values[2] *= maxAbsElement;

    SortEigenstuff()(sortType, true, values, vectors);
  }

private:

  void ComputeOrthogonalComplement(pxr::GfVec3f const& W,
    pxr::GfVec3f& U, pxr::GfVec3f& V) const
  {
    // Robustly compute a right-handed orthonormal set { U, V, W }.
    // The vector W is guaranteed to be unit-length, in which case
    // there is no need to worry about a division by zero when
    // computing invLength.
    float invLength;
    if (std::fabs(W[0]) > std::fabs(W[1]))
    {
      // The component of maximum absolute value is either W[0]
      // or W[2].
      invLength = 1.0 / std::sqrt(W[0] * W[0] + W[2] * W[2]);
      U = { -W[2] * invLength, 0.0, +W[0] * invLength };
    }
    else
    {
      // The component of maximum absolute value is either W[1]
      // or W[2].
      invLength = 1.0 / std::sqrt(W[1] * W[1] + W[2] * W[2]);
      U = { 0.0, +W[2] * invLength, -W[1] * invLength };
    }
    V = W ^ U;
  }

  void ComputeEigenvector0(float a00, float a01, float a02, float a11, 
    float a12, float a22, float eval0, pxr::GfVec3f& evec0) const
  {
    // Compute a unit-length eigenvector for eigenvalue[i0].  The
    // matrix is rank 2, so two of the rows are linearly independent.
    // For a robust computation of the eigenvector, select the two
    // rows whose cross product has largest length of all pairs of
    // rows.
    pxr::GfVec3f row0(a00 - eval0, a01, a02);
    pxr::GfVec3f row1(a01, a11 - eval0, a12);
    pxr::GfVec3f row2(a02, a12, a22 - eval0);
    pxr::GfVec3f  r0xr1 = row0 ^ row1;
    pxr::GfVec3f  r0xr2 = row0 ^ row2;
    pxr::GfVec3f  r1xr2 = row1 ^ row2;
    float d0 = r0xr1 * r0xr1;
    float d1 = r0xr2 * r0xr2;
    float d2 = r1xr2 * r1xr2;

    float dmax = d0;
    int imax = 0;
    if (d1 > dmax) {
        dmax = d1;
        imax = 1;
    }
    if (d2 > dmax) {
        imax = 2;
    }

    if (imax == 0) {
        evec0 = r0xr1 / std::sqrt(d0);
    } else if (imax == 1) {
        evec0 = r0xr2 / std::sqrt(d1);
    } else {
        evec0 = r1xr2 / std::sqrt(d2);
    }
  }

  void ComputeEigenvector1(float a00, float a01, float a02, float a11, 
    float a12, float a22, pxr::GfVec3f const& evec0, float eval1, 
    pxr::GfVec3f& evec1) const
  {
    // Robustly compute a right-handed orthonormal set
    // { U, V, evec0 }.
    pxr::GfVec3f U, V;
    ComputeOrthogonalComplement(evec0, U, V);

    // Let e be eval1 and let E be a corresponding eigenvector which
    // is a solution to the linear system (A - e*I)*E = 0.  The matrix
    // (A - e*I) is 3x3, not invertible (so infinitely many
    // solutions), and has rank 2 when eval1 and eval are different.
    // It has rank 1 when eval1 and eval2 are equal.  Numerically, it
    // is difficult to compute robustly the rank of a matrix.  Instead,
    // the 3x3 linear system is reduced to a 2x2 system as follows.
    // Define the 3x2 matrix J = [U V] whose columns are the U and V
    // computed previously.  Define the 2x1 vector X = J*E.  The 2x2
    // system is 0 = M * X = (J^T * (A - e*I) * J) * X where J^T is
    // the transpose of J and M = J^T * (A - e*I) * J is a 2x2 matrix.
    // The system may be written as
    //     +-                        -++-  -+       +-  -+
    //     | U^T*A*U - e  U^T*A*V     || x0 | = e * | x0 |
    //     | V^T*A*U      V^T*A*V - e || x1 |       | x1 |
    //     +-                        -++   -+       +-  -+
    // where X has row entries x0 and x1.

    pxr::GfVec3f AU(
      a00 * U[0] + a01 * U[1] + a02 * U[2],
      a01 * U[0] + a11 * U[1] + a12 * U[2],
      a02 * U[0] + a12 * U[1] + a22 * U[2]
    );

    pxr::GfVec3f AV(
        a00 * V[0] + a01 * V[1] + a02 * V[2],
        a01 * V[0] + a11 * V[1] + a12 * V[2],
        a02 * V[0] + a12 * V[1] + a22 * V[2]
    );

    float m00 = U[0] * AU[0] + U[1] * AU[1] + U[2] * AU[2] - eval1;
    float m01 = U[0] * AV[0] + U[1] * AV[1] + U[2] * AV[2];
    float m11 = V[0] * AV[0] + V[1] * AV[1] + V[2] * AV[2] - eval1;

    // For robustness, choose the largest-length row of M to compute
    // the eigenvector.  The 2-tuple of coefficients of U and V in the
    // assignments to eigenvector[1] lies on a circle, and U and V are
    // unit length and perpendicular, so eigenvector[1] is unit length
    // (within numerical tolerance).
    float absM00 = std::fabs(m00);
    float absM01 = std::fabs(m01);
    float absM11 = std::fabs(m11);
    float maxAbsComp;
    if (absM00 >= absM11) {
      maxAbsComp = std::max(absM00, absM01);
      if (maxAbsComp > 0.0) {
        if (absM00 >= absM01) {
          m01 /= m00;
          m00 = 1.0 / std::sqrt(1.0 + m01 * m01);
          m01 *= m00;
        } else {
          m00 /= m01;
          m01 = 1.0 / std::sqrt(1.0 + m00 * m00);
          m00 *= m01;
        }
        evec1 = (m01 * U) - (m00 * V);
      } else {
        evec1 = U;
      }
    } else {
      maxAbsComp = std::max(absM11, absM01);
      if (maxAbsComp > 0.0) {
        if (absM11 >= absM01) {
            m01 /= m11;
            m11 = 1.0 / std::sqrt(1.0 + m01 * m01);
            m01 *= m11;
        } else {
            m11 /= m01;
            m01 = 1.0 / std::sqrt(1.0 + m11 * m11);
            m11 *= m01;
        }
        evec1 = (m11 * U) - (m01 * V);
      } else {
          evec1 = U;
      }
    }
  }
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif