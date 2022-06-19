#ifndef JVR_PBD_MATRIX_H
#define JVR_PBD_MATRIX_H

#include "../common.h"
#include <vector>
#include <limits>

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

    void Resize(int rows, int columns);
    void Transpose();
    int GetIndex(int row, int column);

    short SetValue(int row, int column, T value);
    short SetRow(int row, std::vector<T>& values);
    short SetColumn(int column, std::vector<T>& values);

    short GetValue(int row, int column, T& value);
    short GetRow(int row, std::vector<T>& values);
    short GetColumn(int column, std::vector<T>& values);

    short GetRowMinimum(int row, T& value);
    short GetRowsMinimum(std::vector<T>& values);
    short GetRowMaximum(int row, T& value);
    short GetRowsMaximum(std::vector<T>& values);

    short GetColumnMinimum(int column, T& value);
    short GetColumnsMinimum(std::vector<T>& values);
    short GetColumnMaximum(int column, T& value);
    short GetColumnsMaximum(std::vector<T>& values);

    void SwapRows(int a, int b);
    void SwapColumns(int a, int b);

    PBDMatrix operator+(PBDMatrix& other);
    PBDMatrix& operator+=(PBDMatrix& other);
    PBDMatrix operator-(PBDMatrix& other);
    PBDMatrix& operator-=(PBDMatrix& other);
    PBDMatrix operator*(PBDMatrix& other);
    PBDMatrix& operator*=(PBDMatrix& other);
    PBDMatrix operator*(const std::vector<T>& vector);
    PBDMatrix& operator*=(const T factor);
    PBDMatrix operator*(const T factor);

    short LUDecomposition(std::vector<int>& pivot); 
    short SolveLU(const std::vector<int>& pivot,
        std::vector<T>& b, std::vector<T>& x);
    T GetDeterminant(std::vector<int>& pivot);

    PBDMatrix Inverse();
    PBDMatrix& InverseInPlace();

    void Echo();

private:
    int                         _rows;
    int                         _columns;
    std::vector<T>              _data;
    PBDMatrix<T>*               _lu;
    bool                        _transposed;
    bool                        _singular;
    bool                        _even;
};


template<typename T>
PBDMatrix<T>::PBDMatrix() 
  : _lu(NULL)
  , _rows(0)
  , _columns(0)
  , _transposed(false)
  , _even(true)
  , _singular(false)
{
}

template<typename T>
PBDMatrix<T>::PBDMatrix(int rows, int columns) 
  : _lu(NULL)
  , _rows(rows)
  , _columns(columns)
  , _transposed(false)
  , _even(true)
  , _singular(false)
{
  _data.resize(_rows * _columns);
  memset(&_data[0], 0, _rows * _columns * sizeof(T));
}

template<typename T>
PBDMatrix<T>::PBDMatrix(const PBDMatrix& other) 
  : _lu(NULL)
{
  _rows = other._rows;
  _columns = other._columns;
  _data = other._data;
  _transposed = other._transposed;
  _even = other._even;
  _singular = other._singular;

  if(other._lu) {
    _lu = new PBDMatrix<T>(*(other._lu));
  }
}

template<typename T>
PBDMatrix<T>::~PBDMatrix()
{
  if(_lu) delete _lu;
}

template<typename T>
void PBDMatrix<T>::Resize(int rows, int columns)
{
  _rows = rows;
  _columns = columns;
  _data.resize(_rows * _columns);
  memset(&_data[0], 0, _rows * _columns * sizeof(T));
}

template<typename T>
void PBDMatrix<T>::Transpose()
{
  int tmp = _rows;
  _rows = _columns;
  _columns = tmp;
  _transposed = 1 - _transposed;
}

template<typename T>
int PBDMatrix<T>::GetIndex(int row, int column)
{
  if(row >= 0 && row < _rows && column >= 0 && column < _columns) {
    if(_transposed){
      return _rows * column + row;
    } else {
      return _columns * row + column;
    }
  }
  return -1;
}

template<typename T>
short PBDMatrix<T>::SetValue(int row, int column, T value)
{
  int idx = GetIndex(row, column);
  if(idx >= 0) {
    _data[idx] = value;
    return MATRIX_VALID;
  } else {
    return MATRIX_INDEX_OUTOFBOUND;
  }
}

template<typename T>
short PBDMatrix<T>::SetRow(int row, std::vector<T>& values)
{
  if(row < _rows && values.size() == _columns) {
    for(int column = 0; column < _columns; ++column) {
      _data[GetIndex(row, column)] = values[column];
    }
    return MATRIX_VALID;
  } else {
    return MATRIX_SIZE_MISMATCH;
  }
}

template<typename T>
short PBDMatrix<T>::SetColumn(int column, std::vector<T>& values)
{
  if(column < _columns && values.size() == _rows) {
    for(int row = 0; row < _rows; ++row) {
      SetValue(row, column, values[row]);
    }
    return MATRIX_VALID;
  } else {
    return MATRIX_SIZE_MISMATCH;
  }
}

template<typename T>
short PBDMatrix<T>::GetValue(int row, int column, T& value)
{
  int idx = GetIndex(row, column);
  if(idx >= 0) {
    value = _data[idx];
    return MATRIX_VALID;
  } else  {
    return MATRIX_INDEX_OUTOFBOUND;
  }
}

template<typename T>
short PBDMatrix<T>::GetRow(int row, std::vector<T>& values)
{
  if(row >= 0 && row < _rows) {
    values.resize(_columns);
    for(size_t column = 0; column < _columns; ++column) {
       GetValue(row, column, values[column]);
    }
    return MATRIX_VALID;
  } else {
    return MATRIX_INDEX_OUTOFBOUND;
  }
}

template<typename T>
short PBDMatrix<T>::GetColumn(int column, std::vector<T>& values)
{
  if(column >= 0 && column < _columns) {
    values.resize(_rows);
    for(size_t row = 0; row < _rows; ++row) {
        GetValue(row, column,  values[row]);
    }
    return MATRIX_VALID;
  } else {
    return MATRIX_INDEX_OUTOFBOUND;
  }
}

template<typename T>
short PBDMatrix<T>::GetRowMinimum(int row, T& value)
{
  if(row >= 0 && row < _rows) {
    value = std::numeric_limits<T>::max();
    for(size_t column = 0; column < _columns; ++column) {
      T curValue;
      GetValue(row, column, curValue);
      if(curValue < value) {
        value = curValue;
      }
    }
    return MATRIX_VALID;
  } else {
    return MATRIX_INDEX_OUTOFBOUND;
  }
}

template<typename T>
short PBDMatrix<T>::GetRowsMinimum(std::vector<T>& values)
{
  values.resize(_rows);
  for(size_t row = 0; row < _rows; ++row) {
    GetRowMinimum(row, values[row]);
  }
  return MATRIX_VALID;
}

template<typename T>
short PBDMatrix<T>::GetRowMaximum(int row, T& value)
{
  if(row >= 0 && row < _rows) {
    value = std::numeric_limits<T>::min();
    for(size_t column = 0; column < _columns; ++column) {
      T curValue;
      GetValue(row, column, curValue);
      if(curValue > value) {
        value = curValue;
      }
    }
    return MATRIX_VALID;
  } else {
    return MATRIX_INDEX_OUTOFBOUND;
  }
}

template<typename T>
short PBDMatrix<T>::GetRowsMaximum(std::vector<T>& values)
{
  values.resize(_rows);
  for(size_t row = 0; row < _rows; ++row) {
    GetRowMaximum(row, values[row]);
  }
  return MATRIX_VALID;
}

template<typename T>
short PBDMatrix<T>::GetColumnMinimum(int column, T& value)
{
  if(column >= 0 && column < _columns) {
    value = std::numeric_limits<T>::max();
    for(size_t row = 0; row < _rows; ++row) {
      T curValue;
      GetValue(row, column, curValue);
      if(curValue < value) {
        value = curValue;
      }
    }
    return MATRIX_VALID;
  } else {
    return MATRIX_INDEX_OUTOFBOUND;
  }
}

template<typename T>
short PBDMatrix<T>::GetColumnsMinimum(std::vector<T>& values)
{
  values.resize(_columns);
  for(size_t column = 0; column < _columns; ++column) {
    if(GetColumnMinimum(column, values[column]) != MATRIX_VALID) 
      return MATRIX_INDEX_OUTOFBOUND;
  }
  return MATRIX_VALID;
}

template<typename T>
short PBDMatrix<T>::GetColumnMaximum(int column, T& value)
{
  if(column >= 0 && column < _columns) {
    value = std::numeric_limits<T>::min();
    for(size_t row = 0; row < _rows; ++row) {
      T curValue;
      GetValue(row, column, curValue);
      if(curValue > value) {
        value = curValue;
      }
    }
    return MATRIX_VALID;
  } else {
    return MATRIX_INDEX_OUTOFBOUND;
  }
}

template<typename T>
short PBDMatrix<T>::GetColumnsMaximum(std::vector<T>& values)
{
  values.resize(_columns);
  for(size_t column = 0; column < _columns; ++column) {
    if(GetColumnMaximum(column, values[column]) != MATRIX_VALID) 
      return MATRIX_INDEX_OUTOFBOUND;
  }
  return MATRIX_VALID;
}

template<typename T>
void PBDMatrix<T>::SwapRows(int a, int b)
{
  for(size_t column = 0; column < _columns; ++column) {
    std::swap(_data[a * _columns + column], _data[b * _columns + column]);
  }
}

template<typename T>
void PBDMatrix<T>::SwapColumns(int a, int b)
{
  for(size_t row = 0; row < _rows; ++row) {
    std::swap(_data[_columns * row + a], _data[_columns + row + b]);
  }
}

template<typename T>
PBDMatrix<T> PBDMatrix<T>::operator+(PBDMatrix<T>& other)
{
  if(_data.size() != other._data.size()) {
    return *this;
  }
  PBDMatrix result(_rows, _columns);
  for(size_t i = 0; i < _data.size(); ++i) {
    result[i] = _data[i] + other._data[i];
  }
  return result;
}

template<typename T>
PBDMatrix<T>& PBDMatrix<T>::operator+=(PBDMatrix<T>& other)
{
  if(_data.size() != other._data.size()) {
    return *this;
  }
  for(size_t i = 0; i < _data.size(); ++i) {
    _data[i] += other._data[i];
  }
  return *this;
}

template<typename T>
PBDMatrix<T> PBDMatrix<T>::operator-(PBDMatrix<T>& other)
{
  if(_data.size() != other._data.size()) {
    return *this;
  }
  PBDMatrix result(_rows, _columns);
  for(size_t i = 0; i < _data.size(); ++i) {
    result[i] = _data[i] - other._data[i];
  }
  return result;
}

template<typename T>
PBDMatrix<T>& PBDMatrix<T>::operator-=(PBDMatrix<T>& other)
{
  if(_data.size() != other._data.size()) {
    return *this;
  }
  for(size_t i = 0; i < _data.size(); ++i) {
    _data[i] -= other._data[i];
  }
  return *this;
}

template<typename T>
PBDMatrix<T> PBDMatrix<T>::operator*(PBDMatrix<T>& other)
{
  if(_columns != other.rows) {
    return *this;
  } 

  PBDMatrix<T> result(_rows, _columns);
  for(size_t i = 0; i < _rows; ++i) {
    for(size_t k = 0; k < other._columns; ++k) {
      for(size_t j = 0; j < _columns; ++j) {
        size_t idx = GetIndex(i, j);
        result._data[idx] = _data[idx] + GetValue(i, k) * other.GetValue(k, j);
      }
    }
  }
}

template<typename T>
PBDMatrix<T>& PBDMatrix<T>::operator*=(PBDMatrix<T>& other)
{
  if(_columns != other.rows) {
    return *this;
  } 

  PBDMatrix<T> copy(*this);
  memset(&_data[0], 0, _rows * _columns * sizeof(T));

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
PBDMatrix<T> PBDMatrix<T>::operator*(const std::vector<T>& vector)
{
  if(_columns != vector.size()) {
    return *this;
  } 

  PBDMatrix<T> result(_columns, 1);
  for(size_t row = 0; row < _rows; ++row) {
    T product = static_cast<T>(0);
    for(size_t column = 0; column < _columns; ++column) {
      product += GetValue(row, column) * vector[column];
    }
    result.SetValue(row, 0, product);
  }
  return result;
}

template<typename T>
PBDMatrix<T>& PBDMatrix<T>::operator*=(const T factor)
{
  for(auto& v: _data) v *= factor;
  return *this;
}

template<typename T>
PBDMatrix<T> PBDMatrix<T>::operator*(const T factor)
{
  PBDMatrix<T> result(*this);
  return result * factor;
}

template<typename T>
short PBDMatrix<T>::LUDecomposition(std::vector<int>& pivot)
{
  T singularityThreshold = static_cast<T>(0);
  if(_rows != _columns) return MATRIX_NON_SQUARE;

  size_t m = _columns;
  if(_lu) delete _lu;
  _lu = new PBDMatrix<T>(*this);

  // initialize permutation array and parity
  size_t row, column, i;
  for(row = 0; row < m; ++row)pivot[row] = row;
    
  _even = true;
  _singular = false;

  // loop over columns
  for(column = 0; column < m; ++column) {
    // upper
    for(row = 0; row < column; ++ row) {
      std::vector<T> luRow;
      _lu->GetRow(row, luRow);
      T sum = luRow[column];
      for(i = 0; i < row; ++i) {
        T value;
        if(_lu->GetValue(i, column, value) == MATRIX_VALID)
            sum -= (luRow[i] * value);
      }
        
      _lu->SetValue(row, column, sum);
    }

    // lower
    size_t max = column;
    T largest = std::numeric_limits<T>::min();
    for(row = column; row < m; ++row) {
      std::vector<T> luRow;
      _lu->GetRow(row, luRow);
      T sum = luRow[column];
      for(i = 0; i < column; ++i) {
        T value;
        if(_lu->GetValue(i, column, value) == MATRIX_VALID) 
            sum -= luRow[i] * value;
      }
        
      _lu->SetValue(row, column, sum);

      // maintain best permutation choice
      if(std::abs(sum) > largest) {
        largest = std::abs(sum);
        max = row;
      }
    }

    // singularity check
    T value;
    if(_lu->GetValue(max, column, value) == MATRIX_VALID) {
        if(std::abs(value) < singularityThreshold) {
        return MATRIX_IS_SINGULAR;
        }
    } else {
        return MATRIX_INDEX_OUTOFBOUND;
    }

    // pivot if necessary
    if(max != column) {
      _lu->SwapRows(max, column);
      std::swap(pivot[max], pivot[column]);
      _even = 1 - _even;
    }

    // divide the lower elements by the "winning" diagonal elt.
    T luDiag;
    if(_lu->GetValue(column, column, luDiag) == MATRIX_VALID) {
        for(row = column + 1; row < m; ++row) {
            T value;
            if(_lu->GetValue(row, column, value) == MATRIX_VALID) {
                _lu->_data[row * m + column] = value / luDiag;
            } else {
                return MATRIX_INDEX_OUTOFBOUND;
            }
        }
    }
  }
  return MATRIX_VALID;
}
  
template<typename T>
short PBDMatrix<T>::SolveLU(const std::vector<int>& pivot, std::vector<T>& b,
  std::vector<T>& x)
{
  if(!_lu) return MATRIX_INVALID;
  const int m = pivot.size();

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
      T value;
      if(_lu->GetValue(i, column, value) == MATRIX_VALID)
        x[i] -= xColumn * value;
    }
  }

  // solve UX = Y
  T value;
  for(column = m-1; column >= 0; --column) {
    if(_lu->GetValue(column, column, value) == MATRIX_VALID) {
      x[column] = x[column] / value;
      xColumn = x[column];
      for(size_t i = 0; i < column; ++i) {
          if(_lu->GetValue(i, column, value) == MATRIX_VALID)
            x[i] -= xColumn * value;
      }
    }
  }

  return MATRIX_VALID;
}

template<typename T>
T PBDMatrix<T>::GetDeterminant(std::vector<int>& pivot)
{
  if(_singular || !_lu)return static_cast<T>(0);
    
  size_t m = pivot.size();
  size_t i;
  T determinant = _even ? static_cast<T>(1) : static_cast<T>(-1);
  
  for(i = 0; i < m; ++i) {
    determinant *= _lu->GetValue(i, i);
  }
  return determinant;
}

template<typename T>
PBDMatrix<T> PBDMatrix<T>::Inverse()
{
  PBDMatrix result(*this);
  std::vector<int> pivot(_columns);

  if(LUDecomposition(pivot) == MATRIX_VALID) {
    result.Resize(_columns, _columns);

    for(size_t c=0; c < _columns; ++c) {
      std::vector<T> b(_columns);
      std::vector<T> w(_columns);
      b[c] = static_cast<T>(1);
      SolveLU(pivot, b, w);
      result.SetColumn(c, w);
    }
    return result;
  } else {
    return *this;
  }
}

template<typename T>
PBDMatrix<T>& PBDMatrix<T>::InverseInPlace()
{
  std::vector<int> pivot(_columns);
  if(LUDecomposition(pivot) == MATRIX_VALID) {
    size_t i;
    for(i = 0; i < _columns; ++i) {
      std::vector<T> b(_columns);
      std::vector<T> w(_columns);
      b[i] = static_cast<T>(1);
      SolveLU(pivot, b, w);
      SetColumn(i, w);
    }
  }
  return *this;
}

template<typename T>
void PBDMatrix<T>::Echo()
{
    for(size_t column = 0; column < _columns; ++column) {
        for(size_t row = 0; row < _rows; ++row) {
            std::cout << _data[GetIndex(row, column)] << ",";
        }
        std::cout << "\n";
    }
    std::cout << "\n" << std::endl;
}

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_MATRIX_H

