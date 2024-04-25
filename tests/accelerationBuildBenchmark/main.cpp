#include <iostream>

#include <pxr/pxr.h>
#include <pxr/base/arch/defines.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/tf/hashMap.h>
#include <pxr/base/tf/type.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/ray.h>
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
#include "../../src/acceleration/grid3d.h"
#include "../../src/acceleration/octree.h"

JVR_NAMESPACE_USING_DIRECTIVE


int main (int argc, char *argv[])
{
  if(argc != 2) {
    std::cout << "you have to pass a usd filename " << std::endl;
    return -1;
  }
  std::string filename = argv[1];

  pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(filename);

  pxr::SdfPath firstMeshId;

  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  for (pxr::UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<pxr::UsdGeomMesh>()) {firstMeshId = prim.GetPath(); break;} 

  pxr::UsdPrim prim = stage->GetPrimAtPath(firstMeshId);
  if(prim.IsValid()) {
    Mesh *mesh = new Mesh(pxr::UsdGeomMesh(prim), xformCache.GetLocalToWorldTransform(prim));

    Location gridHit, bvhHit;
    double gridDistance = DBL_MAX;
    double bvhDistance = DBL_MAX;

    size_t N = 120;
    std::vector<pxr::GfRay> rays(N);
    for(size_t x=0; x < N; ++x) {
      rays[x].SetEnds(pxr::GfVec3f(0.f), pxr::GfVec3f(RANDOM_0_1 - 0.5f, RANDOM_0_1 - 0.5f, RANDOM_0_1 - 0.5f));
    }

    std::vector<pxr::GfVec3f> bvhItsec(N);
    std::vector<pxr::GfVec3f> gridItsec(N);

    uint64_t sT = CurrentTime();
    BVH bvh;
    bvh.Init({mesh});
    std::cout << "bvh build took " << ((double)(CurrentTime() - sT) *1e-9) << "seconds" << std::endl;

    std::cout << "bvh raycast... ";
    sT = CurrentTime();
    for(size_t x = 0; x< N; ++x) {
      if (bvh.Raycast(rays[x], &bvhHit, DBL_MAX, &bvhDistance)) {
        bvhItsec[x] = bvhHit.ComputePosition(mesh->GetPositionsCPtr(), 
         &(mesh->GetTriangle(bvhHit.GetComponentIndex())->vertices)[0], 3, mesh->GetMatrix());
      }
    }
    std::cout << "bvh raycast took " << ((double)(CurrentTime() - sT) *1e-9) << "seconds" << std::endl;

    sT = CurrentTime();
    Grid3D grid;
    grid.Init({mesh});
    std::cout << "grid build took " << ((double)(CurrentTime() - sT) *1e-9) << "seconds" << std::endl;
    
    std::cout << "grid raycast... ";
    sT = CurrentTime();
    for(size_t x = 0; x< N; ++x) {
      if (grid.Raycast(rays[x], &gridHit, DBL_MAX, &gridDistance)) {
          gridItsec[x] = gridHit.ComputePosition(mesh->GetPositionsCPtr(), 
          &(mesh->GetTriangle(gridHit.GetComponentIndex())->vertices)[0], 3, mesh->GetMatrix());
      }
    }
    std::cout << "grid raycast took " << ((double)(CurrentTime() - sT) *1e-9) << "seconds" << std::endl;

    for(size_t x = 0; x < N; ++x) {
      if((bvhItsec[x] - gridItsec[x]).GetLength() > 0.000001f) {
        std::cout << x << " problematic " << std::endl;
        std::cout << bvhItsec[x] << " vs " << gridItsec[x] << std::endl;
        std::cout << rays[x] << std::endl;
      }
    }


  }

  return 0;
  
}