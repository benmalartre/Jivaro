#include <pxr/usd/usdGeom/xform.h>

#include "../geometry/mesh.h"
#include "../tests/push.h"

JVR_NAMESPACE_OPEN_SCOPE

void TestPush::_TraverseStageFindingMeshes(pxr::UsdStageRefPtr& stage)
{
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  for (pxr::UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      _meshes.push_back(new Mesh(pxr::UsdGeomMesh(prim), 
        xformCache.GetLocalToWorldTransform(prim)));
      _baseMeshes.push_back(new Mesh(*_meshes.back()));
      _meshesId.push_back(prim.GetPath());
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

  _TraverseStageFindingMeshes(stage);
  
}


void TestPush::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);

}

void TestPush::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;
}

JVR_NAMESPACE_CLOSE_SCOPE