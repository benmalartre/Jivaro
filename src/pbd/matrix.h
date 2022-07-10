#ifndef JVR_PBD_MATRIX_H
#define JVR_PBD_MATRIX_H

#include "../common.h"
#include "../pbd/maths.h"
#include <limits>
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE

template<typename T>
class PBDMatrix {
public:
    enum State {
        MATRIX_VALID,
        MATRIX_INVALID,
        MATRIX_SIZE_ZERO,
        MATRIX_SIZE_MISMATCH,
        MATRIX_INDEX_OUTOFBOUND,
        MATRIX_NON_SQUARE,
        MATRIX_IS_SINGULAR
    };
    PBDMatrix();
    PBDMatrix(int rows, int columns);
    PBDMatrix(const PBDMatrix& other);
    ~PBDMatrix();

    inline void Resize(int rows, int columns);
    inline PBDMatrix<T>& Transpose();
    inline int GetIndex(int row, int column);
    inline int Size(){return _rows * _columns;};
    inline void SetValue(int row, int column, T value);
    inline void SetRow(int row, T* values);
    inline void SetColumn(int column, T* values);
    inline T GetValue(int row, int column);
    inline void GetRow(int row, std::vector<T>& values);
    inline void GetColumn(int column, std::vector<T>& values);
    inline T GetRowMinimum(int row);
    inline void GetRowsMinimum(std::vector<T>& values);
    inline T GetRowMaximum(int row);
    inline void GetRowsMaximum(std::vector<T>& values);
    inline T GetColumnMinimum(int column);
    inline void GetColumnsMinimum(std::vector<T>& values);
    inline T GetColumnMaximum(int column);
    inline void GetColumnsMaximum(std::vector<T>& values);
    inline void SwapRows(int a, int b);
    inline void SwapColumns(int a, int b);

    inline PBDMatrix operator+(PBDMatrix& other);
    inline PBDMatrix& operator+=(PBDMatrix& other);
    inline PBDMatrix operator-(PBDMatrix& other);
    inline PBDMatrix& operator-=(PBDMatrix& other);
    inline PBDMatrix operator*(PBDMatrix& other);
    inline PBDMatrix& operator*=(PBDMatrix& other);
    inline T* operator*(T* vector);
    inline PBDMatrix& operator*=(const T factor);
    inline PBDMatrix operator*(const T factor);

    inline short LUDecomposition(const size_t m, int* pivot); 
    inline short LUDecomposition2(T tol, int* pivot);
    inline short SolveLU(const size_t m, int* pivot, T* b, T* x);
    inline T GetDeterminant(const size_t m);
    inline void Solve(T* vector, T* result);
 
    inline void Inverse(PBDMatrix& result);
    inline void InverseInPlace();

    void Echo(const std::string& msg);
    T* Data() {return _data;};
    const T* CData() const {return _data;};

private:
    int                         _rows;
    int                         _columns;
    T*                          _data;
    PBDMatrix<T>*               _lu;
    bool                        _singular;
    bool                        _even;
};


template<typename T>
PBDMatrix<T>::PBDMatrix() 
  : _lu(NULL)
  , _data(NULL)
  , _rows(0)
  , _columns(0)
  , _even(true)
  , _singular(false)
{
}

template<typename T>
PBDMatrix<T>::PBDMatrix(int rows, int columns) 
  : _lu(NULL)
  , _data(NULL)
  , _rows(rows)
  , _columns(columns)
  , _even(true)
  , _singular(false)
{
  size_t nCells = _rows * _columns;
  if(nCells) {
    _data = new T[_rows * _columns];
    memset(&_data[0], 0, _rows * _columns * sizeof(T));
  }
}

template<typename T>
PBDMatrix<T>::PBDMatrix(const PBDMatrix& other) 
  : _lu(NULL)
  , _data(NULL)
  , _rows(other._rows)
  , _columns(other._columns)
  , _even(other._even)
  , _singular(other._singular)
{
  size_t nCells = _rows * _columns;
  if(nCells) {
    _data = new T[_rows * _columns];
   memcpy(_data, other._data, _rows * _columns * sizeof(T));
  }
}

template<typename T>
PBDMatrix<T>::~PBDMatrix()
{
  if(_lu) delete _lu;
  if(_data) delete [] _data;
}

template<typename T>
inline void PBDMatrix<T>::Resize(int rows, int columns)
{
  if(_data)delete [] _data;
  _rows = rows;
  _columns = columns;
  _data = new T[_rows * _columns];
  memset(_data, 0, _rows * _columns * sizeof(T));
}

template<typename T>
inline PBDMatrix<T>& PBDMatrix<T>::Transpose()
{
  size_t tmp = _rows;
  _rows = _columns;
  _columns = tmp;
  T v;
  for(size_t row = 0; row < _rows; ++row) {
    for(size_t column = 0; column <= row; ++column) {
      v = _data[GetIndex(row, column)];
      _data[GetIndex(row, column)] = _data[GetIndex(column, row)];
      _data[GetIndex(column, row)] = v;
    }
  }
  return *this;
}

template<typename T>
inline int PBDMatrix<T>::GetIndex(int row, int column)
{   
  return _columns * row + column;
}

template<typename T>
inline void PBDMatrix<T>::SetValue(int row, int column, T value)
{
  _data[GetIndex(row, column)] = value;
}

template<typename T>
inline void PBDMatrix<T>::SetRow(int row, T* values)
{
  for(int column = 0; column < _columns; ++column) {
    _data[GetIndex(row, column)] = values[column];
  }
}

template<typename T>
inline void PBDMatrix<T>::SetColumn(int column, T* values)
{
  for(int row = 0; row < _rows; ++row) {
    SetValue(row, column, values[row]);
  }
}

template<typename T>
inline T PBDMatrix<T>::GetValue(int row, int column)
{
  return _data[GetIndex(row, column)];
}

template<typename T>
inline void PBDMatrix<T>::GetRow(int row, std::vector<T>& values)
{
  for(size_t column = 0; column < _columns; ++column) {
    values[column] = GetValue(row, column);
  }
}

template<typename T>
inline void PBDMatrix<T>::GetColumn(int column, std::vector<T>& values)
{
  for(size_t row = 0; row < _rows; ++row) {
    values[row] = GetValue(row, column);
  }
}

template<typename T>
inline T PBDMatrix<T>::GetRowMinimum(int row)
{
  T value = std::numeric_limits<T>::max();
  for(size_t column = 0; column < _columns; ++column) {
    T curValue;
    GetValue(row, column, curValue);
    if(curValue < value) {
    value = curValue;
    }
  }
  return value;
}

template<typename T>
inline void PBDMatrix<T>::GetRowsMinimum(std::vector<T>& values)
{
  values.resize(_rows);
  for(size_t row = 0; row < _rows; ++row) {
    values[row] = GetRowMinimum(row);
  }
}

template<typename T>
inline T PBDMatrix<T>::GetRowMaximum(int row)
{
  T value = std::numeric_limits<T>::min();
  for(size_t column = 0; column < _columns; ++column) {
    T curValue;
    GetValue(row, column, curValue);
    if(curValue > value) {
    value = curValue;
    }
  }
  return value;
}

template<typename T>
inline void PBDMatrix<T>::GetRowsMaximum(std::vector<T>& values)
{
  values.resize(_rows);
  for(size_t row = 0; row < _rows; ++row) {
    values[row] = GetRowMaximum(row);
  }
}

template<typename T>
inline T PBDMatrix<T>::GetColumnMinimum(int column)
{
  T value = std::numeric_limits<T>::max();
  for(size_t row = 0; row < _rows; ++row) {
    T curValue;
    GetValue(row, column, curValue);
    if(curValue < value) {
      value = curValue;
    }
  }
  return value;
}

template<typename T>
inline void PBDMatrix<T>::GetColumnsMinimum(std::vector<T>& values)
{
  values.resize(_columns);
  for(size_t column = 0; column < _columns; ++column) {
    values[column] = GetColumnMinimum(column);
  }
}

template<typename T>
inline T PBDMatrix<T>::GetColumnMaximum(int column)
{
  T value = std::numeric_limits<T>::min();
  for(size_t row = 0; row < _rows; ++row) {
    T curValue;
    GetValue(row, column, curValue);
    if(curValue > value) {
      value = curValue;
    }
  } 
  return value;
}

template<typename T>
inline void PBDMatrix<T>::GetColumnsMaximum(std::vector<T>& values)
{
  values.resize(_columns);
  for(size_t column = 0; column < _columns; ++column) {
    values[column] = GetColumnMaximum(column);
  }
}

template<typename T>
inline void PBDMatrix<T>::SwapRows(int a, int b)
{
  T tmp;
  for(size_t column = 0; column < _columns; ++column) {
    std::swap(_data[a * _columns + column], _data[b * _columns + column]);
  }
}

template<typename T>
inline void PBDMatrix<T>::SwapColumns(int a, int b)
{
  T tmp;
  for(size_t row = 0; row < _rows; ++row) {
    std::swap(_data[_columns * row + a], _data[_columns + row + b]);
  }
}

template<typename T>
inline PBDMatrix<T> PBDMatrix<T>::operator+(PBDMatrix<T>& other)
{
  if(Size() != other.Size()) {
    return *this;
  }
  PBDMatrix result(_rows, _columns);
  for(size_t i = 0; i < Size(); ++i) {
    result[i] = _data[i] + other._data[i];
  }
  return result;
}

template<typename T>
inline PBDMatrix<T>& PBDMatrix<T>::operator+=(PBDMatrix<T>& other)
{
  if(Size() != other.Size()) {
    return *this;
  }
  for(size_t i = 0; i < Size(); ++i) {
    _data[i] += other._data[i];
  }
  return *this;
}

template<typename T>
inline PBDMatrix<T> PBDMatrix<T>::operator-(PBDMatrix<T>& other)
{
  if(Size() != other.Size()) {
    return *this;
  }
  PBDMatrix result(_rows, _columns);
  for(size_t i = 0; i < Size(); ++i) {
    result[i] = _data[i] - other._data[i];
  }
  return result;
}

template<typename T>
inline PBDMatrix<T>& PBDMatrix<T>::operator-=(PBDMatrix<T>& other)
{
  if(Size() != other.Size()) {
    return *this;
  }
  for(size_t i = 0; i < Size(); ++i) {
    _data[i] -= other._data[i];
  }
  return *this;
}

template<typename T>
inline PBDMatrix<T> PBDMatrix<T>::operator*(PBDMatrix<T>& other)
{
  PBDMatrix<T> result(_rows, _columns);
  for(size_t i = 0; i < _rows; ++i) {
    for(size_t k = 0; k < other._columns; ++k) {
      for(size_t j = 0; j < _columns; ++j) {
        size_t idx = GetIndex(i, j);
        result._data[idx] += GetValue(i, k) * other.GetValue(k, j);
      }
    }
  }
  return result;
}

template<typename T>
inline PBDMatrix<T>& PBDMatrix<T>::operator*=(PBDMatrix<T>& other)
{
  PBDMatrix<T> copy(*this);
  memset(_data, 0, _rows * _columns * sizeof(T));

  for(size_t i = 0; i < _rows; ++i) {
    for(size_t k = 0; k < other._columns; ++k) {
      for(size_t j = 0; j < _columns; ++j) {
        size_t idx = GetIndex(i, j);
        _data[idx] += copy.GetValue(i, k) * other.GetValue(k, j);
      }
    }
  }
}

template<typename T>
inline T* PBDMatrix<T>::operator*(T* vector)
{
  T product;
  size_t row, column;
  T buffer[_rows];
  memcpy(&buffer[0], vector, _rows * sizeof(T));
  for( row = 0; row < _rows; ++row) {
    product = static_cast<T>(0);
    for(column = 0; column < _columns; ++column) {
      product += GetValue(row, column) * buffer[column];
    }
    vector[row] = product;
  }
  return vector;
}

template<typename T>
inline PBDMatrix<T>& PBDMatrix<T>::operator*=(const T factor)
{
  for(auto& v: _data) v *= factor;
  return *this;
}

template<typename T>
inline PBDMatrix<T> PBDMatrix<T>::operator*(const T factor)
{
  PBDMatrix<T> result(*this);
  return result * factor;
}

template<typename T>
short PBDMatrix<T>::LUDecomposition2(T tol, int* pivot) 
{
    if(_lu) delete _lu;
    _lu = new PBDMatrix<T>(*this);
    int i, j, k, imax; 
    T maxA, *ptr, absA;

    for (i = 0; i <= _columns; i++)
        pivot[i] = i; //Unit permutation matrix, P[N] initialized with N

    for (i = 0; i < _columns; i++) {
        maxA = 0.0;
        imax = i;

        for (k = i; k < _columns; k++)
            if ((absA = fabs(_lu->Data()[k*_rows+i])) > maxA) { 
                maxA = absA;
                imax = k;
            }

        if (maxA < tol) return MATRIX_INVALID; //failure, matrix is degenerate

        if (imax != i) {
            //pivoting P
            j = pivot[i];
            pivot[i] = pivot[imax];
            pivot[imax] = j;

            //pivoting rows of A
            SwapRows(i, imax);

            //counting pivots starting from N (for determinant)
            pivot[_columns]++;
        }

        for (j = i + 1; j < _columns; ++j) {
            _lu->Data()[j*_rows+i] /= _lu->Data()[i*_rows+i];

            for (k = i + 1; k < _columns; k++)
                _lu->Data()[j*_rows+k] -= _lu->Data()[j*_rows+i] * _lu->Data()[i*_rows+k];
        }
    }

    return MATRIX_VALID;  //decomposition done 
}


template<typename T>
inline short PBDMatrix<T>::LUDecomposition(const size_t m, int* pivot)
{
  T singularityThreshold = static_cast<T>(0);
  if(_rows != _columns) return MATRIX_NON_SQUARE;

  if(_lu) delete _lu;
  _lu = new PBDMatrix<T>(*this);

  // initialize permutation array and parity
  size_t row, column, i, max;
  for(row = 0; row < m; ++row)pivot[row] = row;
    
  _even = true;
  _singular = false;

  std::vector<T> luRow(_columns);
  T sum, largest, luDiag;
  // loop over columns
  for(column = 0; column < m; ++column) {
    // upper
    for(row = 0; row < column; ++ row) {
      
      _lu->GetRow(row, luRow);
      sum = luRow[column];
      for(i = 0; i < row; ++i) {
        sum -= (luRow[i] * _lu->GetValue(i, column));
      }
      _lu->SetValue(row, column, sum);
    }

    // lower
    max = column;
    largest = std::numeric_limits<T>::min();
    for(row = column; row < m; ++row) {
      _lu->GetRow(row, luRow);
      sum = luRow[column];
      for(i = 0; i < column; ++i) {
        sum -= luRow[i] * _lu->GetValue(i, column);
      }
        
      _lu->SetValue(row, column, sum);

      // maintain best permutation choice
      if(std::abs(sum) > largest) {
        largest = std::abs(sum);
        max = row;
      }
    }

    // singularity check
    if(std::abs(_lu->GetValue(max, column)) < singularityThreshold) {
      return MATRIX_IS_SINGULAR;
    }

    // pivot if necessary
    T tmp;
    if(max != column) {
      _lu->SwapRows(max, column);
      tmp = pivot[max];
      pivot[max] = pivot[column];
      pivot[column] = tmp;
      _even = 1 - _even;
    }

    // divide the lower elements by the "winning" diagonal elt.
    luDiag = _lu->GetValue(column, column);
    for(row = column + 1; row < m; ++row) {
      _lu->_data[row * m + column] = _lu->GetValue(row, column) / luDiag;
    }
    
  }
  return MATRIX_VALID;
}
  
template<typename T>
inline short PBDMatrix<T>::SolveLU(const size_t m, int* pivot, T* b, T* x)
{
  if(!_lu) return MATRIX_INVALID;

  if(_lu->_columns != m)return MATRIX_SIZE_MISMATCH;
  if(_lu->_singular)return MATRIX_IS_SINGULAR;
  int row, column, i;

  // apply permutations to b
  for(row = 0; row < m; ++row) {
    x[row] = b[pivot[row]];
  }

  // solve LY = b
  T xColumn;
  for(column = 0; column < m; ++column) {
    xColumn = x[column];
    for(i = column+1; i < m; ++i) {
      x[i] -= xColumn * _lu->GetValue(i, column);
    }
  }

  // solve UX = Y
  for(column = m-1; column >= 0; --column) {
    x[column] = x[column] / _lu->GetValue(column, column);
    xColumn = x[column];
    for(size_t i = 0; i < column; ++i) {
      x[i] -= xColumn * _lu->GetValue(i, column);
    }
  }
  return MATRIX_VALID;
}

template<typename T>
inline T PBDMatrix<T>::GetDeterminant(const size_t m)
{
  if(_singular || !_lu)return static_cast<T>(0);
    
  T determinant = _even ? static_cast<T>(1) : static_cast<T>(-1);
  
  for(size_t i = 0; i < m; ++i) {
    determinant *= _lu->GetValue(i, i);
  }
  return determinant;
}

template<typename T>
inline void PBDMatrix<T>::Solve(T* vector, T* result)
{
  T test[_columns * _columns];
  Echo("Matrix");
  PBDMath::CholeskyDecomposition<T>(Data(), &test[0], (int)_columns);

  for(size_t c = 0; c < _columns; ++c){
      for(size_t r=0; r < _rows; ++r) {
          std::cout << test[c*_columns+r] << "," ;
      }
      std::cout << std::endl;
  }
  std::cout << "@@@@@@" << std::endl;
  

  Transpose();
  Echo("TRanspose");
  PBDMath::CholeskyDecomposition<T>(Data(), &test[0], (int)_columns);


  for(size_t c = 0; c < _columns; ++c){
      for(size_t r=0; r < _rows; ++r) {
          std::cout << test[c*_columns+r] << "," ;
      }
      std::cout << std::endl;
  }
  std::cout << "@@@@@@" << std::endl;
  //CholeskyDecomposition(const float** matrix, float** result, int n)
  int pivot[_columns];

  if(LUDecomposition(_columns, &pivot[0]) == MATRIX_VALID) {  
      _lu->Echo("LU");
    for(int i =0; i < _columns; ++i) {
      result[i] = vector[pivot[i]];
      for(int k = 0; k < i; ++k) {
        result[i] -= _lu->Data()[GetIndex(i, k)] * result[k];
      }
    }

    for(int i = _columns - 1; i >= 0; --i) {
      for(int k = i + 1; k < _columns; ++k) {
        result[i] -= _lu->Data()[GetIndex(i, k)] * result[k];
      }
      result[i] /= _lu->Data()[GetIndex(i, i)];
    }
  }
}

template<typename T>
inline void PBDMatrix<T>::Inverse(PBDMatrix<T>& result)
{
  int pivot[_columns];
  T b[_columns];
  T w[_columns];

  if(LUDecomposition(_columns, &pivot[0]) == MATRIX_VALID) {  
    result.Resize(_columns, _columns);  
    for(size_t c=0; c < _columns; ++c) {
      memset(&b[0], static_cast<T>(0), _columns * sizeof(T));
      memset(&w[0], static_cast<T>(0), _columns * sizeof(T));
      b[c] = static_cast<T>(1);
      SolveLU(_columns, &pivot[0], b, w);
      result.SetColumn(c, w);
    }
  }
}

template<typename T>
inline void PBDMatrix<T>::InverseInPlace()
{
  int pivot[_columns];
  T b[_columns];
  T w[_columns];

  if(LUDecomposition(_columns, &pivot[0]) == MATRIX_VALID) {
    size_t i;
    for(i = 0; i < _columns; ++i) {
      memset(&b[0], static_cast<T>(0), _columns * sizeof(T));
      memset(&w[0], static_cast<T>(0), _columns * sizeof(T));
      b[i] = static_cast<T>(1);
      SolveLU(_columns, &pivot[0], b, w);
      SetColumn(i, &w[0]);
    }
  }
}

template<typename T>
inline void PBDMatrix<T>::Echo(const std::string& msg)
{
  std::cout << msg << std::endl;
  for(size_t row = 0; row < _rows; ++row) {
    for(size_t column = 0; column < _columns; ++column) {
      std::cout << _data[GetIndex(row, column)] << ",";
    }
    std::cout << "\n";
  }
  std::cout << "\n" << std::endl;
}

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_MATRIX_H
