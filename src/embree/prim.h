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
struct AmnUsdEmbreePrimVarDatas{
  AMN_INTERPOLATION_TYPE      _interpolationType;
  pxr::VtArray<T>             _datas;
};

struct AmnUsdEmbreePrim {
  unsigned                    _geomId;
  RTCGeometryType             _type;
  RTCGeometry                 _geom;
  std::string                 _name;
  std::vector<int>            _instanceIDs;
};

AMN_NAMESPACE_CLOSE_SCOPE