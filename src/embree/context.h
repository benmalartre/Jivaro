#pragma once

#include "default.h"
#include "prim.h"
#include "../app/camera.h"
AMN_NAMESPACE_OPEN_SCOPE

struct UsdEmbreeMaster;
typedef 
pxr::TfHashMap< pxr::SdfPath, UsdEmbreePrim*, pxr::SdfPath::Hash > _PrimCacheMap;

struct UsdEmbreeContext {
  // usd
  std::vector<std::string>                        _files;
  pxr::UsdStageRefPtr                             _stage;
  std::vector<UsdEmbreePrim*>                  _prims;
  std::vector<UsdEmbreeMaster*>                _masters;
  pxr::TfToken                                    _axis;
  pxr::UsdTimeCode                                _time;
  long long                                       _numPrims;
  pxr::GfMatrix4f                                 _worldMatrix;
  pxr::UsdGeomXformCache*                         _xformCache;
  pxr::UsdGeomBBoxCache*                          _bboxCache;
  _PrimCacheMap                                   _primCacheMap;

  // image
  int                                             _width;
  int                                             _height;
  int*                                            _pixels;
  int*                                            _lowPixels;

  // embree
  RTCScene                                        _scene;
  RTCDevice                                       _device;
  bool                                            _changed;
  float                                           _debug;

  // opengl
  GLuint                                          _screenSpaceQuadPgm;

  // parameters
  int                                             _renderMode;

  // methods
  UsdEmbreeContext();
  ~UsdEmbreeContext();
  
  void SetFilePath(const std::string& filePath);
  void InitDevice(Camera* camera);
  void CommitDevice();
  void ReleaseDevice();
  void GetNumPrims(const pxr::UsdPrim& prim);
  void CollectPrims(const pxr::UsdPrim& prim);
  void Resize(int width, int height);

  void TraverseStage();
};

AMN_NAMESPACE_CLOSE_SCOPE
