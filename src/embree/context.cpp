#include "context.h"
#include "device.h"
#include "mesh.h"
#include "../utils/glutils.h"

PXR_NAMESPACE_OPEN_SCOPE
// constructor
//----------------------------------------------------------------------------
AmnUsdEmbreeContext::AmnUsdEmbreeContext():
  _scene(NULL),
  _device(NULL),
  _changed(false),
  _debug(0.f),
  _width(0),
  _height(0),
  _pixels(NULL),
  _xformCache(NULL),
  _bboxCache(NULL),
  _screenSpaceQuadPgm(SCREENSPACEQUAD_PROGRAM_SHADER){};

// destructor
//----------------------------------------------------------------------------
AmnUsdEmbreeContext::~AmnUsdEmbreeContext()
{
  if(_bboxCache)delete _bboxCache;
  if(_xformCache)delete _xformCache;
  if(_pixels)embree::alignedFree(_pixels);
}

// traverse all prim range
//----------------------------------------------------------------------------
void AmnUsdEmbreeContext::GetNumPrims(const pxr::UsdPrim& prim)
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
void AmnUsdEmbreeContext::CollectPrims( const pxr::UsdPrim& prim)
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
      AmnUsdEmbreeMesh* mesh = TranslateMesh(this, pxr::UsdGeomMesh(child));
      std::cout << "EMBREE GEOM ID : " << mesh->_geomId << std::endl;
      _prims.push_back(mesh);
    }
    CollectPrims(child);
  }
}

// traverse stage
//----------------------------------------------------------------------------
void AmnUsdEmbreeContext::TraverseStage()
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
void AmnUsdEmbreeContext::SetFilePath(const std::string& filePath)
{
  _files.push_back(filePath);
}

// initialize embree device
//----------------------------------------------------------------------------
void AmnUsdEmbreeContext::InitDevice()
{
  DeviceInit();
  std::cout << "INIT DEVICE : " << _device << std::endl;
  std::cout << "INIT SCENE : " << _scene << std::endl;
}

void AmnUsdEmbreeContext::CommitDevice()
{
  CommitScene();
}

void AmnUsdEmbreeContext::ReleaseDevice()
{
  DeviceCleanup();
}

void AmnUsdEmbreeContext::Resize(int width, int height)
{
  _width = width;
  _height = height;
  if(_pixels)embree::alignedFree(_pixels);
  _pixels = (int*) embree::alignedMalloc(_width * _height * sizeof(int), 64);
}

PXR_NAMESPACE_CLOSE_SCOPE