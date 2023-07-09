#ifndef JVR_GEOMETRY_MATRIX_H
#define JVR_GEOMETRY_MATRIX_H

#include <vector>
#include <algorithm>
#include "../common.h"
#include <pxr/base/gf/math.h>

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

  Matrix();
  Matrix(size_t row, size_t column);
  bool IsValid();
  void Resize(size_t row, size_t column);
  void Clear();
  Matrix Transpose();
  void TransposeInPlace();

  size_t NumRows() const { return _rows; };
  size_t NumColumns()const { return _columns; };
  size_t GetIndex(size_t row, size_t colum) const;
  size_t GetColumnFromIndex(size_t idx) const;
  size_t GetRowFromIndex(size_t idx) const;

  void Set(size_t row, size_t colum, T value);
  void SetRow(size_t row, const Vector& values);
  void SetColumn(size_t column, const Vector& values);

  T Get(size_t row, size_t column) const;
  Vector GetRow(size_t row) const;
  Vector GetColumn(size_t column) const;
  T GetRowMinimum(size_t row) const;
  Vector GetRowsMinimum() const;
  T GetRowMaximum(size_t row) const;
  Vector GetRowsMaximum() const;
  T GetColumnMinimum(size_t column) const;
  Vector GetColumnsMinimum() const;
  T GetColumnMaximum(size_t column) const;
  Vector GetColumnsMaximum() const;
  T GetDeterminant(const Matrix<T>::Pivots& pivots) const;
  Matrix Add(const Matrix& other);
  void AddInPlace(const Matrix& other);
  Matrix Subtract(const Matrix& other);
  void SubtractInPlace(const Matrix& other);
  Matrix Multiply(const Matrix& other);
  void MultiplyInPlace(const Matrix& other);
  Matrix Scale(const Matrix<T>& other, float scale);
  void ScaleInPlace(const Matrix<T>& other, float scale);
  Vector MultiplyVector(const Vector& vector);
  void SwapRows(size_t a, size_t b);
  void SwapColumns(size_t a, size_t b);
  Matrix LUDecomposition(Pivots& pivots);
  int SolveLU(Pivots& pivots, Vector& b, Vector& x);
  Matrix Inverse();
  void InverseInPlace();

private:
  inline void _SetFlag(bool state, short flag) {
    state ? BITMASK_SET(_state, flag) : BITMASK_CLEAR(_state, flag);
  };
  inline void _SetTransposed(bool state) { 
    _SetFlag(state, TRANSPOSED);
  };
  inline bool _IsTransposed() const { return BITMASK_CHECK(_state, TRANSPOSED); };

  inline void _SetEven(bool state) {
    _SetFlag(state, EVEN);
  };
  inline bool _IsEven() const { return BITMASK_CHECK(_state, EVEN); }
  ;
  inline void _SetSingular(bool state) {
    _SetFlag(state, SINGULAR);
  };
  inline bool _IsSingular() const { return BITMASK_CHECK(_state, SINGULAR); };

  size_t _rows;
  size_t _columns;
  size_t _state;      // transposed, singular and even, encoded on three first bits
  Vector _matrix;
};

template <typename T>
Matrix<T>::Matrix()
  : _rows(0), _columns(0), _state(0)
{
}

template <typename T>
Matrix<T>::Matrix(size_t rows, size_t columns)
  : _rows(rows), _columns(columns), _state(0)
{
  _matrix.resize(_rows * _columns);
  memset(&_matrix[0], 0, _matrix.size() * sizeof(T));
}

template <typename T>
bool Matrix<T>::IsValid()
{
  return _rows > 0 && _columns > 0;
}

template <typename T>
void Matrix<T>::Resize(size_t rows, size_t columns)
{
  _rows = rows;
  _columns = columns;
  _matrix.resize(_rows * _columns);
}

template <typename T>
void Matrix<T>::Clear()
{
  std::fill(_matrix.begin(), _matrix.end(), 0);
}

template <typename T>
Matrix<T> Matrix<T>::Transpose() {
  Matrix<T> result(this);
  result._SetTransposed(1 - _IsTransposed());
  result._rows = _columns;
  result._columns = _rows;
  return result;
}

template <typename T>
void Matrix<T>::TransposeInPlace() {
  _SetTransposed(1 - _IsTransposed());
  size_t rows = _rows;
  _rows = _columns;
  _columns = rows;
}

template <typename T>
size_t Matrix<T>::GetIndex(size_t rows, size_t columns) const
{
  if (rows < _rows && columns < _columns) {
    if (_IsTransposed()) {
      return _rows * _columns + rows;
    }
    else {
      return _columns * rows + columns;
    }
  }
  return INVALID_INDEX;
}


template <typename T>
size_t Matrix<T>::GetColumnFromIndex(size_t idx) const
{
  if (idx < _matrix.size())return idx % _columns;
  return INVALID_INDEX;
}

template <typename T>
size_t Matrix<T>::GetRowFromIndex(size_t idx) const
{
  if (idx < _matrix.size())
    return(idx + 1 - GetColumnFromIndex(idx)) / _columns;
  return INVALID_INDEX;
}

template <typename T>
void Matrix<T>::Set(size_t row, size_t column, T value)
{
  _matrix[GetIndex(row, column)] = value;
}

template <typename T>
void Matrix<T>::SetRow(size_t row, const Vector& values)
{
  for (size_t i = 0; i < _columns; ++i) {
    _matrix[GetIndex(row, i)] = values[i];
  }
}

template <typename T>
void Matrix<T>::SetColumn(size_t column, const Vector& values)
{
  for (size_t i = 0; i < _columns; ++i) {
    _matrix[GetIndex(i, column)] = values[i];
  }
}

template <typename T>
T Matrix<T>::Get(size_t row, size_t column) const
{
  return _matrix[GetIndex(row, column)];
}

template <typename T>
std::vector<T> Matrix<T>::GetRow(size_t row) const
{
  std::vector<T> values;
  values.resize(_columns);
  for (size_t i = 0; i < _columns; ++i) {
    values[i] = Get(row, i);
  }
  return values;
}

template <typename T>
std::vector<T> Matrix<T>::GetColumn(size_t column) const
{
  std::vector<T> values;
  values.resize(_rows);
  for (size_t i = 0; i < _rows; ++i) {
    values[i] = Get(i, column);
  }
  return values;
}

template <typename T>
T Matrix<T>::GetRowMinimum(size_t row) const
{
  T minValue = std::numeric_limits<T>::max();

  for (size_t i = 0; i < _rows; ++i) {
    T value = Get(row, i);
    if (value < minValue)minValue = value;
  }
  return minValue;
}

template <typename T>
std::vector<T> Matrix<T>::GetRowsMinimum() const
{
  std::vector<T> results;
  results.resize(_rows);
  for (size_t i = 0; i < _rows; ++i) {
    results[i] = GetRowMinimum(i);
  }
  return results;
}

template <typename T>
T Matrix<T>::GetRowMaximum(size_t row) const
{
  T maxValue = std::numeric_limits<T>::min();
  for (size_t i = 0; i < _columns; ++i) {
    T value = Get(row, i);
    if (value > maxValue)maxValue = value;
  }
  return maxValue;
}

template <typename T>
std::vector<T> Matrix<T>::GetRowsMaximum() const
{
  std::vector<T> results;
  results.resize(_rows);
  for (size_t i = 0; i < _rows; ++i) {
    results[i] = GetRowMaximum(i);
  }
  return results;
}

template <typename T>
T Matrix<T>::GetColumnMinimum(size_t column) const
{
  T minValue = std::numeric_limits<T>::max();
  for (size_t i = 0; i < _rows; ++i) {
    T value = Get(i, column);
    if (value < minValue)minValue = value;
  }
  return minValue;
}

template <typename T>
std::vector<T> Matrix<T>::GetColumnsMinimum() const
{
  std::vector<T> results(_columns);
  for (size_t i = 0; i < _columns; ++i) {
    results[i] = GetColumnMinimum(i);
  }
  return results;
}

template <typename T>
T Matrix<T>::GetColumnMaximum(size_t column) const
{
  T maxValue = std::numeric_limits<T>::min();
  for (size_t i = 0; i < _rows; ++i) {
    T value = Get(i, column);
    if (value > maxValue)maxValue = value;
  }
  return maxValue;
}

template <typename T>
std::vector<T> Matrix<T>::GetColumnsMaximum() const
{
  Matrix<T>::Vector results(_columns);
  for (size_t i = 0; i < _columns; ++i) {
    results[i] = GetColumnMaximum(i);
  }
  return results;
}

template <typename T>
void Matrix<T>::AddInPlace(const Matrix<T>& other)
{
  // MATRIX_SIZE_MISMATCH
  if (_matrix.size() == other._matrix.size()) {
    for (size_t i = 0; i < _matrix.size(); i++) {
      _matrix[i] += other._matrix[i];
    }
  }
}

template <typename T>
Matrix<T> Matrix<T>::Add(const Matrix<T>& other)
{
  // MATRIX_SIZE_MISMATCH
  if (_matrix.size() == other._matrix.size()) {
    Matrix add(_rows, _columns);
    for (size_t i = 0; i < _matrix.size(); i++) {
      add[i] = _matrix[i] + other._matrix[i];
    }
  }
}

template <typename T>
void Matrix<T>::SubtractInPlace(const Matrix<T>& other)
{
  // MATRIX_SIZE_MISMATCH
  if (_matrix.size() == other._matrix.size()) {
    for (size_t i = 0; i < _matrix.size(); i++) {
      _matrix[i] -= other._matrix[i];
    }
  }
}

template <typename T>
Matrix<T> Matrix<T>::Subtract(const Matrix<T>& other)
{
  // MATRIX_SIZE_MISMATCH
  if (_matrix.size() == other._matrix.size()) {
    Matrix sub(_rows, _columns);
    for (size_t i = 0; i < _matrix.size(); i++) {
      sub[i] = _matrix[i] - other._matrix[i];
    }
  }
}


template <typename T>
Matrix<T> Matrix<T>::Multiply(const Matrix<T>& other)
{
  if (_columns == other._rows) {
    size_t i, j, k;
    Matrix result(_rows, _columns);
    for (size_t i = 0; i < _rows; ++i)
      for (size_t k = 0; k < other._columns; ++k)
        for (size_t j = 0; j < _columns; ++j)
          result[GetIndex(i, j)] += Get(i, k) * other.Get(k, j);
    return result;
  }
  return Matrix();
}

template <typename T>
void Matrix<T>::MultiplyInPlace(const Matrix<T>& other)
{
  if (_columns == other._rows) {
    size_t i, j, k;
    Matrix tmp(this);
    Clear();
    for (size_t i = 0; i < _rows; ++i)
      for (size_t k = 0; k < other._columns; ++k)
        for (size_t j = 0; j < _columns; ++j)
          _matrix[GetIndex(i, j)] += tmp.Get(i, k) * other.Get(k, j);
    return tmp;
  }
  return Matrix();
}

template <typename T>
std::vector<T> Matrix<T>::MultiplyVector(const Matrix<T>::Vector& vector)
{
  if (vector.size() == _columns) {
    std::vector<T> result(_columns);

    for (size_t i = 0; i < _rows; ++i) {
      T product;
      for (size_t j = 0; j < _columns; ++j) {
        product += Get(i, j) * vector[j];
      }
      result[i] = product;
    }
  }
  return std::vector<T>(_columns);
}

template <typename T>
Matrix<T> Matrix<T>::Scale(const Matrix<T>& other, float scale)
{
  Matrix result(_rows, _columns);
  for (size_t i = 0; i < _matrix.size(); ++i) {
    result._matrix[i] = _matrix[i] * scale;
  }
  return result;
}

template <typename T>
void Matrix<T>::ScaleInPlace(const Matrix<T>& other, float scale)
{
  for (size_t i = 0; i < _matrix.size(); ++i) {
    _matrix[i] *= scale;
  }
}

template <typename T>
void Matrix<T>::SwapRows(size_t a, size_t b)
{
  for (size_t i = 0; i < _columns; ++i) {
    auto tmp = _matrix.begin() + (a * _columns + i);
    _matrix.begin() + (a * _columns + i) = _matrix.begin() + (b * _columns + i);
    _matrix.begin() + (b * _columns + i) = tmp;
  }
}

template <typename T>
void Matrix<T>::SwapColumns(size_t a, size_t b)
{
  for (size_t i = 0; i < _rows; ++i) {
    auto tmp = _matrix.begin() + (_columns * i + a);
    _matrix.begin() + (_columns * i + a) = _matrix.begin() + (_columns * i + b);
    _matrix.begin() + (_columns * i + b) = tmp;
  }
}

template <typename T>
Matrix<T> Matrix<T>::Inverse()
{
  Matrix<T>::Pivots pivots(_columns);
  Matrix<T> result(_columns, _columns);
  if (!LUDecomposition(pivots)) {
    for (size_t i = 0; i < _columns; ++i) {
      Matrix<T>::Vector b(_columns, 0);
      Matrix<T>::Vector w(_columns, 0);
      b[i] = 1;

      SolveLU(pivots, b, w);
      SetColumn(i, w);
    }
  }
  return result;
}

template <typename T>
void Matrix<T>::InverseInPlace()
{
  Matrix<T>::Pivots pivots(_columns);
  Matrix<T> tmp(_columns, _columns);
  Matrix<T> lu = LUDecomposition(pivots);
  if (lu.IsValid()) {
    for (size_t i = 0; i < _columns; ++i) {
      Matrix<T>::Vector b(_columns, 0);
      Matrix<T>::Vector w(_columns, 0);
      b[i] = 1;

      SolveLU(pivots, b, w);
      tmp.SetColumn(i, w);
    }
  }
  for (size_t i = 0; i < tmp._matrix.size(); ++i) {
    _matrix[i] = tmp._matrix[i];
  }
}


template <typename T>
T Matrix<T>::GetDeterminant(const Matrix<T>::Pivots& pivots) const
{
  if (_IsSingular())return;
  size_t m = pivots.size();
  T determinant = 1.f;
  if (!_IsEven())determinant = -1.f;

  for (size_t i = 0; i < m; ++i) {
    determinant *= Get(i, i);
  }
  return determinant;
}

template <typename T>
Matrix<T> Matrix<T>::LUDecomposition(Matrix<T>::Pivots& pivots)
{
  float singularityThreshold = 0.f;
  if (_rows != _columns) {
    return Matrix<T>();
  }

  Matrix<T> lu(*this);
  size_t m = _columns;

  // initialize permutation array And parity
  size_t row, column, i;
  for (size_t row = 0; row < m; ++row) pivots[row] = row;

  _SetEven(true);
  _SetSingular(false);

  // loop over columns
  for (column = 0; column < m; ++column) {
    // upper
    for (row = 0; row < column; ++row) {
      Matrix<T>::Vector luRow = GetRow(row);
      T sum = luRow[column];
      for (size_t i = 0; i < row; ++i) {
        sum -= (luRow[i] * lu.Get(i, column));
      }
      lu.Set(row, column, sum);
    }

    // lower
    size_t perm = column;//  permutation row
    T largest = std::numeric_limits<T>::max();
    for (row = column; row < m; ++row) {
      Matrix<T>::Vector luRow = GetRow(row);
      T sum = luRow[column];
      for (size_t i = 0; i < column; ++i) {
        sum -= (luRow[i] * lu.Get(i, column));
      }
      lu.Set(row, column, sum);

      // maintain best permutation choice
      if (pxr::GfAbs(sum) > largest) {
        largest = pxr::GfAbs(sum);
        perm = row;
      }
    }

    // singularity check
    if (pxr::GfAbs(lu.Get(perm, column)) < singularityThreshold) {
      lu._SetSingular(true);
      return lu;
    }

    // pivot if necessary
    if (perm != column) {
      lu.SwapRows(perm, column);
      auto tmp = pivots.begin() + perm;
      pivots.begin() + perm = pivots.begin() + column;
      pivots.begin() + column = tmp;
      lu._SetEven(1 - lu._IsEven());
    }

    // divide the lower elements by the "winning" diagonal elt.
    T luDiag = lu.Get(column, column);
    for (size_t row = column + 1; row < m; ++row) {
      lu._matrix[row * m + column] /= luDiag;
    }
  }
  return lu;
}

template <typename T>
int Matrix<T>::SolveLU(Matrix<T>::Pivots& pivots, Matrix<T>::Vector& b, Matrix<T>::Vector& x)
{
  size_t m = pivots.size();
  if (_columns != m)return MATRIX_SIZE_MISMATCH;
  if (_IsSingular())return MATRIX_IS_SINGULAR;

  size_t row, column, i;
  // apply permutations to b
  for (row = 0; row < m; ++row) {
    x[row] = b[pivots[row]];
  }

  // solve LY = b
  for (column = 0; column < m; ++column) {
    T value = x[column];
    for (i = column + 1; i < m; ++i) {
      x[i] -= value * Get(i, column);
    }
  }

  // solve UX = Y
  for (column = m - 1; column >= 0; --column) {
    x[column] = x[column] / Get(column, column);
    T value = x[column];
    for (i = 0; i < column; ++i) {
      x[i] -= value * Get(i, column);
    }
  }
  return MATRIX_VALID;
}


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_MATRIX_H