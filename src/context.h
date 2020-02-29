#pragma once

#include "default.h"
#include "prim.h"

namespace AMN {

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


  struct UsdEmbreeContext {
    // usd
    std::vector<std::string>                        _files;
    pxr::UsdStageRefPtr                             _stage;
    std::unordered_map<int, UsdEmbreePrim*>         _prims;
    pxr::TfToken                                    _axis;
    float                                           _time;
    long long                                       _numPrims;
    pxr::GfMatrix4f                                 _worldMatrix;

    // image
    int                                             _width;
    int                                             _height;
    int*                                            _pixels;

    // embree
    RTCScene                                        _scene;
    RTCDevice                                       _device;
    RayStats*                                       _stats;
    bool                                            _changed;
    float                                           _debug;
    embree::Vec3fa*                                 _face_colors; 
    embree::Vec3fa*                                 _vertex_colors;

    // methods
    UsdEmbreeContext():
      _scene(NULL),
      _device(NULL),
      _changed(false),
      _debug(0.f),
      _face_colors(NULL),
      _vertex_colors(NULL),
      _width(0),
      _height(0),
      _pixels(NULL){};
    
    void SetFilePath(const std::string& filePath);
    void InitDevice();
    void CommitDevice();
    void ReleaseDevice();
    void GetNumPrims(const pxr::UsdPrim& prim);
    void CollectPrims(const pxr::UsdPrim& prim);
    void Resize(int width, int height);

    void TraverseStage();
  };

  

  

} // namespace AMN
