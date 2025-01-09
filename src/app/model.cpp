#include <iostream>
#include <string>

#include "../utils/files.h"
#include "../utils/timer.h"
#include "../utils/prefs.h"
#include "../ui/popup.h"
#include "../command/manager.h"
#include "../command/router.h"
#include "../geometry/scene.h"
#include "../app/model.h"
#include "../app/notice.h"
#include "../app/handle.h"
#include "../app/engine.h"
#include "../app/selection.h"
#include "../app/window.h"
#include "../app/view.h"
#include "../app/time.h"
#include "../app/commands.h"
#include "../app/camera.h"
#include "../app/tools.h"
#include "../app/application.h"

#include "../tests/grid.h"
#include "../tests/raycast.h"
#include "../tests/particles.h"
#include "../tests/pbd.h"
#include "../tests/hair.h"
#include "../tests/bvh.h"
#include "../tests/points.h"
#include "../tests/instancer.h"
#include "../tests/velocity.h"
#include "../tests/pendulum.h"
#include "../tests/geodesic.h"
#include "../tests/push.h"


JVR_NAMESPACE_OPEN_SCOPE


// constructor
//----------------------------------------------------------------------------
Model::Model()
{  
  SetEmptyStage();
  
};

// destructor
//----------------------------------------------------------------------------
Model::~Model()
{
};

void
Model::SetStage(UsdStageRefPtr& stage)
{
  _stage = stage;
  _rootLayer = stage->GetRootLayer();
  _sessionLayer = _stage->GetSessionLayer();
  _stage->SetEditTarget(_rootLayer);

  UndoRouter::Get().TrackLayer(_rootLayer);
  UndoRouter::Get().TrackLayer(_sessionLayer);
}


void 
Model::SetEmptyStage()
{
  _rootLayer = SdfLayer::CreateAnonymous();
  _sessionLayer = SdfLayer::CreateAnonymous();
  _stage = UsdStage::Open(_rootLayer, _sessionLayer);
  _stage->SetEditTarget(_rootLayer);

  UsdGeomSetStageUpAxis(_stage, UsdGeomTokens->y);

  UndoRouter::Get().TrackLayer(_rootLayer);
  UndoRouter::Get().TrackLayer(_sessionLayer);
}

void 
Model::LoadUsdStage(const std::string usdFilePath)
{
  _rootLayer = SdfLayer::FindOrOpen(usdFilePath);
  _sessionLayer = SdfLayer::CreateAnonymous();
  _stage = UsdStage::Open(_rootLayer, _sessionLayer);
  _stage->SetEditTarget(_rootLayer);
  UndoRouter::Get().TrackLayer(_rootLayer);
  UndoRouter::Get().TrackLayer(_sessionLayer);
}

UsdPrim 
Model::GetUsdPrim(SdfPath path)
{
  return _stage->GetPrimAtPath(path);
}

UsdPrimRange 
Model::GetAllPrims()
{
  return _stage->Traverse();
}


// get current layer
SdfLayerRefPtr
Model::GetSessionLayer()
{
  return _sessionLayer;
}

// get root layer
SdfLayerRefPtr
Model::GetRootLayer()
{
  return _rootLayer;
}

// selection
void 
Model::SetSelection(const SdfPathVector& selection)
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, selection, SelectCommand::SET);
}

void
Model::ToggleSelection(const SdfPathVector& selection)
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, selection, SelectCommand::TOGGLE);
}

void 
Model::AddToSelection(const SdfPathVector& paths)
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, paths, SelectCommand::ADD);
}

void 
Model::RemoveFromSelection(const SdfPathVector& paths)
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, paths, SelectCommand::REMOVE);
}

void 
Model::ClearSelection()
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, {}, SelectCommand::SET);
}

GfBBox3d
Model::GetStageBoundingBox()
{
  GfBBox3d bbox;
  TfTokenVector purposes = { UsdGeomTokens->default_ };
  UsdGeomBBoxCache bboxCache(
    UsdTimeCode(Time::Get()->GetActiveTime()), purposes, false, false);
  return bboxCache.ComputeWorldBound(_stage->GetPseudoRoot());
}

GfBBox3d 
Model::GetSelectionBoundingBox()
{
  GfBBox3d bbox;
  static TfTokenVector purposes = {
    UsdGeomTokens->default_,
    UsdGeomTokens->proxy,
    UsdGeomTokens->guide,
    UsdGeomTokens->render
  };
  UsdGeomBBoxCache bboxCache(
    UsdTimeCode(Time::Get()->GetActiveTime()), purposes, false, false);
  for (size_t n = 0; n < _selection.GetNumSelectedItems(); ++n) {
    const Selection::Item& item = _selection[n];
    if (item.type == Selection::Type::PRIM) {
      UsdPrim prim = _stage->GetPrimAtPath(item.path);
      
      if (prim.IsActive()) {
        const GfBBox3d primBBox = bboxCache.ComputeWorldBound(prim);
        bbox = bbox.Combine(bbox, GfBBox3d(primBBox.ComputeAlignedRange()));
      }
        
    }
    else if (item.type == Selection::Type::VERTEX) {

    }
    else if (item.type == Selection::Type::EDGE) {

    }
    else if (item.type == Selection::Type::FACE) {

    }
  }

  /*
  UsdPrim& prim = _stage->GetPrimAtPath(SdfPath("/Cube"));
  if (!prim.IsValid()) {
    prim = UsdGeomCube::Define(_stage, SdfPath("/Cube")).GetPrim();
    UsdGeomCube cube(prim);
    cube.CreateSizeAttr().Set(VtValue(1.0));
   
    VtArray<TfToken> xformOpOrderTokens =
    { TfToken("xformOp:scale"), TfToken("xformOp:translate")};
    cube.CreateXformOpOrderAttr().Set(VtValue(xformOpOrderTokens));
   
  }
  UsdGeomCube cube(prim);

  bool resetXformStack = false;
  bool foundScaleOp = false;
  bool foundTranslateOp = false;
  std::vector<UsdGeomXformOp> xformOps = cube.GetOrderedXformOps(&resetXformStack);
  for (auto& xformOp : xformOps) {
 
    if (xformOp.GetName() == TfToken("xformOp:scale")) {
      GfRange3d bboxRange = bbox.GetRange();
      xformOp.Set(VtValue(GfVec3f(bboxRange.GetMax() - bboxRange.GetMin())));
      foundScaleOp = true;
    } else if(xformOp.GetName() == TfToken("xformOp:translate")) {
      GfVec3d center = bbox.ComputeCentroid();
      xformOp.Set(VtValue(center));
      foundTranslateOp = true;
    }
  }
  if (!foundScaleOp) {
    UsdGeomXformOp scaleOp = cube.AddScaleOp();
    GfRange3d bboxRange = bbox.GetRange();
    scaleOp.Set(VtValue(GfVec3f(bboxRange.GetMax() - bboxRange.GetMin())));
  }
  if (!foundTranslateOp) {
    UsdGeomXformOp translateOp = cube.AddTranslateOp();
    GfVec3d center = bbox.ComputeCentroid();
    translateOp.Set(VtValue(center));
  }
  */
  return bbox;
}

JVR_NAMESPACE_CLOSE_SCOPE
