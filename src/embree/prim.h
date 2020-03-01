#pragma once

#include "../default.h"

PXR_NAMESPACE_OPEN_SCOPE

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
  float                       _worldMatrix[16];
  RTCGeometryType             _type;
  RTCGeometry                 _geom;
  std::string                 _name;
};

PXR_NAMESPACE_CLOSE_SCOPE