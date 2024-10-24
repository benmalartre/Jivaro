#include <pxr/usd/usdGeom/xform.h>

#include "../geometry/mesh.h"
#include "../tests/utils.h"
#include "../tests/push.h"

JVR_NAMESPACE_OPEN_SCOPE

void TestPush::_TraverseStageFindingMeshes(UsdStageRefPtr& stage)
{
  UsdGeomXformCache xformCache(UsdTimeCode::Default());
  for (UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<UsdGeomMesh>()) {
      std::cout << "prim " << prim.GetPath() << std::endl;
      std::cout << "xform " << xformCache.GetLocalToWorldTransform(prim) << std::endl;

      _meshes.push_back(new Mesh(UsdGeomMesh(prim), 
        xformCache.GetLocalToWorldTransform(prim)));
      _meshesId.push_back(prim.GetPath());
      _meshes.back()->SetInputOutput();

      _scene.AddGeometry(_meshesId.back(), _meshes.back());

    }
}

void TestPush::InitExec(UsdStageRefPtr& stage)
{
  if (!stage) return;

  // get root prim
  UsdPrim rootPrim = stage->GetDefaultPrim();
  if(!rootPrim.IsValid()) {
    UsdGeomXform root = UsdGeomXform::Define(stage, SdfPath("/Root"));
    rootPrim = root.GetPrim();
    stage->SetDefaultPrim(rootPrim);
  }
  const SdfPath  rootId = rootPrim.GetPath();
  _TraverseStageFindingMeshes(stage);

  UpdateExec(stage, 1);
}


void TestPush::UpdateExec(UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  const float factor = RANDOM_0_X(3);
  for(size_t m = 0; m < _meshes.size(); ++m) {
    GfVec3f* positions = _meshes[m]->GetPositionsPtr();
    const GfVec3f* normal = _meshes[m]->GetNormalsCPtr();
    for(size_t p=0; p < _meshes[m]->GetNumPoints(); ++p) {
      positions[p] += normal[p] * factor;
    }
    _scene.MarkPrimDirty(_meshesId[m], HdChangeTracker::AllDirty);
  }

}

void TestPush::TerminateExec(UsdStageRefPtr& stage)
{
  if (!stage) return;
}

JVR_NAMESPACE_CLOSE_SCOPE