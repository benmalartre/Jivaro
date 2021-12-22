#ifndef JVR_EMBREE_PRIM_H
#define JVR_EMBREE_PRIM_H

#include "../common.h"
#include "../embree/utils.h"
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/base/tf/hashmap.h>

PXR_NAMESPACE_OPEN_SCOPE

enum JVR_INTERPOLATION_TYPE{
  CONSTANT,
  UNIFORM,
  VARYING,
  VERTEX,
  FACE_VARYING
};

template<typename T>
struct UsdEmbreePrimVarDatas{
  JVR_INTERPOLATION_TYPE      _interpolationType;
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

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_EMBREE_PRIM_H