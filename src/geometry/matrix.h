#ifndef JVR_GEOMETRY_MATRIX_H
#define JVR_GEOMETRY_MATRIX_H

#include <vector>
#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

template <typename T>
class Matrix {
#define INVALID_INDEX std::numeric_limits<size_t>::max()

public:
  enum Flags {
    TRANSPOSED = 1,
    SINGULAR = 2,
    EVEN = 4
  };
  enum State {
    MATRIX_VALID,
    MATRIX_INVALID,
    MATRIX_SIZE_ZERO,
    MATRIX_SIZE_MISMATCH,
    MATRIX_INDEX_OUTOFBOUND,
    MATRIX_NON_SQUARE,
    MATRIX_IS_SINGULAR
  };

  using Vector = std::vector<T>;
  using Pivots = std::vector<int>;

  Matrix(size_t row, size_t column);
  void Resize(size_t row, size_t column);
  void Transpose();
  Matrix GetTransposed();

  size_t GetIndex(size_t row, size_t colum);
  size_t GetColumnFromIndex(size_t idx);
  size_t GetRowFromIndex(size_t idx);

  void Set(size_t row, size_t colum, T value);
  void SetRow(size_t row, const Vector& values);
  void SetColumn(size_t column, const Vector& values);

  T Get(size_t row, size_t column);
  Vector GetRow(size_t row);
  Vector GetColumn(size_t column);
  T GetRowMinimum(size_t row);
  Vector GetRowsMinimum();
  T GetRowMaximum(size_t row);
  Vector GetRowsMaximum();
  T GetColumnMinimum(size_t column);
  Vector GetColumnsMinimum();
  T GetColumnMaximum(size_t column);
  Vector GetColumnsMaximum();
  Matrix Add(const Matrix& other);
  void AddInPlace(const Matrix& other);
  Matrix Subtract(const Matrix& other);
  void SubtractInPlace(const Matrix& other);
  Matrix Multiply(const Matrix& other);
  void MultiplyInPlace(const Matrix& other);
  Vector MultiplyVector(const Vector& vector);
  void SwapRows(size_t a, size_t b);
  void SwapColumns(size_t a, size_t b);
  bool LUDecomposition(Pivots& pivots);
  void SolveLU(Pivots& pivots, Vector& b, Vector& x);
  Matrix Inverse();
  void InverseInPlace();

private:
  inline void _SetFlag(bool state, short flag) {
    state ? BITMASK_SET(_state, flag) : BITMASK_CLEAR(_state, flag);
  };
  inline void _SetTransposed(bool state) { 
    _SetFlag(state, TRANSPOSED);
  };
  inline bool _IsTransposed() { return BITMASK_CHECK(_state, TRANSPOSED); };

  inline void _SetEven(bool state) {
    _SetFlag(state, EVEN);
  };
  inline bool _IsEven() { return BITMASK_CHECK(_state, EVEN); }
  ;
  inline void _SetSingular(bool state) {
    _SetFlag(state, SINGULAR);
  };
  inline bool _IsSingular() { return BITMASK_CHECK(_state, SINGULAR); };

  size_t _rows;
  size_t _columns;
  size_t _state; /* transposed, singular, even encoded i three first bits*/
  Vector _matrix;
  Matrix* _lu;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_MATRIX_H