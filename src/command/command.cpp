#include "../command/command.h"
#include "../command/block.h"
#include "../command/inverse.h"
#include "../command/router.h"
#include "../app/application.h"
#include "../app/notice.h"

#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/sdf/copyUtils.h>

PXR_NAMESPACE_OPEN_SCOPE

//==================================================================================
// Open Scene
//==================================================================================
OpenSceneCommand::OpenSceneCommand(const std::string& filename)
  : Command(false)
{
  GetApplication()->OpenScene(filename);
  NewSceneNotice().Send();
  SceneChangedNotice().Send();
}


//==================================================================================
// Create Prim
//==================================================================================
CreatePrimCommand::CreatePrimCommand(pxr::UsdStageRefPtr stage, const std::string& name) 
  : Command(true)
{
  if (!stage) return;
  UndoBlock block;
  stage->DefinePrim(pxr::SdfPath::AbsoluteRootPath().AppendChild(pxr::TfToken(name)));
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

CreatePrimCommand::CreatePrimCommand(pxr::UsdPrim parent, const std::string& name)
  : Command(true)
{
  if (!parent) return;
  UndoBlock block;
  parent.GetStage()->DefinePrim(parent.GetPath().AppendChild(pxr::TfToken(name)));
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

void CreatePrimCommand::Do() {
  _inverse.Invert();
  SceneChangedNotice().Send();
}

//==================================================================================
// Duplicate
//==================================================================================
DuplicatePrimCommand::DuplicatePrimCommand(pxr::UsdStageRefPtr stage, const pxr::SdfPath& path)
  : Command(true)
{
  UndoBlock block;
  pxr::SdfPath destinationPath = pxr::SdfPath(path.GetString() + "_duplicate");
  pxr::UsdPrim sourcePrim = stage->GetPrimAtPath(path);
  pxr::SdfPrimSpecHandleVector stack = sourcePrim.GetPrimStack();

  pxr::UsdStagePopulationMask populationMask({ path });
  pxr::UsdStageRefPtr tmpStage =
    pxr::UsdStage::OpenMasked(stage->GetRootLayer(),
      populationMask, pxr::UsdStage::InitialLoadSet::LoadAll);

  pxr::SdfLayerRefPtr sourceLayer = tmpStage->Flatten();
  pxr::SdfLayerHandle destinationLayer = stage->GetEditTarget().GetLayer();
  pxr::SdfPrimSpecHandle destinationPrimSpec =
    SdfCreatePrimInLayer(destinationLayer, destinationPath);
  pxr::SdfCopySpec(pxr::SdfLayerHandle(sourceLayer),
    path, destinationLayer, destinationPath);
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

void DuplicatePrimCommand::Do() {
  _inverse.Invert();
  SceneChangedNotice().Send();
}

//==================================================================================
// Selection
//==================================================================================
SelectCommand::SelectCommand(Selection::Type type, 
  const pxr::SdfPathVector& paths, int mode)
  : Command(true)
{
  Selection* selection = GetApplication()->GetSelection();
  _previous = selection->GetItems();
  switch (mode) {
  case 0:
    selection->Clear();
    for (auto& path : paths) selection->AddItem(path);
    break;
  case 1:
    for (auto& path : paths) selection->AddItem(path);
    break;
  case 2:
    for (auto& path : paths) selection->RemoveItem(path);
    break;
  case 3:
    for (auto& path : paths) selection->ToggleItem(path);
    break;
  }
  SelectionChangedNotice().Send();
}

void SelectCommand::Do()
{
  Selection* selection = GetApplication()->GetSelection();
  std::vector<Selection::Item> previous = selection->GetItems();
  selection->SetItems(_previous);
  _previous = previous;
  SelectionChangedNotice().Send();
}

//==================================================================================
// Show Hide
//==================================================================================
ShowHideCommand::ShowHideCommand(pxr::SdfPathVector& paths, Mode mode)
  : Command(true)
{
  Application* app = GetApplication();
  pxr::UsdStageRefPtr stage = app->GetStage();
  switch (mode) {
  case SHOW:
    for (auto& path : paths) {
      pxr::UsdPrim prim = stage->GetPrimAtPath(path);
      if (prim.IsValid() && prim.IsA<pxr::UsdGeomImageable>()) {
        pxr::UsdGeomImageable imageable(prim);
        imageable.MakeVisible();
      }
    }
    break;

  case HIDE:
    for (auto& path : paths) {
      pxr::UsdPrim prim = stage->GetPrimAtPath(path);
      if (prim.IsValid() && prim.IsA<pxr::UsdGeomImageable>()) {
        pxr::UsdGeomImageable imageable(prim);
        imageable.MakeInvisible();
      }
    }
    break;

  case TOGGLE:
    for (auto& path : paths) {
      pxr::UsdPrim prim = stage->GetPrimAtPath(path);
      if (prim.IsValid() && prim.IsA<pxr::UsdGeomImageable>()) {
        pxr::UsdGeomImageable imageable(prim);
        pxr::TfToken visibility;
        imageable.GetVisibilityAttr().Get<pxr::TfToken>(&visibility);
        if (visibility == pxr::UsdGeomTokens->inherited) {
          imageable.MakeInvisible();
        }
        else {
          imageable.MakeVisible();
        }
      }
    }
    break;
  }
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

void ShowHideCommand::Do() {
  _inverse.Invert();
  SceneChangedNotice().Send();
}

//==================================================================================
// Activate Deactivate
//==================================================================================
ActivateCommand::ActivateCommand(pxr::SdfPathVector& paths, Mode mode)
  : Command(true)
{
  UndoBlock block;
  Application* app = GetApplication();
  pxr::UsdStageRefPtr stage = app->GetStage();
  switch (mode) {
  case ACTIVATE:
    for (auto& path : paths) {
      pxr::UsdPrim prim = stage->GetPrimAtPath(path);
      prim.SetActive(true);
    }
    break;

  case DEACTIVATE:
    for (auto& path : paths) {
      pxr::UsdPrim prim = stage->GetPrimAtPath(path);
      prim.SetActive(false);
    }
    break;

  case TOGGLE:
    for (auto& path : paths) {
      pxr::UsdPrim prim = stage->GetPrimAtPath(path);
      prim.SetActive(!prim.IsActive());
    }
    break;
  }
  UndoRouter::Get().TransferEdits(&_inverse);
  AttributeChangedNotice().Send();
}

void ActivateCommand::Do() {
  _inverse.Invert();
  SceneChangedNotice().Send();
}

//==================================================================================
// Transform Helper
//==================================================================================
static void _EnsureXformCommonAPI(pxr::UsdPrim& prim, pxr::UsdTimeCode& timeCode)
{
  pxr::GfVec3d translation;
  pxr::GfVec3f rotation;
  pxr::GfVec3f scale;
  pxr::GfVec3f pivot;
  pxr::UsdGeomXformCommonAPI::RotationOrder rotOrder;
  pxr::UsdGeomXformCommonAPI api(prim);
  api.GetXformVectors(&translation, &rotation, &scale, &pivot, &rotOrder, timeCode);
  pxr::UsdGeomXformable xformable(prim);
  xformable.ClearXformOpOrder();
  api.SetXformVectors(translation, rotation, scale, pivot, rotOrder, timeCode);
}

//==================================================================================
// Translate
//==================================================================================
TranslateCommand::TranslateCommand(pxr::UsdStageRefPtr stage, 
  const HandleTargetDescList& targets, pxr::UsdTimeCode& timeCode)
  : Command(true)
{
  for (auto& target : targets) {
    pxr::UsdPrim prim = stage->GetPrimAtPath(target.path);
    pxr::UsdGeomXformCommonAPI xformApi(prim);
    if (!xformApi) {
      _EnsureXformCommonAPI(prim, timeCode);
    }
    xformApi.SetTranslate(target.previous.translation, timeCode);
  }
  UndoRouter::Get().TransferEdits(&_inverse);

  for (auto& target : targets) {
    pxr::UsdGeomXformCommonAPI xformApi(stage->GetPrimAtPath(target.path));
    xformApi.SetTranslate(target.current.translation, timeCode);
  }
  UndoRouter::Get().TransferEdits(&_inverse);
  AttributeChangedNotice().Send();
}

void TranslateCommand::Do() 
{
  _inverse.Invert();
  AttributeChangedNotice().Send();
}

//==================================================================================
// Rotate
//==================================================================================
RotateCommand::RotateCommand(pxr::UsdStageRefPtr stage, 
  const HandleTargetDescList& targets, pxr::UsdTimeCode& timeCode)
  : Command(true)
{
  for (auto& target: targets) {
    pxr::UsdGeomXformCommonAPI xformApi(stage->GetPrimAtPath(target.path));
    xformApi.SetRotate(target.previous.rotation, target.previous.rotOrder, timeCode);
  }
  UndoRouter::Get().TransferEdits(&_inverse);

  UndoBlock block;
  for (auto& target : targets) {
    pxr::UsdGeomXformCommonAPI xformApi(stage->GetPrimAtPath(target.path));
    xformApi.SetRotate(target.current.rotation, target.current.rotOrder, timeCode);
  }
  UndoRouter::Get().TransferEdits(&_inverse);
  AttributeChangedNotice().Send();

}

void RotateCommand::Do()
{
  _inverse.Invert();
  AttributeChangedNotice().Send();
}

//==================================================================================
// Scale
//==================================================================================
ScaleCommand::ScaleCommand(pxr::UsdStageRefPtr stage, 
  const HandleTargetDescList& targets, pxr::UsdTimeCode& timeCode)
  : Command(true)
{
  pxr::UsdGeomXformCache xformCache(timeCode);
  for (auto& target : targets) {
    pxr:UsdPrim prim = stage->GetPrimAtPath(target.path);
    pxr::UsdGeomXformCommonAPI xformApi(prim);
    if (!xformApi) {
      _EnsureXformCommonAPI(prim, timeCode);
    }
    xformApi.SetScale(target.previous.scale, timeCode);
  }
  UndoRouter::Get().TransferEdits(&_inverse);

  for (auto& target : targets) {
    pxr::UsdGeomXformCommonAPI xformApi(stage->GetPrimAtPath(target.path));
    xformApi.SetScale(target.current.scale, timeCode);
  }

  UndoRouter::Get().TransferEdits(&_inverse);
  AttributeChangedNotice().Send();
}


void ScaleCommand::Do() {
  _inverse.Invert();
  AttributeChangedNotice().Send();
}

//==================================================================================
// Pivot
//==================================================================================
PivotCommand::PivotCommand(pxr::UsdStageRefPtr stage, 
  const HandleTargetDescList& targets, pxr::UsdTimeCode& timeCode)
  : Command(true)
{
  pxr::UsdGeomXformCache xformCache(timeCode);
  for (auto& target : targets) {
    pxr:UsdPrim prim = stage->GetPrimAtPath(target.path);
    pxr::UsdGeomXformCommonAPI xformApi(prim);
    if (!xformApi) {
      _EnsureXformCommonAPI(prim, timeCode);
    }
    xformApi.SetPivot(target.previous.pivot, timeCode);
  }
  UndoRouter::Get().TransferEdits(&_inverse);

  for (auto& target : targets) {
    pxr::UsdGeomXformCommonAPI xformApi(stage->GetPrimAtPath(target.path));
    xformApi.SetPivot(target.current.pivot, timeCode);
  }

  UndoRouter::Get().TransferEdits(&_inverse);
  AttributeChangedNotice().Send();
}

void PivotCommand::Do() {
  _inverse.Invert();
  AttributeChangedNotice().Send();
}


//==================================================================================
// Set Attribute
//==================================================================================
SetAttributeCommand::SetAttributeCommand(pxr::UsdAttributeVector& attributes,
  const pxr::VtValue& value, const pxr::UsdTimeCode& timeCode)
  : Command(true)
{
  UndoRouter::Get().TransferEdits(&_inverse);
  for (auto& attribute : attributes) {
    attribute.Set(value, timeCode);
  }
  UndoRouter::Get().TransferEdits(&_inverse);
  AttributeChangedNotice().Send();
}

void SetAttributeCommand::Do()
{
  _inverse.Invert();
  AttributeChangedNotice().Send();
}



PXR_NAMESPACE_CLOSE_SCOPE