//======================================================
// PRIMVAR DECLARATION
//======================================================
#ifndef JVR_GEOMETRY_PRIMVAR_H
#define JVR_GEOMETRY_PRIMVAR_H

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
struct Primvar {
  Primvar() {}
  Primvar(TfToken const& _name,
          VtValue const& _value,
          HdInterpolation const& _interp,
          TfToken const& _role,
          VtIntArray const& _indices=VtIntArray(0)) :
    name(_name),
    value(_value),
    interp(_interp),
    role(_role),
    indices(_indices) {}

  TfToken name;
  VtValue value;
  HdInterpolation interp;
  TfToken role;
  VtIntArray indices;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_PRIMVAR_H