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
  float                       _worldMatrix[12];
  RTCGeometryType             _type;
  RTCGeometry                 _geom;
  std::string                 _name;
  std::vector<int>            _instanceIDs;
};

#define AMN_USD_EMBREE_XFORM_FORMAT RTC_FORMAT_FLOAT3X4_ROW_MAJOR

static void 
AmnUsdEmbreeSetTransform(AmnUsdEmbreePrim* prim, const pxr::GfMatrix4d& m)
{
  prim->_worldMatrix[0] = static_cast<float>(m[0][0]);
  prim->_worldMatrix[1] = static_cast<float>(m[0][1]);
  prim->_worldMatrix[2] = static_cast<float>(m[0][2]);

  prim->_worldMatrix[3] = static_cast<float>(m[1][0]);
  prim->_worldMatrix[4] = static_cast<float>(m[1][1]);
  prim->_worldMatrix[5] = static_cast<float>(m[1][2]);

  prim->_worldMatrix[6] = static_cast<float>(m[2][0]);
  prim->_worldMatrix[7] = static_cast<float>(m[2][1]);
  prim->_worldMatrix[8] = static_cast<float>(m[2][2]);

  prim->_worldMatrix[9]  = static_cast<float>(m[3][0]);
  prim->_worldMatrix[10] = static_cast<float>(m[3][1]);
  prim->_worldMatrix[11] = static_cast<float>(m[3][2]);

  rtcSetGeometryTransform(
    prim->_geom,
    0,
    AMN_USD_EMBREE_XFORM_FORMAT,
    prim->_worldMatrix
  );

}

static void 
AmnUsdEmbreeSetTransform(AmnUsdEmbreePrim* prim, const pxr::GfMatrix4f& m)
{
  prim->_worldMatrix[0] = m[0][0];
  prim->_worldMatrix[1] = m[0][1];
  prim->_worldMatrix[2] = m[0][2];

  prim->_worldMatrix[3] = m[1][0];
  prim->_worldMatrix[4] = m[1][1];
  prim->_worldMatrix[5] = m[1][2];

  prim->_worldMatrix[6] = m[2][0];
  prim->_worldMatrix[7] = m[2][1];
  prim->_worldMatrix[8] = m[2][2];

  prim->_worldMatrix[9]  = m[3][0];
  prim->_worldMatrix[10] = m[3][1];
  prim->_worldMatrix[11] = m[3][2];

  rtcSetGeometryTransform(
    prim->_geom,
    0,
    AMN_USD_EMBREE_XFORM_FORMAT,
    prim->_worldMatrix
  );
}

AMN_NAMESPACE_CLOSE_SCOPE