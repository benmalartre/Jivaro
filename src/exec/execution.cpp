#include <pxr/usd/usdGeom/xform.h>
#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

void Execution::_GetRootPrim(pxr::UsdStageRefPtr& stage)
{
  // get root prim
  _rootPrim = stage->GetDefaultPrim();
  if(!_rootPrim.IsValid()) {
    pxr::UsdGeomXform root = pxr::UsdGeomXform::Define(stage, pxr::SdfPath("/Root"));
    _rootPrim = root.GetPrim();
    stage->SetDefaultPrim(_rootPrim);
  }
  _rootId = _rootPrim.GetPath();
};

JVR_NAMESPACE_CLOSE_SCOPE
