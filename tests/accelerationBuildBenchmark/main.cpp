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
#include "../../src/acceleration/intersector.h"
#include "../../src/acceleration/bvh.h"
#include "../../src/acceleration/grid3d.h"
#include "../../src/acceleration/octree.h"
#include "../../src/geometry/mesh.h"
#include "../../src/geometry/scene.h"


JVR_NAMESPACE_USING_DIRECTIVE

std::vector<Geometry*>    _meshes;
std::vector<pxr::SdfPath> _meshesId;
size_t                    _numRays = 5;

void _TraverseStageFindingMeshes(pxr::UsdStageRefPtr& stage)
{
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  for (pxr::UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      _meshes.push_back(new Mesh(pxr::UsdGeomMesh(prim), 
        xformCache.GetLocalToWorldTransform(prim)));
      _meshesId.push_back(prim.GetPath());
    }
      
}

// thread task
void _FindHits(size_t begin, size_t end, const pxr::GfVec3f* positions, 
  pxr::GfVec3f* results, bool* hits, Intersector* intersector)
{
  for (size_t index = begin; index < end; ++index) {
    pxr::GfRay ray(positions[index * 2], positions[index * 2 + 1] - positions[index * 2]);
    std::cout << "ray " << ray << std::endl;
    double minDistance = DBL_MAX;
    Location hit;
    hits[index] = false;
    if (intersector->Raycast(ray, &hit, DBL_MAX, &minDistance)) {
      Geometry* collided = intersector->GetGeometry(hit.GetGeometryIndex());
      const pxr::GfMatrix4d& matrix = collided->GetMatrix();
      switch (collided->GetType()) {
        case Geometry::MESH:
        {
          Mesh* mesh = (Mesh*)collided;
          Triangle* triangle = mesh->GetTriangle(hit.GetComponentIndex());
          results[index] = hit.ComputePosition(mesh->GetPositionsCPtr(), &triangle->vertices[0], 3, &matrix);
          hits[index] = true;
          std::cout << "hit  " << hit.GetComponentIndex() << " : " << hit.GetCoordinates() << std::endl;
        }
      }
    }
  }
}



void _Raycast(const pxr::GfVec3f* positions, Intersector* intersector, const char* title)
{
  std::cout << title << " raycast  " << _numRays << " random rays..." << std::endl;
  uint64_t sT = CurrentTime();

  pxr::VtArray<pxr::GfVec3f> points(_numRays);
  pxr::VtArray<bool> hits(_numRays, false);

  hits.resize(_numRays, false);

/*
  pxr::WorkParallelForN(_numRays,
    std::bind(&_FindHits, std::placeholders::_1, 
      std::placeholders::_2, positions, &points[0], &hits[0], intersector));*/
  _FindHits(0, _numRays, positions, &points[0], &hits[0], intersector);
  // need accumulate result
  pxr::VtArray<pxr::GfVec3f> result;
  result.reserve(_numRays);
  size_t numHits = 0;
  for(size_t p = 0; p < _numRays; ++p) {
    if(hits[p]) {result.push_back(points[p]);numHits++;};
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

  Scene scene;

  _TraverseStageFindingMeshes(stage);
  if(_meshes.size()) {

    for (size_t m = 0; m < _meshes.size(); ++m) {
      scene.AddGeometry(_meshesId[m], _meshes[m]);
      _meshes[m]->SetInputOnly();
    }

    scene.Sync(stage, 1.f);

    pxr::VtArray<pxr::GfVec3f> rays(2*_numRays);
    for(size_t p=0; p<(2*_numRays);++p) 
      rays[p] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);

    uint64_t sT = CurrentTime();
    BVH bvh;
    bvh.Init({_meshes});
    std::cout << "bvh build took " << ((double)(CurrentTime() - sT) *1e-9) << "seconds" << std::endl;

    _Raycast(&rays[0], &bvh, "bvh");

    sT = CurrentTime();
    Grid3D grid;
    grid.Init({_meshes});
    std::cout << "grid build took " << ((double)(CurrentTime() - sT) *1e-9) << "seconds" << std::endl;

    _Raycast(&rays[0], &grid, "grid");
 

  }

  return 0;
  
}