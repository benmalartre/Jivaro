#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/sdf/fileFormat.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/sdf/copyUtils.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdShade/nodeGraph.h>
#include <pxr/usd/usdShade/connectableAPI.h>
#include <pxr/usd/usdExec/execNode.h>
#include <pxr/usd/usdExec/execConnectableAPI.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>

#include "../utils/strings.h"
#include "../utils/files.h"
#include "../geometry/utils.h"
#include "../command/command.h"
#include "../command/block.h"
#include "../command/inverse.h"
#include "../command/router.h"
#include "../app/application.h"
#include "../app/notice.h"
#include "../app/commands.h"
#include "../app/handle.h"
#include "../app/selection.h"

JVR_NAMESPACE_OPEN_SCOPE

//==================================================================================
// Open Scene
//==================================================================================
OpenSceneCommand::OpenSceneCommand(const std::string& filename)
  : Command(false)
{
  Application* app = GetApplication();
  if (strlen(filename.c_str()) > 0) {
     std::vector<std::string> tokens = SplitString(GetFileName(filename), ".");
    std::string name = tokens.front();
    pxr::SdfPath path("/" + name);
    UndoBlock editBlock;
    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(filename);
    app->SetStage(stage);
    UndoRouter::Get().TrackLayer(stage->GetRootLayer());
  }

  UndoInverse inverse;
  UndoRouter::Get().TransferEdits(&inverse);
  NewSceneNotice().Send();
  SceneChangedNotice().Send();
}

//==================================================================================
// New Scene
//==================================================================================
NewSceneCommand::NewSceneCommand(const std::string& filename)
  : Command(false)
{
  Application* app = GetApplication();
  pxr::SdfFileFormatConstPtr usdaFormat = pxr::SdfFileFormat::FindByExtension("usda");
  pxr::SdfLayerRefPtr layer = pxr::SdfLayer::New(usdaFormat, filename);
  if (layer) {
    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(layer);
    if (stage) {
      app->SetStage(stage);
      UndoRouter::Get().TrackLayer(stage->GetRootLayer());
    }
  }
  UndoInverse inverse;
  UndoRouter::Get().TransferEdits(&inverse);
  NewSceneNotice().Send();
  SceneChangedNotice().Send();
}

//==================================================================================
// Save Layer 
//==================================================================================
SaveLayerCommand::SaveLayerCommand(pxr::SdfLayerHandle layer)
  : Command(false)
{
  layer->Save(true);
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

//==================================================================================
// Save Layer As
//==================================================================================
SaveLayerAsCommand::SaveLayerAsCommand(pxr::SdfLayerHandle layer, const std::string& path)
  : Command(false)
{
  auto newLayer = pxr::SdfLayer::CreateNew(path);
  if (newLayer && layer) {
    newLayer->TransferContent(layer);
    newLayer->Save();
  }
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

//==================================================================================
// Reload Layer 
//==================================================================================
ReloadLayerCommand::ReloadLayerCommand(pxr::SdfLayerHandle layer)
  : Command(false)
  , _layer(layer)
{
  _layer->Reload(true);
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

//==================================================================================
// Layer Text Edit
//==================================================================================
LayerTextEditCommand::LayerTextEditCommand(pxr::SdfLayerRefPtr layer, const std::string& newText)
  : Command(true)
  , _layer(layer)
  , _newText(newText)
{
  if(!_layer)return;
  _layer->ExportToString(&_oldText);
  _layer->ImportFromString(_newText);
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

void LayerTextEditCommand::Do() {
  _layer->ExportToString(&_newText);
  _layer->ImportFromString(_oldText);
  _oldText = _newText;
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}


//==================================================================================
// Create Prim
//==================================================================================
CreatePrimCommand::CreatePrimCommand(pxr::SdfLayerRefPtr layer, const pxr::SdfPath& name) 
  : Command(true)
{
  if (!layer) return;
  UndoRouter::Get().TransferEdits(&_inverse);
  pxr::SdfPrimSpecHandle primSpec =
    pxr::SdfPrimSpec::New(layer, name.GetString(), pxr::SdfSpecifier::SdfSpecifierDef);
  layer->InsertRootPrim(primSpec);
  layer->SetDefaultPrim(primSpec->GetNameToken());
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
  std::cout << "created prim at " << primSpec->GetNameToken() << std::endl;
}

CreatePrimCommand::CreatePrimCommand(pxr::SdfPrimSpecHandle spec, const pxr::SdfPath& name)
  : Command(true)
{
  if (!spec) return;
  UndoRouter::Get().TransferEdits(&_inverse);
  SdfPrimSpec::New(spec, name.GetString(), SdfSpecifier::SdfSpecifierDef);
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
SelectCommand::SelectCommand(short type, 
  const pxr::SdfPathVector& paths, int mode)
  : Command(true)
{
  Selection* selection = GetApplication()->GetSelection();
  _previous = selection->GetSelectedPaths();
  switch (mode) {
  case SET:
    selection->Clear();
    for (auto& path : paths) selection->AddItem(path);
    break;
  case ADD:
    for (auto& path : paths) selection->AddItem(path);
    break;
  case REMOVE:
    for (auto& path : paths) selection->RemoveItem(path);
    break;
  case TOGGLE:
    for (auto& path : paths) selection->ToggleItem(path);
    break;
  }
  SelectionChangedNotice().Send();
}

void SelectCommand::Do()
{
  Selection* selection = GetApplication()->GetSelection();
  std::vector<Selection::Item> previous = selection->GetItems();
  //election->SetItems(_previous);
  //_previous = previous;
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
static void _EnsureXformCommonAPI(pxr::UsdPrim& prim, const pxr::UsdTimeCode& timeCode)
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
  const ManipTargetDescList& targets, const pxr::UsdTimeCode& timeCode)
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
  const ManipTargetDescList& targets, const pxr::UsdTimeCode& timeCode)
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
  const ManipTargetDescList& targets, const pxr::UsdTimeCode& timeCode)
  : Command(true)
{
  pxr::UsdGeomXformCache xformCache(timeCode);
  for (auto& target : targets) {
    pxr::UsdPrim prim = stage->GetPrimAtPath(target.path);
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
  const ManipTargetDescList& targets, const pxr::UsdTimeCode& timeCode)
  : Command(true)
{
  pxr::UsdGeomXformCache xformCache(timeCode);
  for (auto& target : targets) {
    pxr::UsdPrim prim = stage->GetPrimAtPath(target.path);
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
  const pxr::VtValue& value, const pxr::VtValue& previous, const pxr::UsdTimeCode& timeCode)
  : Command(true)
{
  for (auto& attribute : attributes) {
    attribute.Set(previous, timeCode);
  }
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
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

void UsdGenericCommand::Do()
{
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
  const pxr::SdfPathVector& nodes, const pxr::GfVec2f& offset)
  : Command(true)
  , _nodes(nodes)
  , _offset(offset)
{
  Application* app = GetApplication();
  pxr::UsdStageRefPtr stage = app->GetWorkStage();
  for (auto& node : nodes) {
    pxr::UsdPrim prim = stage->GetPrimAtPath(node);
    if (!prim.IsValid()) {
      continue;
    }

    pxr::UsdUINodeGraphNodeAPI api(prim);
    pxr::GfVec2f pos;
   
    pxr::UsdAttribute posAttr = api.GetPosAttr();
    if (posAttr.IsValid()) {
      posAttr.Get(&pos);
      pos += offset;
      posAttr.Set(pos);
    } else {
      posAttr = api.CreatePosAttr(pxr::VtValue(offset));
    }
  }
  UndoRouter::Get().TransferEdits(&_inverse);
  AttributeChangedNotice().Send();
}

void MoveNodeCommand::Do()
{
  _inverse.Invert();
  _offset *= -1;
  AttributeChangedNotice().Send();
}

//==================================================================================
// Expend Node
// The current expansionState of the node in the ui. 
// 'open' = fully expanded
// 'closed' = fully collapsed
// 'minimized' = should take the least space possible
//==================================================================================
ExpendNodeCommand::ExpendNodeCommand(
  const pxr::SdfPathVector& nodes, const pxr::TfToken& state)
  : Command(true)
  , _nodes(nodes)
{
  Application* app = GetApplication();
  pxr::UsdStageRefPtr stage = app->GetWorkStage();
  for (auto& node : nodes) {
    pxr::UsdUINodeGraphNodeAPI api(stage->GetPrimAtPath(node));
    pxr::UsdAttribute expandAttr = api.CreateExpansionStateAttr();
    if (!expandAttr.IsValid()) {
     expandAttr = api.CreatePosAttr(pxr::VtValue(pxr::UsdUITokens->closed));
    }
    expandAttr.Set(state);
  }
  UndoRouter::Get().TransferEdits(&_inverse);
  AttributeChangedNotice().Send();
}

void ExpendNodeCommand::Do()
{
  _inverse.Invert();
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
    pxr::UsdExecNode lhs(lhsPrim);
    pxr::UsdExecNode rhs(rhsPrim);

    pxr::UsdExecOutput output = lhs.GetOutput(source.GetNameToken());
    pxr::UsdExecInput input = rhs.GetInput(destination.GetNameToken());

    input.ConnectToSource(output);
  }
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

void ConnectNodeCommand::Do()
{
  _inverse.Invert();
  SceneChangedNotice().Send();
}



JVR_NAMESPACE_CLOSE_SCOPE