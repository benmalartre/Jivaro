#include "../utils/glutils.h"
#include "context.h"
#include "device.h"
#include "mesh.h"
#include "subdiv.h"
#include "instance.h"


JVR_NAMESPACE_OPEN_SCOPE
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
  _lowPixels(NULL),
  _xformCache(NULL),
  _bboxCache(NULL),
  _screenSpaceQuadPgm(0){};

// destructor
//----------------------------------------------------------------------------
UsdEmbreeContext::~UsdEmbreeContext()
{
  //if(_bboxCache)delete _bboxCache;
  //if(_xformCache)delete _xformCache;
  if(_pixels)embree::alignedFree(_pixels);
  if(_lowPixels)embree::alignedFree(_lowPixels);
}

// traverse all prim range
//----------------------------------------------------------------------------
void UsdEmbreeContext::GetNumPrims(const UsdPrim& prim)
{
  TfToken visibility;
  for(auto child : prim.GetAllChildren())
  {
    if(prim.IsA<UsdGeomImageable>())
    {
      UsdGeomImageable(prim).GetVisibilityAttr().Get(&visibility);
      if(visibility != UsdGeomTokens->invisible)
      {
        _numPrims++;
        GetNumPrims(child);
      }
    }
  }
}

// recurse collect prim
//----------------------------------------------------------------------------
void UsdEmbreeContext::CollectPrims( const UsdPrim& prim)
{
  bool flip = false;
  std::string path = prim.GetPath().GetString();
  std::string search = "/SHA256/stage/manekineko2_grp";
  
  
  if(path == search)
  {
    GfMatrix4d worldMatrix = 
        _xformCache->GetLocalToWorldTransform(prim);
    UsdEmbreeMaster* master = 
      TranslateMaster(this, prim, _xformCache, _scene);
    _masters.push_back(master);

    int N = 16;
    float step = 360 /(float)N;
    float radius = 32;
    for(int i=0; i<N; ++i)
    {
      float ta = i * step * DEGREES_TO_RADIANS;
      UsdEmbreeInstance* instance = 
      TranslateInstance(
        this, 
        master, 
        GfMatrix4d().SetTranslate(
          GfVec3d(sin(ta) * radius, 6, -cos(ta) * radius)
        ),
        _scene
      );
      _prims.push_back(instance);
    }
  }
  
  if(path.find(search) == std::string::npos)flip = true;

  for(auto child : prim.GetAllChildren())
  {
    std::cout << child.GetPrimPath() << std::endl;
    if(child.IsA<UsdGeomXform>())
    {
      std::cout << "XFORM" << std::endl;
    }
    else if(child.IsA<UsdGeomMesh>())
    {
      GfMatrix4d worldMatrix = 
        _xformCache->GetLocalToWorldTransform(child);
      
      if(flip)
      {
        UsdEmbreeSubdiv* subdiv = 
        TranslateSubdiv(this, UsdGeomMesh(child), worldMatrix, _scene);
        _prims.push_back(subdiv);
      }
      else
      {
        UsdEmbreeMesh* mesh = 
        TranslateMesh(this, UsdGeomMesh(child), worldMatrix, _scene);
        _prims.push_back(mesh);
      }
    }
    CollectPrims(child);
  }
  
  
}

// traverse stage
//----------------------------------------------------------------------------
void UsdEmbreeContext::TraverseStage()
{
  _numPrims = 0;
  UsdStageRefPtr _stage = 
      UsdStage::Open(_files[0], UsdStage::LoadAll);
  
  _axis = UsdGeomGetStageUpAxis (_stage );
  if(_axis == UsdGeomTokens->y) 
    std::cerr << "### UP AXIS : Y " << std::endl;
  else if(_axis == UsdGeomTokens->z) 
    std::cerr << "### UP AXIS : Z " << std::endl;
  
  _time = UsdTimeCode::EarliestTime();

  UsdGeomXformCache xformCache(_time);
  _xformCache = &xformCache;
  TfTokenVector includedPurposes = {UsdGeomTokens->default_,
                                        UsdGeomTokens->render};
  UsdGeomBBoxCache bboxCache(_time, includedPurposes);
  _bboxCache = &bboxCache;

  /*
  _xformCache = new UsdGeomXformCache(_time);
  TfTokenVector includedPurposes = {UsdGeomTokens->default_,
                                        UsdGeomTokens->render};
  _bboxCache = new UsdGeomBBoxCache(_time, includedPurposes);
  */
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
void UsdEmbreeContext::InitDevice(Camera* camera)
{
  DeviceInit(camera);
}

void UsdEmbreeContext::CommitDevice()
{
  CommitScene();
}

void UsdEmbreeContext::ReleaseDevice()
{
  DeviceCleanup();
}

// resize (realocate memory)
//----------------------------------------------------------------------------
void UsdEmbreeContext::Resize(int width, int height)
{
  _width = width;
  _height = height;
  if(_pixels)embree::alignedFree(_pixels);
  if(_lowPixels)embree::alignedFree(_lowPixels);
  _pixels = (int*) embree::alignedMalloc(_width * _height * sizeof(int), 64);
  _lowPixels = 
    (int*) embree::alignedMalloc(
      (_width>>4) * (_height>>4) * sizeof(int), 64
    );
}

JVR_NAMESPACE_CLOSE_SCOPE