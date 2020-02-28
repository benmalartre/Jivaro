#pragma once

#include <embree3/rtcore.h>

#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec4f.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/imageable.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <math/vec3.h>

namespace AMN {

  enum INTERPOLATION_TYPE{
    CONSTANT,
    UNIFORM,
    VARYING,
    VERTEX,
    FACE_VARYING
  };

  template<typename T>
  struct UsdEmbreePrimVarDatas{
    INTERPOLATION_TYPE          _interpolationType;
    pxr::VtArray<T>             _datas;
  };

  struct UsdEmbreePrim {
    unsigned                    _geomId;
    float                       _worldMatrix[16];
    RTCGeometryType             _type;
    RTCGeometry                 _geom;
  };

} // namespace AMN
