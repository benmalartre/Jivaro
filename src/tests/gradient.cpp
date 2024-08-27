#include "../geometry/sampler.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/voxels.h"
#include "../geometry/points.h"
#include "../geometry/implicit.h"
#include "../geometry/instancer.h"
#include "../geometry/triangle.h"
#include "../geometry/utils.h"
#include "../geometry/scene.h"

#include "../tests/utils.h"
#include "../tests/gradient.h"

#include "../app/application.h"

JVR_NAMESPACE_OPEN_SCOPE

void TestGradient::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  _GetRootPrim(stage);

  _TraverseStageFindingMeshes(stage);

  _Initialize(stage);

}


void TestGradient::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
}

void TestGradient::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;
  _Terminate(stage);
}

size_t TestGradient::_TraverseStageFindingMeshes(pxr::UsdStageRefPtr& stage)
{
  size_t numMeshes = 0;
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  for (pxr::UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      _meshes.push_back(new Mesh(pxr::UsdGeomMesh(prim), 
        xformCache.GetLocalToWorldTransform(prim)));
      _meshesId.push_back(prim.GetPath());
      numMeshes++;
    }
  return numMeshes;
      
}


void TestGradient::_Initialize(pxr::UsdStageRefPtr& stage) 
{
  pxr::VtArray<pxr::GfVec3f> positions;
  pxr::VtArray<float> widths;
  pxr::VtArray<pxr::GfVec3f> colors;

  // create bvh
  if (_meshes.size()) {
    _bvh.Init(_meshes);

    for(size_t m = 0; m < _meshes.size();++m) {
      _scene.AddGeometry(_meshesId[m], _meshes[m]);
      _meshes[m]->SetInputOnly();
    }

    /*
    _bvhId = _rootId.AppendChild(pxr::TfToken("bvh"));
    _leaves = _SetupBVHInstancer(stage, _bvhId, &_bvh);
    _scene.AddGeometry(_bvhId, (Geometry*)_leaves );
    _scene.MarkPrimDirty(_bvhId, pxr::HdChangeTracker::DirtyInstancer);
    */

    pxr::GfVec3f bmin(_bvh.GetMin());
    pxr::GfVec3f bmax(_bvh.GetMax());

    pxr::GfVec3f size(_bvh.GetSize());

    float radius = pxr::GfMin(size[0], size[1], size[2]) * 0.1f;

    pxr::GfVec3f dimensions(
      pxr::GfCeil(size[0] / radius),
      pxr::GfCeil(size[1] / radius),
      pxr::GfCeil(size[2] / radius)
    );

    size = dimensions * radius;

    pxr::GfVec3f step(
      size[0] / dimensions[0], 
      size[1] / dimensions[1], 
      size[2] / dimensions[2]);
    pxr::GfVec3f offset = size * -0.5f;

    size_t total = dimensions[0] * dimensions[1] * dimensions[2];
    positions.resize(total);
    widths.resize(total);
    colors.resize(total);

    size_t index;
    for(size_t z = 0; z < dimensions[2]; ++z) 
      for(size_t y = 0; y < dimensions[1]; ++y)
        for(size_t x = 0; x < dimensions[0]; ++x) {
          index = z * dimensions[0] * dimensions[1] + y * dimensions[0] + x;

          positions[index] = pxr::GfVec3f(  
            x * step[0] + offset[0], 
            y * step[1] + offset[1],
            z * step[2] + offset[2]
          );
          widths[index] = 0.1;
          colors[index] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
        }
  }

  _points= new Points();

  _points->SetPositions(positions);
  _points->SetWidths(widths);
  _points->SetColors(colors);

  _pointsId = _rootId.AppendChild(pxr::TfToken("pendulum"));
  _scene.AddGeometry(_pointsId, _points);


}

void TestGradient::_Terminate(pxr::UsdStageRefPtr& stage)
{
  _scene.Remove(_pointsId);
}

JVR_NAMESPACE_CLOSE_SCOPE