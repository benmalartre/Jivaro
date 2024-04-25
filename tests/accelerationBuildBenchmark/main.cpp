#include <iostream>

#include <pxr/pxr.h>
#include <pxr/base/arch/defines.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/tf/hashMap.h>
#include <pxr/base/tf/type.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/ray.h>
#include <pxr/base/work/loops.h>
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
#include "../../src/acceleration/intersector.h"
#include "../../src/acceleration/bvh.h"
#include "../../src/acceleration/grid3d.h"
#include "../../src/acceleration/octree.h"

JVR_NAMESPACE_USING_DIRECTIVE



// thread task
void _FindHits(size_t begin, size_t end, const pxr::GfVec3f* positions, 
  pxr::GfVec3f* results, bool* hits, Intersector* intersector)
{
  for (size_t index = begin; index < end; ++index) {
    pxr::GfRay ray(positions[index*2], positions[index*2+1] - positions[index*2]);
    double minDistance = DBL_MAX;
    Location hit;
    if (intersector->Raycast(ray, &hit, DBL_MAX, &minDistance)) {
      Geometry* collided = intersector->GetGeometry(hit.GetGeometryIndex());
      switch (collided->GetType()) {
      case Geometry::MESH:
      {
        Mesh* mesh = (Mesh*)collided;
        Triangle* triangle = mesh->GetTriangle(hit.GetComponentIndex());
        results[index] = hit.ComputePosition(mesh->GetPositionsCPtr(), &triangle->vertices[0], 3, mesh->GetMatrix());
        break;
      }
      case Geometry::CURVE:
      {
        //Curve* curve = (Curve*)collided;
        //Edge* edge = curve->GetEdge(hit.GetElementIndex());
        //results[index] = hit.GetPosition(collided->GetPositionsCPtr(), &edge->vertices[0], 2, curve->GetMatrix());
        break;
      }
      default:
        continue;
      }
      hits[index] = true;
    } else {
      hits[index] = false;
    }
  }
}


void _Raycast(size_t numRays, Intersector* intersector, const char* title)
{
  std::cout << title << " raycast... " << std::endl;
  uint64_t sT = CurrentTime();

  pxr::VtArray<pxr::GfVec3f> positions(2*numRays);
  for(size_t p=0; p<(2*numRays);++p) 
    positions[p] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);

  pxr::VtArray<pxr::GfVec3f> points(numRays);
  pxr::VtArray<bool> hits(numRays, false);
  pxr::VtArray<pxr::GfVec3f> results(numRays);

  pxr::WorkParallelForN(numRays,
    std::bind(&_FindHits, std::placeholders::_1, 
      std::placeholders::_2, &positions[0], &points[0], &hits[0], intersector));

  // need accumulate result
  pxr::VtArray<pxr::GfVec3f> result;
  result.reserve(numRays);
  size_t numHits = 0;
  for(size_t r = 0; r < numRays; ++r) {
    if(hits[r]) {result.push_back(points[r]);numHits++;};
  }

  std::cout << title << " time : " << ((double)(CurrentTime() - sT) *1e-9) << "seconds" << std::endl;
  std::cout << title << " hits : " << numHits << std::endl;


}


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


    uint64_t sT = CurrentTime();
    BVH bvh;
    bvh.Init({mesh});
    std::cout << "bvh build took " << ((double)(CurrentTime() - sT) *1e-9) << "seconds" << std::endl;

    _Raycast(1000, &bvh, "bvh");



    sT = CurrentTime();
    Grid3D grid;
    grid.Init({mesh});
    std::cout << "grid build took " << ((double)(CurrentTime() - sT) *1e-9) << "seconds" << std::endl;

    _Raycast(1000, &grid, "grid");
 

  }

  return 0;
  
}