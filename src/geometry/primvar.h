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
  Primvar(pxr::TfToken const& _name,
          pxr::VtValue const& _value,
          pxr::HdInterpolation const& _interp,
          pxr::TfToken const& _role,
          pxr::VtIntArray const& _indices=pxr::VtIntArray(0)) :
    name(_name),
    value(_value),
    interp(_interp),
    role(_role),
    indices(_indices) {}

  pxr::TfToken name;
  pxr::VtValue value;
  pxr::HdInterpolation interp;
  pxr::TfToken role;
  pxr::VtIntArray indices;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_PRIMVAR_H