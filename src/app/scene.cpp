#include "scene.h"
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>

AMN_NAMESPACE_OPEN_SCOPE

Proxy::Proxy(const pxr::UsdPrim& prim)
  : prim(prim)
{
  if(prim.IsA<pxr::UsdGeomMesh>()) {

  }
}

Proxy::~Proxy()
{
  
}

Scene::Scene()
{

}

Scene::Scene(pxr::UsdStageRefPtr& stage)
{

}

Scene::~Scene()
{

}

AMN_NAMESPACE_CLOSE_SCOPE