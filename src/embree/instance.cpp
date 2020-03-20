#include "../default.h"
#include "../utils/utils.h"
#include "utils.h"
#include "instance.h"
#include "context.h"

AMN_NAMESPACE_OPEN_SCOPE

// recurse usd instance master
void RecurseMaster(
  AmnUsdEmbreeContext* ctxt, 
  AmnUsdEmbreeMaster* master, 
  const pxr::UsdPrim& usdPrim,
  pxr::UsdGeomXformCache* xformCache,
  RTCScene scene
)
{
  for(auto child : usdPrim.GetAllChildren())
  {
    //AmnUsdEmbreePrim* prim = TfMapLookupPtr(ctxt->_primCacheMap, child.GetPath());
    _PrimCacheMap::const_iterator it = ctxt->_primCacheMap.find(child.GetPath());
    if (it != ctxt->_primCacheMap.end()) 
    {
        master->_prims.push_back(it->second);
        /*
        instance->_geometries.push_back(
          rtcAttachGeometry(instance->_scene, it->second->_geom)
        );
        */
    }
    else
    {
      if(child.IsA<pxr::UsdGeomMesh>())
      {
        pxr::GfMatrix4d worldMatrix = 
          xformCache->GetLocalToWorldTransform(child);

        AmnUsdEmbreeSubdiv* mesh = 
          TranslateSubdiv(
            ctxt, 
            pxr::UsdGeomMesh(child), 
            worldMatrix, 
            scene
          );

        master->_prims.push_back(mesh);
      }
    }

    RecurseMaster(ctxt, master, child, xformCache, scene);
  }
}

// translate usd master to embree master
AmnUsdEmbreeMaster* 
TranslateMaster(
  AmnUsdEmbreeContext* ctxt, 
  const pxr::UsdPrim& usdPrim,
  pxr::UsdGeomXformCache* xformCache,
  RTCScene scene
)
{
  AmnUsdEmbreeMaster* result = new AmnUsdEmbreeMaster();
  result->_scene = rtcNewScene(ctxt->_device);
  ctxt->_primCacheMap[usdPrim.GetPath()] = std::move(result);

  RecurseMaster(ctxt, result, usdPrim, xformCache, result->_scene);
  rtcCommitScene (result->_scene);

  return result;

}

// translate usd instance to embree instance
AmnUsdEmbreeInstance* TranslateInstance( 
  AmnUsdEmbreeContext* ctxt, 
  AmnUsdEmbreeMaster* master,
  const pxr::GfMatrix4d& worldMatrix,
  RTCScene scene
)
{
  AmnUsdEmbreeInstance* result = new AmnUsdEmbreeInstance();
  result->_geom = rtcNewGeometry (ctxt->_device, RTC_GEOMETRY_TYPE_INSTANCE);
  result->_color = pxr::GfVec3d(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);

  rtcSetGeometryInstancedScene(result->_geom, master->_scene);
  rtcSetGeometryTimeStepCount(result->_geom, 1);

  rtcAttachGeometry(scene,result->_geom);
  rtcReleaseGeometry(result->_geom);

  // set instance transform
  rtcSetGeometryTransform(
    result->_geom,
    0,
    RTC_FORMAT_FLOAT4X4_COLUMN_MAJOR,
    pxr::GfMatrix4f(worldMatrix).GetArray()
  );

  rtcCommitGeometry(result->_geom);
/*
  pxr::UsdPrim masterPrim = usdPrim.GetMaster();
  _PrimCacheMap::const_iterator it = ctxt->_primCacheMap.find(masterPrim.GetPath());
  if (it != ctxt->_primCacheMap.end()) 
  {
    AmnUsdEmbreeMaster* master = static_cast<AmnUsdEmbreeMaster*>(it->second);
    std::cout << "FOUND MASTER :D ===> " << master << std::endl;

    rtcSetGeometryInstancedScene(result->_geom, master->_scene);
    rtcSetGeometryTimeStepCount(result->_geom, 1);

    rtcAttachGeometry(scene,result->_geom);
    rtcReleaseGeometry(result->_geom);

    AmnUsdEmbreeSetTransform(result, pxr::GfMatrix4d(1).SetTranslate(pxr::GfVec3d(0,12,0)));
    rtcCommitGeometry(result->_geom);
  }
  else
  {
    std::cerr << "Error getting master prim !!!" << std::endl;
  }
*/
  return result;
}


AMN_NAMESPACE_CLOSE_SCOPE