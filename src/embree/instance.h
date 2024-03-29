#pragma once

#include "prim.h"
#include "mesh.h"
#include "subdiv.h"

JVR_NAMESPACE_OPEN_SCOPE

class UsdEmbreeContext;

struct UsdEmbreeMaster  : public UsdEmbreePrim {
  RTCScene                          _scene;
  std::vector<UsdEmbreePrim*>       _prims;
  std::vector<int>                  _geoms;
};

struct UsdEmbreeInstance  : public UsdEmbreePrim {
  UsdEmbreeMaster*                  _master;
  pxr::GfMatrix4d                   _xform;
};

void RecurseMaster(
  UsdEmbreeContext* ctxt, 
  UsdEmbreeMaster* master, 
  const pxr::UsdPrim& usdPrim,
  pxr::UsdGeomXformCache* xformCache,
  RTCScene scene
);

UsdEmbreeMaster* TranslateMaster( 
  UsdEmbreeContext* ctxt, 
  const pxr::UsdPrim& usdPrim,
  pxr::UsdGeomXformCache* xformCache,
  RTCScene scene
);

UsdEmbreeInstance* TranslateInstance( 
  UsdEmbreeContext* ctxt, 
  UsdEmbreeMaster* master,
  const pxr::GfMatrix4d& worldMatrix,
  RTCScene scene
);



JVR_NAMESPACE_CLOSE_SCOPE