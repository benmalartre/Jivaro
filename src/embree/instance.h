#pragma once

#include "prim.h"
#include "mesh.h"
#include "subdiv.h"

AMN_NAMESPACE_OPEN_SCOPE

class AmnUsdEmbreeContext;

struct AmnUsdEmbreeMaster  : public AmnUsdEmbreePrim {
  RTCScene                          _scene;
  std::vector<AmnUsdEmbreePrim*>    _prims;
  std::vector<int>                  _geoms;
};

struct AmnUsdEmbreeInstance  : public AmnUsdEmbreePrim {
  AmnUsdEmbreeMaster*               _master;
  pxr::GfMatrix4d                   _xform;
  pxr::GfVec3d                      _color;
};

void RecurseMaster(
  AmnUsdEmbreeContext* ctxt, 
  AmnUsdEmbreeMaster* master, 
  const pxr::UsdPrim& usdPrim,
  pxr::UsdGeomXformCache* xformCache,
  RTCScene scene
);

AmnUsdEmbreeMaster* TranslateMaster( 
  AmnUsdEmbreeContext* ctxt, 
  const pxr::UsdPrim& usdPrim,
  pxr::UsdGeomXformCache* xformCache,
  RTCScene scene
);

AmnUsdEmbreeInstance* TranslateInstance( 
  AmnUsdEmbreeContext* ctxt, 
  AmnUsdEmbreeMaster* master,
  const pxr::GfMatrix4d& worldMatrix,
  RTCScene scene
);



AMN_NAMESPACE_CLOSE_SCOPE