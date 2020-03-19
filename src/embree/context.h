#pragma once

#include "default.h"
#include "prim.h"
#include "../app/camera.h"
AMN_NAMESPACE_OPEN_SCOPE

// extern AnmUsdEmbreeContext* EMBREE_CTXT;
// render statistics
struct RayStats
{
  int numRays;
  int pad[32-1];
};

#if defined(RAY_STATS)
#if defined(ISPC)
inline void RayStats_addRay(RayStats& stats)       { stats.numRays += popcnt(1); }
inline void RayStats_addShadowRay(RayStats& stats) { stats.numRays += popcnt(1); }
#else // C++
__forceinline void RayStats_addRay(RayStats& stats)        { stats.numRays++; }
__forceinline void RayStats_addShadowRay(RayStats& stats)  { stats.numRays++; }
#endif
#else // disabled
inline void RayStats_addRay(RayStats& stats)       {}
inline void RayStats_addShadowRay(RayStats& stats) {}
#endif

inline bool NativePacketSupported(RTCDevice device)
{
  if (sizeof(float) == 1*4) return true;
  else if (sizeof(float) == 4*4) return rtcGetDeviceProperty(device,RTC_DEVICE_PROPERTY_NATIVE_RAY4_SUPPORTED);
  else if (sizeof(float) == 8*4) return rtcGetDeviceProperty(device,RTC_DEVICE_PROPERTY_NATIVE_RAY8_SUPPORTED);
  else if (sizeof(float) == 16*4) return rtcGetDeviceProperty(device,RTC_DEVICE_PROPERTY_NATIVE_RAY16_SUPPORTED);
  else return false;
}


struct AmnUsdEmbreeContext {
  // usd
  std::vector<std::string>                        _files;
  pxr::UsdStageRefPtr                             _stage;
  std::vector<AmnUsdEmbreePrim*>                  _prims;
  pxr::TfToken                                    _axis;
  pxr::UsdTimeCode                                _time;
  long long                                       _numPrims;
  pxr::GfMatrix4f                                 _worldMatrix;
  pxr::UsdGeomXformCache*                         _xformCache;
  pxr::UsdGeomBBoxCache*                          _bboxCache;

  // image
  int                                             _width;
  int                                             _height;
  int*                                            _pixels;
  int*                                            _lowPixels;

  // embree
  RTCScene                                        _scene;
  RTCDevice                                       _device;
  RayStats*                                       _stats;
  bool                                            _changed;
  float                                           _debug;

  // opengl
  GLuint                                          _screenSpaceQuadPgm;

  // parameters
  int                                             _renderMode;

  // methods
  AmnUsdEmbreeContext();
  ~AmnUsdEmbreeContext();
  
  void SetFilePath(const std::string& filePath);
  void InitDevice(AmnCamera* camera);
  void CommitDevice();
  void ReleaseDevice();
  void GetNumPrims(const pxr::UsdPrim& prim);
  void CollectPrims(const pxr::UsdPrim& prim);
  void Resize(int width, int height);

  void TraverseStage();
};

AMN_NAMESPACE_CLOSE_SCOPE
