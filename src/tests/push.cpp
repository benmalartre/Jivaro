#include <pxr/usd/usdGeom/xform.h>

#include "../geometry/mesh.h"
#include "../tests/push.h"

JVR_NAMESPACE_OPEN_SCOPE

void TestPush::_TraverseStageFindingMeshes(pxr::UsdStageRefPtr& stage)
{
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  for (pxr::UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      std::cout << "prim " << prim.GetPath() << std::endl;
      std::cout << "xform " << xformCache.GetLocalToWorldTransform(prim) << std::endl;

      _meshes.push_back(new Mesh(pxr::UsdGeomMesh(prim), 
        xformCache.GetLocalToWorldTransform(prim)));
      _meshesId.push_back(prim.GetPath());
      _meshes.back()->SetInputOutput();

      _scene.AddGeometry(_meshesId.back(), _meshes.back());

    }
}

void TestPush::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  // get root prim
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  if(!rootPrim.IsValid()) {
    pxr::UsdGeomXform root = pxr::UsdGeomXform::Define(stage, pxr::SdfPath("/Root"));
    rootPrim = root.GetPrim();
    stage->SetDefaultPrim(rootPrim);
  }
  const pxr::SdfPath  rootId = rootPrim.GetPath();
std::cout << "find meshes " << time << std::endl;
  _TraverseStageFindingMeshes(stage);
  std::cout << "num meshesh : " << _meshes.size() << std::endl;
  UpdateExec(stage, 1);
}


void TestPush::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  std::cout << "update exec " << time << std::endl;
  _scene.Sync(stage, time);
  const float factor = RANDOM_0_X(3);
  for(size_t m = 0; m < _meshes.size(); ++m) {
    pxr::GfVec3f* positions = _meshes[m]->GetPositionsPtr();
    const pxr::GfVec3f* normal = _meshes[m]->GetNormalsCPtr();
    std::cout << "push " << factor << _meshesId[m] << std::endl;
    for(size_t p=0; p < _meshes[m]->GetNumPoints(); ++p) {
      positions[p] += normal[p] * factor;
    }
    _scene.MarkPrimDirty(_meshesId[m], pxr::HdChangeTracker::AllDirty);
  }

}

void TestPush::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;
}

JVR_NAMESPACE_CLOSE_SCOPE