#ifndef JVR_GEOMETRY_MATRIX_H
#define JVR_GEOMETRY_MATRIX_H

#include <vector>
#include <algorithm>
#include <numeric>
#include <iomanip>

#include <Eigen/Sparse>
 
#include <pxr/base/gf/math.h>
#include <pxr/base/vt/array.h>
#include "../common.h"
#include "../utils/timer.h"

JVR_NAMESPACE_OPEN_SCOPE

template <typename T>
struct SparseMatrixInfos 
{
  typedef typename std::pair<int, int> Key;
  VtArray<Key>  keys;
  VtArray<T>    values;

  SparseMatrixInfos(size_t size) {
    keys.resize(size);
    values.resize(size);
  };
  void AddEntry(size_t row, size_t col, T val) {
    keys.push_back(std::make_pair(row, col));
    values.push_back(val);
  };
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const SparseMatrixInfos<T> &infos) {
  for(size_t i = 0; i < infos.keys.size(); ++i)
    os << "((" << infos.keys[i].first << "," << infos.keys[i].second << "), " << 
      std::fixed << std::setprecision(4) << infos.values[i] << "),";
  return os;
};


template <typename T>
class Matrix {
public:
  static const size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

  static inline uint64_t feedT = 0;
  static inline uint64_t computeT = 0;
  static inline uint64_t solveT = 0;

  enum State {
    MATRIX_VALID,
    MATRIX_INVALID,
    MATRIX_SIZE_ZERO,
    MATRIX_SIZE_MISMATCH,
    MATRIX_INDEX_OUTOFBOUND,
    MATRIX_NON_SQUARE,
    MATRIX_IS_SINGULAR
  };

  typedef typename std::pair<int, int>  Key;
  typedef typename std::vector<T>       Vector;
  typedef typename std::vector<int>     Pivots;

  Matrix();
  Matrix(size_t row, size_t column);
  bool IsValid();
  void Resize(size_t row, size_t column);
  void Clear();

  size_t NumRows() const { return _rows; };
  size_t NumColumns()const { return _columns; };


  void Set(size_t row, size_t colum, T value);
  void Set(int n, Key *keys, T* values);
  void SetRow(size_t row, const Vector& values);
  void SetColumn(size_t column, const Vector& values);

  T Get(size_t row, size_t column) const;
  T* GetData();
  const T* GetData() const;
  T GetDeterminant(const Matrix<T>::Pivots& pivots) const;
  Matrix Add(const Matrix& other);
  void AddInPlace(const Matrix& other);
  Matrix Subtract(const Matrix& other);
  void SubtractInPlace(const Matrix& other);
  Matrix Multiply(const Matrix& other);
  void MultiplyInPlace(const Matrix& other);
  Matrix Scale(const Matrix<T>& other, float scale);
  void ScaleInPlace(const Matrix<T>& other, float scale);

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

  Vector MultiplyVector(const Vector& vector);
  void SwapRows(size_t a, size_t b);
  void SwapColumns(size_t a, size_t b);
  Matrix LUDecomposition();
  Vector SolveLU( const Vector &b );
  Matrix Transpose();
  void TransposeInPlace();
  Matrix Inverse();
  Matrix AsDiagonal();
  void InverseInPlace();

  static void ResetTs();

private:
  
  size_t    _rows;
  size_t    _columns;
  Vector    _matrix;
  Pivots    _pivots;
  bool      _singular;
  bool      _even;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Matrix<T> &m) {
  os << std::fixed << std::setprecision(5) << "matrix: (";
  size_t numVals = m.NumRows() * m.NumColumns();
  for (size_t r = 0; r < m.NumRows(); ++r)
    for (size_t c = 0; c < m.NumColumns(); ++c)
      os << m.GetData()[r * m.NumColumns() + c] << ((r * m.NumColumns() + c) < numVals - 1 ? "," : ")");

  return os;
}

template <typename T>
Matrix<T>::Matrix()
  : _rows(0), _columns(0), _even(true), _singular(false)
{
}

template <typename T>
Matrix<T>::Matrix(size_t rows, size_t columns)
  : _rows(rows), _columns(columns), _even(true), _singular(false)
{
  _matrix.resize(_rows * _columns);
  _pivots.resize(_columns);
  memset(&_matrix[0], 0, _matrix.size() * sizeof(T));
}

template <typename T>
bool Matrix<T>::IsValid()
{
  return !_singular && _rows > 0 && _columns > 0;
}

template <typename T>
void Matrix<T>::Resize(size_t rows, size_t columns)
{
  _rows = rows;
  _columns = columns;
  _matrix.resize(_rows * _columns);
  _pivots.resize(_columns);
}

template <typename T>
void Matrix<T>::Clear()
{
  std::fill(_matrix.begin(), _matrix.end(), 0);
}

template <typename T>
Matrix<T> Matrix<T>::Transpose() {
  Matrix<T> result(*this);
  for(size_t c = 0; c < _columns; ++c) 
    for(size_t r = 0; r < _rows; ++r)
      result._matrix[r * _columns + c] = _matrix[c * _rows + r];
  return result;
}

template <typename T>
void Matrix<T>::TransposeInPlace() {
  Matrix<T>::Vector tmp(this->_matrix);
  for(size_t c = 0; c < _columns; ++c) 
    for(size_t r = 0; r < _rows; ++r)
      tmp[r * _columns + c] = _matrix[c * _rows + r];
  _matrix = tmp;
  size_t rows = _rows;
  _rows = _columns;
  _columns = rows;
}

template <typename T>
typename Matrix<T>::Vector Matrix<T>::GetRow(size_t row) const
{
  Matrix<T>::Vector values;
  values.resize(_columns);
  for (size_t i = 0; i < _columns; ++i) {
    values[i] = Get(row, i);
  }
  return values;
}

template <typename T>
typename Matrix<T>::Vector Matrix<T>::GetColumn(size_t column) const
{
  Matrix<T>::Vector values;
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
T Matrix<T>::GetDeterminant(const Matrix<T>::Pivots& pivots) const
{
  if (_singular)return;
  size_t m = pivots.size();
  T determinant = 1.f;
  if (!_even)determinant = -1.f;

  for (size_t i = 0; i < m; ++i) {
    determinant *= _matrix[i * _columns + i];
  }
  return determinant;
}

template <typename T>
void Matrix<T>::Set(size_t row, size_t column, T value)
{
  uint64_t startT = CurrentTime();
  _matrix[row * _columns + column] = value;
  feedT += (CurrentTime() - startT);
}

template <typename T>
void Matrix<T>::Set(int n, Matrix<T>::Key *keys, T* values)
{
  uint64_t startT = CurrentTime();
  for(size_t i = 0; i < n; ++i)
    _matrix[keys[i].first * _columns + keys[i].second] = values[i];
  feedT += (CurrentTime() - startT);
}

template <typename T>
const T* Matrix<T>::GetData() const
{
  return &_matrix[0];
}

template <typename T>
T* Matrix<T>::GetData()
{
  return &_matrix[0];
}

template <typename T>
T Matrix<T>::Get(size_t row, size_t column) const
{
  return _matrix[row * _columns + column];
}

template <typename T>
void Matrix<T>::SetRow(size_t row, const Matrix<T>::Vector& values)
{
  for (size_t i = 0; i < _columns; ++i) {
    _matrix[row * _columns + i] = values[i];
  }
}

template <typename T>
void Matrix<T>::SetColumn(size_t column, const Matrix<T>::Vector& values)
{
  for (size_t i = 0; i < _columns; ++i) {
    _matrix[i * _columns + column] = values[i];
  }
}

template <typename T>
void Matrix<T>::AddInPlace(const Matrix<T>& other)
{
  // MATRIX_SIZE_MISMATCH
  if (_matrix.size() == other._matrix.size()) {
    for (size_t i = 0; i < _matrix.size(); ++i) {
      _matrix[i] += other._matrix[i];
    }
  }
}

template <typename T>
Matrix<T> Matrix<T>::Add(const Matrix<T>& other)
{
  // MATRIX_SIZE_MISMATCH
  if (_matrix.size() == other._matrix.size()) {
    Matrix<T> add(_rows, _columns);
    for (size_t i = 0; i < _matrix.size(); i++) {
      add._matrix[i] = _matrix[i] + other._matrix[i];
    }
    return add;
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
    Matrix<T> sub(_rows, _columns);
    for (size_t i = 0; i < _matrix.size(); i++) {
      sub._matrix[i] = _matrix[i] - other._matrix[i];
    }
    return sub;
  }
}


template <typename T>
Matrix<T> Matrix<T>::Multiply(const Matrix<T>& other)
{
  if (_columns == other._rows) {
    size_t i, j, k;
    Matrix<T> result(_rows, _columns);
    for (size_t i = 0; i < _rows; ++i)
      for (size_t k = 0; k < other._columns; ++k)
        for (size_t j = 0; j < _columns; ++j)
          result._matrix[i * _columns + j] += _matrix[i * _columns + k] * other._matrix[k * _columns + j];
    return result;
  }
  return Matrix();
}

template <typename T>
void Matrix<T>::MultiplyInPlace(const Matrix<T>& other)
{
  if (_columns == other._rows) {
    size_t i, j, k;
    Matrix<T>::Vector tmp(this->_matrix);
    Clear();
    for (size_t i = 0; i < _rows; ++i)
      for (size_t k = 0; k < other._columns; ++k)
        for (size_t j = 0; j < _columns; ++j)
          _matrix[i * _columns + j] += tmp[i * _columns + k] * other._matrix[k * _columns + j];
  }
}

template <typename T>
typename Matrix<T>::Vector Matrix<T>::MultiplyVector( const Matrix<T>::Vector &vector)
{
  Matrix<T>::Vector result(_columns);
  if (vector.size() == _columns) {
    for (size_t i = 0; i < _rows; ++i) {
      T product;
      for (size_t j = 0; j < _columns; ++j) {
        product += _matrix[i * _columns + j] * vector[j];
      }
      result[i] = product;
    }
  }
  return result;
}

template <typename T>
Matrix<T> Matrix<T>::Scale(const Matrix<T>& other, float scale)
{
  Matrix<T> result(_rows, _columns);
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
  T tmp = _pivots[a];
  _pivots[a] = _pivots[b];
  _pivots[b] = tmp;
  _even = 1 - _even;

  for (size_t i = 0; i < _columns; ++i) {
    tmp = _matrix[a * _columns + i];
    _matrix[a * _columns + i] = _matrix[b * _columns + i];
    _matrix[b * _columns + i] = tmp;
  }
}

template <typename T>
void Matrix<T>::SwapColumns(size_t a, size_t b)
{
  for (size_t i = 0; i < _rows; ++i) {
    T tmp = _matrix[_columns * i + a];
    _matrix[_columns * i + a] = _matrix[_columns * i + b];
    _matrix[_columns * i + b] = tmp;
  }
}

template <typename T>
Matrix<T> Matrix<T>::AsDiagonal()
{
  if(_rows == 1) {
    Matrix<T> diagonal(_columns, _columns);
    for(size_t c = 0; c < _columns; ++c) {
      diagonal.Set(c, c, this->_matrix[c]);
    }
    return diagonal;
  } else if(_columns == 1) {
    Matrix<T> diagonal(_rows, _rows);
    for(size_t c = 0; c < _columns; ++c) {
      diagonal.Set(c, c, this->_matrix[c]);
    }
    return diagonal;
  }
  return Matrix<T>();
}

template <typename T>
Matrix<T> Matrix<T>::Inverse()
{
  uint64_t startT = CurrentTime();
  Matrix<T> result(_columns, _columns);
  Matrix<T> lu = LUDecomposition();
  computeT += CurrentTime() - startT;

  startT = CurrentTime();
  if (lu.IsValid()) {
    Matrix<T>::Vector b(_columns, 0);
    for (size_t i = 0; i < _columns; ++i) {
      memset(&b[0], 0, _columns * sizeof(T));
      b[i] = 1;
      result.SetColumn(i, lu.SolveLU(b));
    }
  }
  solveT += CurrentTime() - startT;
  return result;
}

template <typename T>
void Matrix<T>::InverseInPlace()
{
  uint64_t startT = CurrentTime();
  Matrix<T> lu = LUDecomposition();
  computeT += CurrentTime() - startT;

  startT = CurrentTime();
  if (lu.IsValid()) {
    Matrix<T>::Vector b(_columns, 0);
    for (size_t i = 0; i < _columns; ++i) {
      memset(&b[0], 0, _columns * sizeof(T));
      b[i] = 1;
      SetColumn(i, lu.SolveLU(b));
    }
  }
  solveT += CurrentTime() - startT;
}

template <typename T>
Matrix<T> Matrix<T>::LUDecomposition()
{
  float singularityThreshold = 0.f;
  if (_rows != _columns) {
    return Matrix<T>();
  }

  Matrix<T> lu(*this);
  int m = _columns;

  // initialize permutation array And parity
  int row, column, i;
  for (size_t row = 0; row < m; ++row) lu._pivots[row] = row;

  _even = true;
  _singular = false;

  T sum;
  // loop over columns
  for (column = 0; column < m; ++column) {
    // upper
    for (row = 0; row < column; ++row) {
      sum = lu._matrix[row * _columns + column];
      for (size_t i = 0; i < row; ++i) {
        sum -= (lu._matrix[row * _columns + i] * lu._matrix[i * _columns + column]);
      }
      lu._matrix[row * _columns + column] = sum;
    }

    // lower
    size_t perm = column;//  permutation row
    T largest = std::numeric_limits<T>::max();
    for (row = column; row < m; ++row) {
      sum = lu._matrix[row * _columns + column];
      for (size_t i = 0; i < column; ++i) {
        sum -= (lu._matrix[row * _columns + i] * lu._matrix[i * _columns + column]);
      }
      lu._matrix[row * _columns + column] = sum;

      // maintain best permutation choice
      if (GfAbs(sum) > largest) {
        largest = GfAbs(sum);
        perm = row;
      }
    }

    // singularity check
    if (GfAbs(lu._matrix[perm * _columns + column]) < singularityThreshold) {
      _singular = true;
      std::cerr << "matrix is singular !" << std::endl;
      return lu;
    }

    // pivot if necessary
    if (perm != column) {
      lu.SwapRows(perm, column);
    }

    // divide the lower elements by the "winning" diagonal elt.
    T luDiag = lu._matrix[column * _columns + column];
    for (size_t row = column + 1; row < m; ++row) {
      lu._matrix[row * m + column] /= luDiag;
    }
  }
  return lu;
}

template <typename T>
typename Matrix<T>::Vector Matrix<T>::SolveLU( const Matrix<T>::Vector &vector)
{
  size_t m = _pivots.size();
  if (_columns != m) {
    std::cerr << "SolveLU : matrix size mismatch !" << std::endl;
    return Matrix<T>::Vector(vector.size());
  }
  if (_singular) {
    std::cerr << "SolveLU : matrix is singular !" << std::endl;
    return Matrix<T>::Vector(vector.size());
  }

  int row, column, i;
  Matrix<T>::Vector result(vector.size());
  // apply permutations to b
  for (row = 0; row < m; ++row)
    result[row] = vector[_pivots[row]];

  // solve LY = b
  T value;
  for (column = 0; column < m; ++column) {
    value = result[column];
    for (i = column + 1; i < m; ++i) {
      result[i] -= value * _matrix[i * _columns + column];
    }
  }

  // solve UX = Y
  for (column = m - 1; column >= 0; --column) {
    result[column] /= _matrix[column * _columns + column];
    value = result[column];
    for (i = 0; i < column; ++i) {
      result[i] -= value * _matrix[i * _columns + column];
    }
  }
  return result;
}

template<typename T>
void Matrix<T>::ResetTs()
{
  feedT = 0;
  computeT = 0;
  solveT = 0;
}


template <typename T>
class SparseMatrix {

public:
  typedef typename std::pair<int, int>                                  Key;
  typedef typename Eigen::Triplet<T>                                    Triplet;
  typedef typename Eigen::Vector<T, Eigen::Dynamic>                     Vector;
  //typedef typename Eigen::SimplicialCholesky< Eigen::SparseMatrix<T> >  CholeskySolver;
  typedef typename Eigen::SimplicialLDLT< Eigen::SparseMatrix<T> >      SimplicialSolver;
  //typedef typename Eigen::SparseLU< Eigen::SparseMatrix<T>, Eigen::COLAMDOrdering<int> >            LUSolver;
  //typedef typename Eigen::ConjugateGradient< Eigen::SparseMatrix<T>, Eigen::Lower|Eigen::Upper >   CGSolver;
  typedef SimplicialSolver Solver;

  static inline uint64_t feedT = 0;
  static inline uint64_t computeT = 0;
  static inline uint64_t solveT = 0;

  using Data = Eigen::SparseMatrix<T>;
  SparseMatrix();
  SparseMatrix(size_t row, size_t column);

  void Resize(size_t row, size_t column);

  inline size_t NumRows() const { return static_cast<size_t>(_matrix.rows()); };
  inline size_t NumColumns()const { return static_cast<size_t>(_matrix.cols());  };

  void Set(size_t row, size_t colum, T value);
  void Set(int n, Key *keys, T* values);
  
  void SetRow(size_t row,  const Vector &values);
  void SetColumn(size_t column, const Vector &values);

  inline T Get(size_t row, size_t column) const;
  inline const T* GetData() const;
  inline T* GetData();
  inline void GetDenseData(T* result, bool transposed=false) const;
  inline T GetDeterminant() const;
  inline SparseMatrix Add(const SparseMatrix& other);
  inline void AddInPlace(const SparseMatrix& other);
  inline SparseMatrix Subtract(const SparseMatrix& other);
  inline void SubtractInPlace(const SparseMatrix& other);
  inline SparseMatrix Multiply(const SparseMatrix& other);
  inline void MultiplyInPlace(const SparseMatrix& other);
  inline SparseMatrix Scale(float scale);
  inline void ScaleInPlace(float scale);
  inline Vector MultiplyVector(const Vector& vector);
  inline void Compute(Solver& solver);
  inline bool CheckSuccess(Solver& solver, const std::string &message);
  inline Vector Solve(Solver& solver, Vector& b);
  inline void Factorize(Solver& solver);
  inline SparseMatrix Transpose();
  inline void TransposeInPlace();
  inline SparseMatrix Inverse(Solver& solver);
  inline void InverseInPlace(Solver& solver);
  inline SparseMatrix AsDiagonal();

  inline void Compress();

  static void ResetTs();

private:
  Eigen::SparseMatrix<T> _matrix;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const SparseMatrix<T> &m) {
  os << std::fixed << std::setprecision(5) << "matrix: (";
  size_t numVals = m.NumRows() * m.NumColumns();
  std::vector<T> datas(numVals);
  m.GetDenseData(&datas[0], true);
  for (size_t v = 0; v < numVals; ++v)
    os << datas[v] << (v < numVals - 1 ? "," : ")");

  return os;
}

template <typename T>
SparseMatrix<T>::SparseMatrix()
{
}

template <typename T>
SparseMatrix<T>::SparseMatrix(size_t rows, size_t columns)
{
  _matrix = Eigen::SparseMatrix<T>(rows, columns);
}

template <typename T>
void SparseMatrix<T>::Resize(size_t rows, size_t columns)
{
  _matrix.resize(rows, columns);
}

template <typename T>
T SparseMatrix<T>::Get(size_t row, size_t column) const
{
  return _matrix.coeff(row, column);
}

template <typename T>
T* SparseMatrix<T>::GetData()
{
  return &_matrix.data()[0];
}

template <typename T>
const T* SparseMatrix<T>::GetData() const
{
  return _matrix.data().valuePtr();
}

template <typename T>
void SparseMatrix<T>::GetDenseData(T* result, bool transposed) const
{
  if(transposed) {
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> transposed(_matrix.transpose());
    memcpy(result, transposed.data(), _matrix.rows() * _matrix.cols() * sizeof(T));
  } else {
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> matrix(_matrix);
    memcpy(result, matrix.data(), 
      _matrix.rows() * _matrix.cols() * sizeof(T));
  }
}

template <typename T>
SparseMatrix<T> SparseMatrix<T>::Transpose() {
  SparseMatrix<T> result(*this);
  result._matrix = _matrix.transpose();
  return result;
}

template <typename T>
void SparseMatrix<T>::TransposeInPlace() {
  SparseMatrix<T> tmp(*this);
  tmp._matrix = _matrix.transpose();
  _matrix = tmp._matrix;
}

template <typename T>
void SparseMatrix<T>::Set(size_t row, size_t column, T value)
{
  uint64_t startT = CurrentTime();
  _matrix.coeffRef(row, column) = value;
  feedT += (CurrentTime() - startT);
}

template <typename T>
void SparseMatrix<T>::Set(int n, SparseMatrix<T>::Key *keys, T* values)
{   
  uint64_t startT = CurrentTime();
  std::vector<SparseMatrix<T>::Triplet> triplets(n);
  for (size_t i = 0; i < n; ++i) 
    triplets[i] = SparseMatrix<T>::Triplet( keys[i].first, keys[i].second, values[i]);
  
  _matrix.setFromTriplets(triplets.begin(), triplets.end());
  feedT += (CurrentTime() - startT);
}


template <typename T>
void SparseMatrix<T>::SetRow(size_t row, const SparseMatrix<T>::Vector& values)
{
  for (size_t i = 0; i < _matrix.columns(); ++i)
    _matrix.coeffRef(row, i) = values[i];
 
}

template <typename T>
void SparseMatrix<T>::SetColumn(size_t column, const SparseMatrix<T>::Vector& values)
{
  for (size_t i = 0; i < _matrix.rows(); ++i)
    _matrix.coeffRef(i, column) = values[i];
}


template <typename T>
void SparseMatrix<T>::AddInPlace(const SparseMatrix<T>& other)
{
  _matrix += other.matrix;
}

template <typename T>
SparseMatrix<T> SparseMatrix<T>::Add(const SparseMatrix<T>& other)
{
  SparseMatrix<T> result(*this);
  result._matrix += other._matrix;
  return result;
}

template <typename T>
void SparseMatrix<T>::SubtractInPlace(const SparseMatrix<T>& other)
{
  _matrix -= other._matrix;
}

template <typename T>
SparseMatrix<T> SparseMatrix<T>::Subtract(const SparseMatrix<T>& other)
{
  SparseMatrix<T> result(*this);
  result._matrix -= other._matrix;
  return result;
}

template <typename T>
SparseMatrix<T> SparseMatrix<T>::Multiply(const SparseMatrix<T>& other)
{
  SparseMatrix<T> result(*this);
  result._matrix = _matrix * other._matrix;
  return result;
}

template <typename T>
void SparseMatrix<T>::MultiplyInPlace(const SparseMatrix<T>& other)
{
  _matrix = _matrix * other._matrix;
}

template <typename T>
typename SparseMatrix<T>::Vector SparseMatrix<T>::MultiplyVector(
  const SparseMatrix<T>::Vector &vector)
{
  return _matrix * vector;
}

template <typename T>
SparseMatrix<T> SparseMatrix<T>::Scale(float scale)
{
  SparseMatrix<T> result(*this);
  result._matrix *= scale;
  return result;
}

template <typename T>
void SparseMatrix<T>::ScaleInPlace(float scale)
{
  _matrix *= scale;
}

template <typename T>
SparseMatrix<T> SparseMatrix<T>::Inverse(  SparseMatrix<T>::Solver &solver)
{
  uint64_t startT = CurrentTime();
  solver.analyzePattern(_matrix);
  solver.factorize(_matrix);
  Eigen::SparseMatrix<T> I(_matrix.rows(),_matrix.rows());
  I.setIdentity();
  computeT += (CurrentTime() - startT);

  startT = CurrentTime();
  SparseMatrix<T> result;
  result._matrix = solver.solve(I);
  solveT += (CurrentTime() - startT);

  return result;
}


template <typename T>
void SparseMatrix<T>::InverseInPlace(  typename SparseMatrix<T>::Solver &solver)
{
  uint64_t startT = CurrentTime();
  solver.compute(_matrix); 
  
  if(solver.info()!=Eigen::Success) {
     std::cout << "Fail factorize matrix :(" << std::endl;
   }

  Eigen::SparseMatrix<T> I(_matrix.rows(),_matrix.rows());
  I.setIdentity();
  computeT += (CurrentTime() - startT);

  startT = CurrentTime();
  _matrix = solver.solve(I);
  solveT += (CurrentTime() - startT);
}

template <typename T>
SparseMatrix<T> SparseMatrix<T>::AsDiagonal()
{
  return _matrix.diagonal();
}

template <typename T>
void SparseMatrix<T>::Compress()
{
  _matrix.makeCompressed();
}

template <typename T>
T SparseMatrix<T>::GetDeterminant() const
{
  return _matrix.determinant();
}

template <typename T>
void SparseMatrix<T>::Factorize(typename SparseMatrix<T>::Solver &solver)
{
  solver.compute(_matrix);
}

template <typename T>
typename SparseMatrix<T>::Vector SparseMatrix<T>::Solve(typename SparseMatrix<T>::Solver& solver, typename SparseMatrix<T>::Vector& b)
{
  return solver.solve(b);
}

template <typename T>
void SparseMatrix<T>::Compute(typename SparseMatrix<T>::Solver& solver)
{
  solver.analyzePattern(_matrix); 
  solver.factorize(_matrix); 

}

template <typename T>
bool SparseMatrix<T>::CheckSuccess(typename SparseMatrix<T>::Solver& solver, const std::string &message)
{
  if (solver.info() != Eigen::Success) {
    std::cerr << message<< std::endl;
    return false;
  }
  return true;
}



template<typename T>
void SparseMatrix<T>::ResetTs()
{
  feedT = 0;
  computeT = 0;
  solveT = 0;
}


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_MATRIX_H