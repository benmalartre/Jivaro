#pragma once

#include "default.h"
#include "prim.h"
#include "utils.h"
#include "../app/camera.h"

AMN_NAMESPACE_OPEN_SCOPE

struct UsdEmbreeMaster;
typedef 
pxr::TfHashMap< pxr::SdfPath, UsdEmbreePrim*, pxr::SdfPath::Hash > _PrimCacheMap;

struct UsdEmbreeContext {
  // usd
  std::vector<std::string>                        _files;
  pxr::UsdStageRefPtr                             _stage;
  std::vector<UsdEmbreePrim*>                     _prims;
  std::vector<UsdEmbreeMaster*>                   _masters;
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
  int                                             _screenSpaceQuadPgm;

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


 /*  HOW TO USE THIS SHIT...
  _context->Resize(_viewport->GetWidth(), _viewport->GetHeight());
  _context->SetFilePath(filename);
  _context->InitDevice(_viewport->GetCamera());
  _context->TraverseStage();
  _context->CommitDevice();
  
  std::string imageDirectory = "/Users/benmalartre/Documents/RnD/amnesie/images";
  int imageId = NumFilesInDirectory(imageDirectory.c_str()) + 1;
  std::string imagePath = imageDirectory + "/img.";
  std::string imageExt = ".jpg";
  embree::FileName outputImageFilename(imagePath + std::to_string(imageId) + imageExt);

  _viewport->GetCamera()->ComputeFrustum();

  RenderToFile(outputImageFilename, _viewport->GetCamera(), 2048, 1024);
  RenderToMemory(_viewport->GetCamera());
  _viewport->SetContext(_context);
*/

/*
  if(_mode == EMBREE && _context)
  {
    
    _context->Resize(_parent->GetWidth(), _parent->GetHeight());
    _camera->SetWindow(
      _parent->GetX(),
      _parent->GetY(),
      _parent->GetWidth(),
      _parent->GetHeight()
    );
    
    RenderToMemory(_camera, false);
    SetImage();
    
  }   
*/