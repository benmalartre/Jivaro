#pragma once

#include "../default.h"

AMN_NAMESPACE_OPEN_SCOPE

enum AMN_INTERPOLATION_TYPE{
  CONSTANT,
  UNIFORM,
  VARYING,
  VERTEX,
  FACE_VARYING
};

template<typename T>
struct UsdEmbreePrimVarDatas{
  AMN_INTERPOLATION_TYPE      _interpolationType;
  pxr::VtArray<T>             _datas;
};

struct UsdEmbreePrim {
  unsigned                    _geomId;
  RTCGeometryType             _type;
  RTCGeometry                 _geom;
  std::string                 _name;
  std::vector<int>            _instanceIDs;
  pxr::GfVec3f                _color;
};

AMN_NAMESPACE_CLOSE_SCOPE