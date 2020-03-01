#include "context.h"
#include "device.h"
#include "mesh.h"

namespace AMN {

  // constructor
  //----------------------------------------------------------------------------
  UsdEmbreeContext::UsdEmbreeContext():
    _scene(NULL),
    _device(NULL),
    _changed(false),
    _debug(0.f),
    _width(0),
    _height(0),
    _pixels(NULL),
    _xformCache(NULL),
    _bboxCache(NULL){};

  // destructor
  //----------------------------------------------------------------------------
  UsdEmbreeContext::~UsdEmbreeContext()
  {
    if(_bboxCache)delete _bboxCache;
    if(_xformCache)delete _xformCache;
    if(_pixels)embree::alignedFree(_pixels);
  }

  // traverse all prim range
  //----------------------------------------------------------------------------
  void UsdEmbreeContext::GetNumPrims(const pxr::UsdPrim& prim)
  {
    pxr::TfToken visibility;
    for(auto child : prim.GetAllChildren())
    {
      if(prim.IsA<pxr::UsdGeomImageable>())
      {
        pxr::UsdGeomImageable(prim).GetVisibilityAttr().Get(&visibility);
        if(visibility != pxr::UsdGeomTokens->invisible)
        {
          _numPrims++;
          GetNumPrims(child);
        }
      }
    }
  }

  // recurse collect prim
  //----------------------------------------------------------------------------
  void UsdEmbreeContext::CollectPrims( const pxr::UsdPrim& prim)
  {
    for(auto child : prim.GetAllChildren())
    {
      std::cout << child.GetPrimPath() << std::endl;
      if(child.IsA<pxr::UsdGeomXform>())
      {
        std::cout << "XFORM" << std::endl;
      }
      else if(child.IsA<pxr::UsdGeomMesh>())
      {
        std::cout << "MESH" << std::endl;
        UsdEmbreeMesh* mesh = TranslateMesh(this, pxr::UsdGeomMesh(child));
        std::cout << "EMBREE GEOM ID : " << mesh->_geomId << std::endl;
        _prims.push_back(mesh);
      }
      CollectPrims(child);
    }
  }

  // traverse stage
  //----------------------------------------------------------------------------
  void UsdEmbreeContext::TraverseStage()
  {
    _numPrims = 0;
    pxr::UsdStageRefPtr _stage = 
        pxr::UsdStage::Open(_files[0], pxr::UsdStage::LoadAll);
    
    _axis = pxr::UsdGeomGetStageUpAxis (_stage );
    if(_axis == pxr::UsdGeomTokens->y) 
      std::cerr << "### UP AXIS : Y " << std::endl;
    else if(_axis == pxr::UsdGeomTokens->z) 
      std::cerr << "### UP AXIS : Z " << std::endl;
    
    _time = pxr::UsdTimeCode::EarliestTime();

    _xformCache = new pxr::UsdGeomXformCache(_time);
    pxr::TfTokenVector includedPurposes = {pxr::UsdGeomTokens->default_,
                                          pxr::UsdGeomTokens->render};
    _bboxCache = new pxr::UsdGeomBBoxCache(_time, includedPurposes);
    GetNumPrims(_stage->GetPseudoRoot());
    _prims.reserve(_numPrims);
    CollectPrims(_stage->GetPseudoRoot());
  }

  // set file path (append to list)
  //----------------------------------------------------------------------------
  void UsdEmbreeContext::SetFilePath(const std::string& filePath)
  {
    _files.push_back(filePath);
  }

  // initialize embree device
  //----------------------------------------------------------------------------
  void UsdEmbreeContext::InitDevice()
  {
    DeviceInit();
    std::cout << "INIT DEVICE : " << _device << std::endl;
    std::cout << "INIT SCENE : " << _scene << std::endl;
  }

  void UsdEmbreeContext::CommitDevice()
  {
    CommitScene();
  }

  void UsdEmbreeContext::ReleaseDevice()
  {
    DeviceCleanup();
  }

  void UsdEmbreeContext::Resize(int width, int height)
  {
    _width = width;
    _height = height;
    if(_pixels)embree::alignedFree(_pixels);
    _pixels = (int*) embree::alignedMalloc(_width * _height * sizeof(int), 64);
  }
  
} // namespace AMN