#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/sdf/copyUtils.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdShade/nodeGraph.h>
#include <pxr/usd/usdShade/connectableAPI.h>
#include <pxr/usd/usdExec/execNode.h>
#include <pxr/usd/usdExec/execConnectableAPI.h>

#include "../command/command.h"
#include "../command/block.h"
#include "../command/inverse.h"
#include "../command/router.h"
#include "../app/application.h"
#include "../app/notice.h"

PXR_NAMESPACE_OPEN_SCOPE

//==================================================================================
// Open Scene
//==================================================================================
OpenSceneCommand::OpenSceneCommand(const std::string& filename)
  : Command(false)
{
  GetApplication()->OpenScene(filename);
  UndoInverse inverse;
  UndoRouter::Get().TransferEdits(&inverse);
  NewSceneNotice().Send();
  SceneChangedNotice().Send();
}

//==================================================================================
// New Scene
//==================================================================================
NewSceneCommand::NewSceneCommand()
  : Command(false)
{
  GetApplication()->NewScene();
  UndoInverse inverse;
  UndoRouter::Get().TransferEdits(&inverse);
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
  UndoRouter::Get().TransferEdits(&_inverse);
  auto sphere = pxr::UsdGeomSphere::Define(stage, pxr::SdfPath::AbsoluteRootPath().AppendChild(pxr::TfToken(name)));
  stage->SetDefaultPrim(sphere.GetPrim());
  //stage->DefinePrim(pxr::SdfPath::AbsoluteRootPath().AppendChild(pxr::TfToken(name)));
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

CreatePrimCommand::CreatePrimCommand(pxr::UsdPrim parent, const std::string& name)
  : Command(true)
{
  if (!parent) return;
  UndoRouter::Get().TransferEdits(&_inverse);
  pxr::UsdGeomSphere::Define(parent.GetStage(), pxr::SdfPath::AbsoluteRootPath().AppendChild(pxr::TfToken(name)));
  //parent.GetStage()->DefinePrim(parent.GetPath().AppendChild(pxr::TfToken(name)));
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
// Delete
//==================================================================================
DeletePrimCommand::DeletePrimCommand(pxr::UsdStageRefPtr stage, const pxr::SdfPathVector& paths)
  : Command(true)
{
  for (auto& path : paths) {
    stage->RemovePrim(path);
  }
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

void DeletePrimCommand::Do() {
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
  pxr::UsdStageRefPtr stage = app->GetWorkStage();
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
  UndoRouter::Get().TransferEdits(&_inverse);
  Application* app = GetApplication();
  pxr::UsdStageRefPtr stage = app->GetWorkStage();
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
  SceneChangedNotice().Send();
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


//==================================================================================
// Usd Generic
//==================================================================================
UsdGenericCommand::UsdGenericCommand()
  : Command(true)
{
  std::cout << "GENRIC CREATE" << std::endl;
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

void UsdGenericCommand::Do()
{
  std::cout << "GENRIC UNDO" << std::endl;
  _inverse.Invert();
  SceneChangedNotice().Send();
}

//==================================================================================
// Create Node
//==================================================================================
CreateNodeCommand::CreateNodeCommand(const std::string& name, const pxr::SdfPath& path)
  : Command(true)
  , _name(name)
  , _path(path)
{
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

void CreateNodeCommand::Do()
{
  _inverse.Invert();
  SceneChangedNotice().Send();
}

//==================================================================================
// Move Node
//==================================================================================
MoveNodeCommand::MoveNodeCommand(
  const GraphEditorUI::NodeSet& nodes, const pxr::GfVec2f& offset)
  : Command(true)
  , _nodes(nodes)
  , _offset(offset)
{
  for (auto& node : nodes) {
    pxr::UsdUINodeGraphNodeAPI api(node->GetPrim());
    pxr::GfVec2f pos;
    api.GetPosAttr().Get(&pos);
    pos += offset;
    api.GetPosAttr().Set(pos);
  }
  UndoRouter::Get().TransferEdits(&_inverse);
}

void MoveNodeCommand::Do()
{
  for (auto& node : _nodes) {
    node->SetPosition(node->GetPosition() - _offset);
  }
  _inverse.Invert();
  _offset *= -1;
}

//==================================================================================
// Expend Node
// The current expansionState of the node in the ui. 
// 'open' = fully expanded
// 'closed' = fully collapsed
// 'minimized' = should take the least space possible
//==================================================================================
ExpendNodeCommand::ExpendNodeCommand(
  const GraphEditorUI::NodeSet& nodes, const pxr::TfToken& state)
  : Command(true)
  , _nodes(nodes)
{
  for (auto& node : nodes) {
    pxr::UsdUINodeGraphNodeAPI api(node->GetPrim());
    api.GetExpansionStateAttr().Set(state);
  }
  UndoRouter::Get().TransferEdits(&_inverse);
  AttributeChangedNotice().Send();
}

void ExpendNodeCommand::Do()
{
  std::cout << "UNDO EXPEND NODE " << std::endl;
  _inverse.Invert();
  for (auto& node : _nodes) {
    node->UpdateExpansionState();
  }
  AttributeChangedNotice().Send();
}

//==================================================================================
// Connect Node
//==================================================================================
ConnectNodeCommand::ConnectNodeCommand(const pxr::SdfPath& source, const pxr::SdfPath& destination)
  : Command(true)
  , _source(source)
  , _destination(destination)
{
  pxr::UsdStageRefPtr stage = GetApplication()->GetWorkStage();
  pxr::UsdPrim lhsPrim = stage->GetPrimAtPath(source.GetPrimPath());
  pxr::UsdPrim rhsPrim = stage->GetPrimAtPath(destination.GetPrimPath());
  if (!lhsPrim.IsValid() || !rhsPrim.IsValid()) {
    TF_WARN("[ConnectNodeCommand] Invalid attributes path provided!");
    return;
  }
  if (lhsPrim.IsA<pxr::UsdShadeShader>()) {
    pxr::UsdShadeShader lhs(lhsPrim);
    pxr::UsdShadeShader rhs(rhsPrim);

    pxr::UsdShadeOutput output = lhs.GetOutput(source.GetNameToken());
    pxr::UsdShadeInput input = rhs.GetInput(destination.GetNameToken());

    input.ConnectToSource(output);
  } else if (lhsPrim.IsA<pxr::UsdExecNode>()) {

  }
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

void ConnectNodeCommand::Do()
{
  _inverse.Invert();
  SceneChangedNotice().Send();
}



PXR_NAMESPACE_CLOSE_SCOPE