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
  GfMatrix4d                   _xform;
};

void RecurseMaster(
  UsdEmbreeContext* ctxt, 
  UsdEmbreeMaster* master, 
  const UsdPrim& usdPrim,
  UsdGeomXformCache* xformCache,
  RTCScene scene
);

UsdEmbreeMaster* TranslateMaster( 
  UsdEmbreeContext* ctxt, 
  const UsdPrim& usdPrim,
  UsdGeomXformCache* xformCache,
  RTCScene scene
);

UsdEmbreeInstance* TranslateInstance( 
  UsdEmbreeContext* ctxt, 
  UsdEmbreeMaster* master,
  const GfMatrix4d& worldMatrix,
  RTCScene scene
);



JVR_NAMESPACE_CLOSE_SCOPE