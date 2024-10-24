#include "../common.h"
#include "../embree/utils.h"
#include "../embree/instance.h"
#include "../embree/context.h"

JVR_NAMESPACE_OPEN_SCOPE

// recurse usd instance master
void RecurseMaster(
  UsdEmbreeContext* ctxt, 
  UsdEmbreeMaster* master, 
  const UsdPrim& usdPrim,
  UsdGeomXformCache* xformCache,
  RTCScene scene
)
{
  for(auto child : usdPrim.GetAllChildren())
  {
    //UsdEmbreePrim* prim = TfMapLookupPtr(ctxt->_primCacheMap, child.GetPath());
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
      if(child.IsA<UsdGeomMesh>())
      {
        GfMatrix4d worldMatrix = 
          xformCache->GetLocalToWorldTransform(child);

        UsdEmbreeSubdiv* mesh = 
          TranslateSubdiv(
            ctxt, 
            UsdGeomMesh(child), 
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
UsdEmbreeMaster* 
TranslateMaster(
  UsdEmbreeContext* ctxt, 
  const UsdPrim& usdPrim,
  UsdGeomXformCache* xformCache,
  RTCScene scene
)
{
  UsdEmbreeMaster* result = new UsdEmbreeMaster();
  result->_scene = rtcNewScene(ctxt->_device);
  ctxt->_primCacheMap[usdPrim.GetPath()] = std::move(result);

  RecurseMaster(ctxt, result, usdPrim, xformCache, result->_scene);
  rtcCommitScene (result->_scene);

  return result;

}

// translate usd instance to embree instance
UsdEmbreeInstance* TranslateInstance( 
  UsdEmbreeContext* ctxt, 
  UsdEmbreeMaster* master,
  const GfMatrix4d& worldMatrix,
  RTCScene scene
)
{
  UsdEmbreeInstance* result = new UsdEmbreeInstance();
  result->_geom = rtcNewGeometry (ctxt->_device, RTC_GEOMETRY_TYPE_INSTANCE);
  result->_color = GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);

  rtcSetGeometryInstancedScene(result->_geom, master->_scene);
  rtcSetGeometryTimeStepCount(result->_geom, 1);

  rtcAttachGeometry(scene,result->_geom);
  rtcReleaseGeometry(result->_geom);

  // set instance transform
  rtcSetGeometryTransform(
    result->_geom,
    0,
    RTC_FORMAT_FLOAT4X4_COLUMN_MAJOR,
    GfMatrix4f(worldMatrix).GetArray()
  );

  rtcCommitGeometry(result->_geom);
/*
  UsdPrim masterPrim = usdPrim.GetMaster();
  _PrimCacheMap::const_iterator it = ctxt->_primCacheMap.find(masterPrim.GetPath());
  if (it != ctxt->_primCacheMap.end()) 
  {
    UsdEmbreeMaster* master = static_cast<UsdEmbreeMaster*>(it->second);
    std::cout << "FOUND MASTER :D ===> " << master << std::endl;

    rtcSetGeometryInstancedScene(result->_geom, master->_scene);
    rtcSetGeometryTimeStepCount(result->_geom, 1);

    rtcAttachGeometry(scene,result->_geom);
    rtcReleaseGeometry(result->_geom);

    UsdEmbreeSetTransform(result, GfMatrix4d()).SetTranslate(GfVec3d(0,12,0)));
    rtcCommitGeometry(result->_geom);
  }
  else
  {
    std::cerr << "Error getting master prim !!!" << std::endl;
  }
*/
  return result;
}


JVR_NAMESPACE_CLOSE_SCOPE