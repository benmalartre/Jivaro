#include "../geometry/matrix.h"

JVR_NAMESPACE_OPEN_SCOPE

template <typename T>
Matrix<T>::Matrix<T>(size_t rows, size_t columns)
  : _rows(rows), _columns(columns), _state(0)
{
  _matrix.resize(_rows * _columns);
  memset(&_matrix[0], 0, _matrix.size() * sizeof(T));
}

template <typename T>
void Matrix<T>::Resize(size_t rows, size_t columns)
{
  _rows = rows;
  _column = columns;
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
  result._rows = _colums;
  result._columns = rows;
  return result;
}

template <typename T>
void Matrix<T>::TransposeInPlace() {
  _SetTransposed(1 - _IsTransposed());
  size_t rows = _rows;
  _rows = _colums;
  _columns = rows;
}

template <typename T>
size_t Matrix<T>::GetIndex(size_t rows, size_t columns)
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
size_t Matrix<T>::GetColumnFromIndex(size_t idx)
{
  if (idx < _matrix.size())return idx % _columns;
  return INVALID_INDEX;
}

template <typename T>
size_t Matrix<T>::GetRowFromIndex(size_t idx)
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
T Matrix<T>::Get(size_t row, size_t column)
{
  return _matrix[GetIndex(row, column)];
}

template <typename T>
std::vector<T> Matrix<T>::GetRow(size_t row)
{
  std::vector<T> values;
  values.resize(_columns);
  for (size_t i = 0; i < _columns; ++i) {
    values[i] = Get(row, i);
  }
  return values;
}

template <typename T>
std::vector<T> Matrix<T>::GetColumn(size_t column)
{
  std::vector<T> values;
  values.resize(_rows);
  for (size_t i = 0; i < _rows; ++i) {
    values[i] = Get(i, column);
  }
  return values;
}

template <typename T>
T Matrix<T>::GetRowMinimum(size_t row)
{
  T minValue = std::numeric_limits<T>::max();

  for (size_t i = 0; i < _rows; ++i) {
    T value = Get(row, i);
    if (value < minValue)minValue = value;
  }
  return minValue;
}

template <typename T>
std::vector<T> Matrix<T>::GetRowsMinimum()
{
  std::vector<T> results;
  results.resize(_rows);
  for (size_t i = 0; i < _rows; ++i) {
    results[i] = GetRowMinimum(i)
  }
  return results;
}

template <typename T>
T Matrix<T>::GetRowMaximum(size_t row)
{
  T maxValue = std::numeric_limits<T>::min();
  for (size_t column = 0; column < _columns; ++column) {
    T value = Get(row, column);
    if (value > maxValue)maxValue = value;
  }
  return maxValue;
}

template <typename T>
std::vector<T> Matrix<T>::GetRowsMaximum()
{
  std::vector<T> results;
  results.resize(_rows);
  for (size_t i = 0; i < _rows; ++i) {
    results[i] = GetRowMaximum(i)
  }
  return results;
}

template <typename T>
T Matrix<T>::GetColumnMinimum(size_t column)
{
  T minValue = std::numeric_limits<T>::max();
  for (size_t i = 0; i < _columns; ++i) {
    T value = Get(row, i);
    if (value < minValue)minValue = value;
  }
  return minValue;
}

template <typename T>
std::vector<T> Matrix<T>::GetColumnsMinimum()
{
  std::vector<T> results(_columns);
  for (size_t i = 0; i < _columns; ++i) {
    results[i] = GetColumnMinimum(i);
  }
  return results;
}

template <typename T>
T Matrix<T>::GetColumnMaximum(size_t column)
{
  T maxValue = std::numeric_limits<T>::min();
  for (size_t i = 0; i < _columns; ++i) {
    T value = Get(row, i);
    if (value > maxValue)maxValue = value;
  }
  return maxValue;
}

template <typename T>
std::vector<T> Matrix<T>::GetColumnsMaximum()
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
  if (_columns == _other._rows) {
    size_t i, j, k;
    Matrix result(_rows, _columns);
    for(size_t i = 0; i < _rows; ++i)
      for(size_t k = 0; k < other._columns; ++k)
        for(size_t j = 0; j < _columns; ++j)
          result[GetIndex(i, j)] += Get(i, k) * other.Get(k, j);
    return result;
  }
  return Matrix();
}

template <typename T>
void Matrix<T>::MultiplyInPlace(const Matrix<T>& other)
{
  if (_columns == _other._rows) {
    size_t i, j, k;
    Matrix tmp(this);
    Clear();
    for (size_t i = 0; i < _rows; ++i)
      for (size_t k = 0; k < other._columns; ++k)
        for (size_t j = 0; j < _columns; ++j)
          _matrix[GetIndex(i, j)] += tmp.Get(i, k) * other.Get(k, j);
          return result;
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
    std::swap(
      _matrix.begin() + (a * _columns + i), 
      _matrix.begin() + (b * _columns + i)
    );
  }
}

template <typename T>
void Matrix<T>::SwapColumns(size_t a, size_t b)
{
  for (size_t i = 0; i < _rows; ++i) {
    std::swap(
      _matrix.begin() + (_columns * i + a),
      _matrix.begin() + (_columns * i + b)
    );
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
  if (!LUDecomposition(pivots)) {
    for (size_t i = 0; i < _columns; ++i) {
      Matrix<T>::Vector b(_columns, 0);
      Matrix<T>::Vector w(_columns, 0);
      b[i] = 1;

      SolveLU(pivots, b, w);
      tmp.SetColumn(i, w);
    }
  }
  for (size_t i = 0; i < tmp._matrix.size(); ++i) {
    _matrix[i] = tmp[i];
  }
}


template <typename T>
T Matrix<T>::GetDeterminant(const Matrix<T>::Pivots& pivots)
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
  if (_rows <> _columns) {
    return Matrix<T>();
  }

  Matrix<T> lu(this);
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
    if (perm <> column) {
      lu.SwapRows(perm, column);
      std::swap(pivots.begin() + perm, pivots.begin() + column);
      lu._SetEven(1 - lu._IsEven());
    }
  
    // divide the lower elements by the "winning" diagonal elt.
    T luDiag.f = lu.Get(column, column);
    for (size_t row = column + 1; row < m; ++row) {
      lu._matrix[row * m + column] /= luDiag;
    }
  }
}

template <typename T>
int Matrix<T>::SolveLU(Matrix<T>::Pivots & pivots, Matrix<T>::Vector & b, Matrix<T>::Vector & x)
{
  size_t m = pivots.size();
  if (_columns <> m)return MATRIX_SIZE_MISMATCH;
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