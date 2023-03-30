#include "../geometry/matrix.h"

JVR_NAMESPACE_OPEN_SCOPE

template <typename T>
Matrix<T>::Matrix<T>(size_t rows, size_t columns)
  : _rows(rows), _columns(columns)
{
  _matrix.resize(_rows * _columns);
}

template <typename T>
void Matrix<T>::Resize(size_t rows, size_t columns)
{
  _rows = rows;
  _column = columns;
  _matrix.resize(_rows * _columns);
}


template <typename T>
void Matrix<T>::Transpose() {
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
Matrix<T>::Vector Matrix<T>::GetRow(size_t row)
{
  Vector values;
  values.resize(_columns);
  for (size_t i = 0; i < _columns; ++i) {
    values[i] = Get(row, i);
  }
  return values;
}

template <typename T>
Matrix<T>::Vector Matrix<T>::GetColumn(size_t column)
{
  Vector values;
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
Matrix<T>::Vector Matrix<T>::GetRowsMinimum()
{
  Matrix<T>::Vector results;
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
void Matrix<T>::GetRowsMaximum(*m.Matrix_t, Array maximums.f(1))
ReDim maximums(*m\rows)
Define i
For i = 0 To * m\rows - 1
maximums(i) = GetRowMaximum(*m, i)
Next
ProcedureReturn #True
EndProcedure

template <typename T>
void Matrix<T>::GetColumnMinimum(*m.Matrix_t, column.i)
Define minValue.f = Math::#F32_MAX, value.f
Define j
For j = 0 To * m\columns - 1
value = Get(*m, row, j)
If value < minValue:
minValue = value
EndIf
Next
ProcedureReturn minValue
EndProcedure

template <typename T>
void Matrix<T>::GetColumnsMinimum(*m.Matrix_t, Array minimums.f(1))
ReDim minimums(*m\columns)
Define i
For i = 0 To * m\columns - 1
minimums(i) = GetColumnMinimum(*m, i)
Next
ProcedureReturn #True
EndProcedure

template <typename T>
void Matrix<T>::GetColumnMaximum(*m.Matrix_t, column.i)
Define maxValue.f = Math::#F32_MIN, value.f
Define j
For j = 0 To * m\columns - 1
value = Get(*m, j, column)
If value > maxValue:
maxValue = value
EndIf
Next
ProcedureReturn maxValue
EndProcedure

template <typename T>
void Matrix<T>::GetColumnsMaximum(*m.Matrix_t, Array maximums.f(1))
ReDim maximums(*m\columns)
Define i
For i = 0 To * m\columns - 1
maximums(i) = GetColumnMaximum(*m, i)
Next
ProcedureReturn #True
EndProcedure

template <typename T>
void Matrix<T>::AddInPlace(*m.Matrix_t, *o.MAtrix_t)
If Not ArraySize(*m\matrix()) = ArraySize(*o\matrix()) : ProcedureReturn #MATRIX_SIZE_MISMATCH : EndIf
Define i
For i = 0 To ArraySize(*m\matrix()) - 1
* m\matrix(i) + *o\matrix(i)
Next

EndProcedure

template <typename T>
void Matrix<T>::Add(*m.Matrix_t, *a.Matrix_t, *b.Matrix_t)
If Not(*a\rows = *b\rows And * a\columns = *b\columns) : ProcedureReturn #MATRIX_SIZE_MISMATCH : EndIf
Resize(*m, *a\rows, *a\columns)
Define i
For i = 0 To ArraySize(*m\matrix()) - 1
* m\matrix(i) = *a\matrix(i) + *b\matrix(i)
Next
EndProcedure

template <typename T>
void Matrix<T>::SubtractInPlace(*m.Matrix_t, *o.MAtrix_t)
If Not ArraySize(*m\matrix()) = ArraySize(*o\matrix()) : ProcedureReturn #MATRIX_SIZE_MISMATCH : EndIf
Define i
For i = 0 To ArraySize(*m\matrix()) - 1
* m\matrix(i) - *o\matrix(i)
Next

EndProcedure

template <typename T>
void Matrix<T>::Subtract(*m.Matrix_t, *a.Matrix_t, *b.Matrix_t)
If Not(*a\rows = *b\rows And * a\columns = *b\columns) : ProcedureReturn #MATRIX_SIZE_MISMATCH : EndIf
Resize(*m, *a\rows, *a\columns)
Define i
For i = 0 To ArraySize(*m\matrix()) - 1
* m\matrix(i) = *a\matrix(i) - *b\matrix(i)
Next
EndProcedure


template <typename T>
void Matrix<T>::Multiply(*m.Matrix_t, *a.Matrix_t, *b.Matrix_t)
If* a\columns <>* b\rows : ProcedureReturn #MATRIX_SIZE_MISMATCH : EndIf

Define i, j, k
; first reset output matrix
For i = 0 To ArraySize(*m\matrix()) - 1 : *m\matrix(i) = 0 : Next

For i = 0 To * a\rows - 1
For k = 0 To * b\columns - 1
For j = 0 To * a\columns - 1
* m\matrix(GetIndex(*m, i, j)) + (Get(*a, i, k) * Get(*b, k, j))
Next
Next
Next
EndProcedure

template <typename T>
void Matrix<T>::MultiplyInPlace(*m.Matrix_t, *o.Matrix_t)
Define * tmp.Matrix_t = Copy(*m)
Multiply(*m, *tmp, *o)
Delete(*tmp)
EndProcedure

template <typename T>
void Matrix<T>::MultiplyVector(*m.Matrix_t, *o.Matrix_t, Array vector.f(1))
If ArraySize(vector()) < > * o\columns : ProcedureReturn #MATRIX_SIZE_MISMATCH : EndIf
Resize(*m, *o\columns, 1)
Define product.f
Define i, j
For i = 0 To * o\rows - 1
product = 0
For j = 0 To * o\columns - 1
product + Get(*o, i, j) * vector(j)
Next
Matrix::Set(*m, i, 0, product)
Next
EndProcedure

template <typename T>
void Matrix<T>::Scale(*m.Matrix_t, *o.Matrix_t, v.f)
Resize(*m, *o\rows, *o\columns)
Define i
For i = 0 To ArraySize(*o\matrix()) - 1
* m\matrix(i) = *o\matrix(i) * v
Next
EndProcedure

Procedure ScaleInPlace(*m.Matrix_t, v.f)
Define i
For i = 0 To ArraySize(*m\matrix()) - 1
* m\matrix(i) * v
Next
EndProcedure

template <typename T>
void Matrix<T>::SwapRows(*m.Matrix_t, a.i, b.i)
Define i
For i = 0 To * m\columns - 1
Swap * m\matrix(a * *m\columns + i), * m\matrix(b** m\columns + i)
Next
EndProcedure

Procedure SwapColumns(*m.Matrix_t, a.i, b.i)
Define i
For i = 0 To * m\rows - 1
Swap * m\matrix(*m\columns * i + a), * m\matrix(*m\columns* i + b)
Next
EndProcedure

template <typename T>
void Matrix<T>::Inverse(*m.Matrix_t, *o.Matrix_t)
Dim piv(*o\columns)
If Not LUDecomposition(*o, piv())
Resize(*m, *o\columns, *o\columns)
Define i
For i = 0 To * o\columns - 1
Dim b.f(*o\columns)
Dim w.f(*o\columns)
b(i) = 1.0
SolveLU(*o, piv(), b(), w())
SetColumn(*m, i, w())
Next
ProcedureReturn #True
EndIf
ProcedureReturn #False
EndProcedure

Procedure InverseInPlace(*m.Matrix_t)
Dim piv(*m\columns)
If Not LUDecomposition(*m, piv())
Define i
For i = 0 To * m\columns - 1
Dim b.f(*m\columns)
Dim w.f(*m\columns)
b(i) = 1.0
SolveLU(*m, piv(), b(), w())
SetColumn(*m, i, w())
Next
ProcedureReturn #True
EndIf
ProcedureReturn #False
EndProcedure


template <typename T>
void Matrix<T>::GetDeterminant(*m.Matrix_t, Array pivot.i(1))
If* m\singular Or Not* m\lu : ProcedureReturn 0 : EndIf

Define m = ArraySize(pivot())
Define i
Define determinant.f
If * m\even : determinant = 1 : Else : determinant = -1 : EndIf

For i = 0 To m - 1
determinant * Get(*m\lu, i, i)
Next
ProcedureReturn determinant
EndProcedure

template <typename T>
int Matrix<T>::LUDecomposition(Matrix<T>::Pivots& pivots)
{
  float singularityThreshold = 0.f;
  if (_rows <> _columns) {
    return MATRIX_NON_SQUARE;
  }

  size_t m = _columns;
If * m\lu : Delete(*m\lu) : EndIf
* m\lu = Copy(*m)

; initialize permutation array And parity
Define row, column, i
For row = 0 To m - 1 : pivot(row) = row : Next

* m\even = #True
* m\singular = #False

; loop over columns
For column = 0 To m - 1
; upper
For row = 0 To column - 1
Dim luRow.f(0)
GetRow(*m\lu, row, luRow())
Define sum.f = luRow(column)
For i = 0 To row - 1
sum - (luRow(i) * Get(*m\lu, i, column))
Next

Set(*m\lu, row, column, sum)
Next

; lower
Define max = column;  permutation row
Define largest = Math::#F32_MIN
For row = column To m - 1
Dim luRow(0)
GetRow(*m\lu, row, luRow())
Define sum.f = luRow(column)
For i = 0 To column - 1
sum - (luRow(i) * Get(*m\lu, i, column))
Next

Set(*m\lu, row, column, sum)

; maintain best permutation choice
If Abs(sum) > largest
largest = Abs(sum)
max = row
EndIf
Next

; singularity check
If Abs(Get(*m\lu, max, column)) < singularityThreshold
  ProcedureReturn #MATRIX_IS_SINGULAR
  EndIf

  ; pivot if necessary
  If max <> column
  SwapRows(*m\lu, max, column)
  Swap pivot(max), pivot(column)
  * m\even = 1 - *m\even
  EndIf

  ; divide the lower elements by the "winning" diagonal elt.
  Define luDiag.f = Get(*m\lu, column, column)
  For row = column + 1 To m - 1
  * m\lu\matrix(row * m + column) = Get(*m\lu, row, column) / luDiag
  Next
  Next

  EndProcedure

template <typename T>
int Matrix<T>::SolveLU(Matrix<T>::Pivots & pivots, Matrix<T>::Vector & b, Matrix<T>::Vector & x)
{
  if (!_lu)return MATRIX_INVALID;

  size_t m = pivots.size();
  if (_lu->_columns <> m)return MATRIX_SIZE_MISMATCH;
  if (_lu->_IsSingular())return MATRIX_IS_SINGULAR;

  size_t row, column, i
  // apply permutations to b
  for (row = 0; row < m; ++row) {
    x[row] = b[pivots[row]];
  }

  // solve LY = b
  for (column = 0; column < m; ++column) {
    T value = x[column];
    for (i = column + 1; i < m; ++i) {
      x[i] -= value * _lu->Get(i, column);
    }
  }

  // solve UX = Y
  for (column = m - 1; column >= 0; --column) {
    x[column] = x[column] / _lu->Get(column, column);
    T value = x[column];
    for (i = 0; i < column; ++i) {
      x[i] -= value * _lu->Get(i, column);
    }
  }
  return MATRIX_VALID;
}