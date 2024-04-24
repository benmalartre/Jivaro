#include <iostream>

#include <pxr/pxr.h>
#include <pxr/base/arch/defines.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/tf/hashMap.h>
#include <pxr/base/tf/type.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/stageCache.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usdGeom/mesh.h>


#include "../../src/common.h"
#include "../../src/utils/timer.h"
#include "../../src/geometry/mesh.h"
#include "../../src/acceleration/bvh.h"


JVR_NAMESPACE_USING_DIRECTIVE

int main (int argc, char *argv[])
{
  if(argc != 1) {
    std::cout << "you have to pass a usd filename " << std::endl;
    return -1;
  }
  std::string filename = argv[0];

  pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(filename);

  pxr::SdfPath firstMeshId;

  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  for (pxr::UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<pxr::UsdGeomMesh>()) {firstMeshId = prim.GetPath(); break;} 

  pxr::UsdPrim prim = stage->GetPrimAtPath(firstMeshId);
  if(prim.IsValid()) {
    Mesh *mesh = new Mesh(pxr::UsdGeomMesh(prim), xformCache.GetLocalToWorldTransform(prim));

    uint64_t sT = CurrentTime();
  }

  return 0;
  
}