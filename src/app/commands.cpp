#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/sdf/fileFormat.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/sdf/copyUtils.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/capsule.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/mesh.h>

#include <pxr/usd/usdShade/nodeGraph.h>
#include <pxr/usd/usdShade/connectableAPI.h>
//#include <pxr/usd/usdExec/execNode.h>
//#include <pxr/usd/usdExec/execConnectableAPI.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>

#include "../utils/strings.h"
#include "../utils/files.h"
#include "../geometry/utils.h"
#include "../geometry/geometry.h"
#include "../command/command.h"
#include "../command/block.h"
#include "../command/inverse.h"
#include "../command/router.h"
#include "../app/application.h"
#include "../app/notice.h"
#include "../app/commands.h"
#include "../app/handle.h"
#include "../app/selection.h"
#include "../app/index.h"

JVR_NAMESPACE_OPEN_SCOPE

//==================================================================================
// Open Scene
//==================================================================================
OpenSceneCommand::OpenSceneCommand(const std::string& filename)
  : Command(false)
{
  Application* app = Application::Get();
  if (strlen(filename.c_str()) > 0) {
    Model* model = app->GetModel();
    model->LoadUsdStage(filename);

    app->GetIndex()->SetStage(model->GetStage());
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
  Application* app = Application::Get();
  Model* model = app->GetModel();
  SdfFileFormatConstPtr usdaFormat = SdfFileFormat::FindByExtension("usda");
  SdfLayerRefPtr layer = SdfLayer::New(usdaFormat, filename);
  if (layer) {
    model->SetStage(UsdStage::Open(layer));
    app->GetIndex()->SetStage(model->GetStage());
  }

  UndoInverse inverse;
  UndoRouter::Get().TransferEdits(&inverse);
  NewSceneNotice().Send();
  SceneChangedNotice().Send();
}

//==================================================================================
// Save Layer 
//==================================================================================
SaveLayerCommand::SaveLayerCommand(SdfLayerHandle layer)
  : Command(false)
{
  layer->Save(true);
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

//==================================================================================
// Save Layer As
//==================================================================================
SaveLayerAsCommand::SaveLayerAsCommand(SdfLayerHandle layer, const std::string& path)
  : Command(false)
{
  auto newLayer = SdfLayer::CreateNew(path);
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
ReloadLayerCommand::ReloadLayerCommand(SdfLayerHandle layer)
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
LayerTextEditCommand::LayerTextEditCommand(SdfLayerRefPtr layer, const std::string& newText)
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
CreatePrimCommand::CreatePrimCommand(SdfLayerRefPtr layer, const SdfPath& path, const TfToken& type, 
  bool asDefault, Geometry* geometry) 
  : Command(true)
{
   if (!layer) 
    return;

  SdfPath parentPath = path.GetParentPath();
  SdfPrimSpecHandle parentSpec = layer->GetPrimAtPath(parentPath);

  if(!parentSpec)
    parentSpec = layer->GetPseudoRoot();

  SdfPrimSpecHandle primSpec =
    SdfPrimSpec::New(parentSpec, path.GetName(), SdfSpecifier::SdfSpecifierDef);
  primSpec->SetTypeName(type);

  if(asDefault)
    layer->SetDefaultPrim(primSpec->GetNameToken());

  /*if(geometry)
    _SetSpecsFromGeometry(primSpec, geometry);*/

  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

void CreatePrimCommand::Do() {
  _inverse.Invert();
  SceneChangedNotice().Send();
}


//==================================================================================
// Rename Prim
//==================================================================================
RenamePrimCommand::RenamePrimCommand(SdfLayerRefPtr layer, const SdfPath& path, const TfToken& name) 
  : Command(true)
{
  if (!layer) 
    return;

  Selection* selection = Application::Get()->GetModel()->GetSelection();
  
  SdfPrimSpecHandle primSpec = layer->GetPrimAtPath(path);
  
  SdfBatchNamespaceEdit batchEdit;
  SdfNamespaceEdit renameEdit = SdfNamespaceEdit::Rename(path, name);
  batchEdit.Add(renameEdit);
  SdfNamespaceEditDetailVector details;
  if (layer->CanApply(batchEdit, &details)) {
    layer->Apply(batchEdit);
    selection->Clear();
    selection->AddItem(path.ReplaceName(name));
  } else {
    for (const auto &detail : details) {
      std::cout << detail.edit.currentPath.GetString() << " " << detail.reason << std::endl;
    }
  }

  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

void RenamePrimCommand::Do() {
  _inverse.Invert();
  SceneChangedNotice().Send();
}

//==================================================================================
// Duplicate
//==================================================================================
DuplicatePrimCommand::DuplicatePrimCommand(SdfLayerRefPtr layer, const SdfPath& path)
  : Command(true)
{

  SdfPath destinationPath = SdfPath(path.GetString() + "_duplicate");
  SdfPrimSpecHandle sourcePrim = layer->GetPrimAtPath(path);

  UsdStagePopulationMask populationMask({ path });
  UsdStageRefPtr tmpStage =
    UsdStage::OpenMasked(layer,
      populationMask, UsdStage::InitialLoadSet::LoadAll);

  SdfLayerRefPtr sourceLayer = tmpStage->Flatten();
  SdfLayerHandle destinationLayer = layer;
  SdfPrimSpecHandle destinationPrimSpec =
    SdfCreatePrimInLayer(destinationLayer, destinationPath);
  SdfCopySpec(SdfLayerHandle(sourceLayer),
    path, destinationLayer, destinationPath);
  UndoRouter::Get().TransferEdits(&_inverse);

  Selection* selection = Application::Get()->GetModel()->GetSelection();
  selection->Clear();
  selection->AddItem(destinationPath);
  
  SceneChangedNotice().Send();
  
}

void DuplicatePrimCommand::Do() {
  Selection* selection = Application::Get()->GetModel()->GetSelection();
  std::vector<Selection::Item> current = selection->GetItems();
  _inverse.Invert();
  selection->SetItems(_selection);
  _selection = current;
  SceneChangedNotice().Send();
}

//==================================================================================
// Delete
//==================================================================================
DeletePrimCommand::DeletePrimCommand(SdfLayerRefPtr layer, const SdfPathVector& paths)
  : Command(true)
{
  Selection* selection = Application::Get()->GetModel()->GetSelection();
  _selection = selection->GetItems();

  for (auto& path : paths) {
    SdfPrimSpecHandle spec = layer->GetPrimAtPath(path);
    if (!spec) continue;

    SdfPrimSpecHandle parent = spec->GetRealNameParent();
    if (!parent) continue;
    parent->RemoveNameChild(spec);
  }
  
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

void DeletePrimCommand::Do() {
  Selection* selection = Application::Get()->GetModel()->GetSelection();
  std::vector<Selection::Item> current = selection->GetItems();
  _inverse.Invert();
  selection->SetItems(_selection);
  _selection = current;
  SceneChangedNotice().Send();
}

//==================================================================================
// Selection
//==================================================================================
SelectCommand::SelectCommand(short type, 
  const SdfPathVector& paths, int mode)
  : Command(true)
{
  Model* model = Application::Get()->GetModel();

  Selection* selection = model->GetSelection();
  _previous = selection->GetItems();
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
  const SdfPathVector affected =
    selection->ComputeAffectedPaths(model->GetStage());
  EngineRegistry::UpdateAllEnginesSelection(affected);
  SelectionChangedNotice().Send();
}

void SelectCommand::Do()
{
  Model* model = Application::Get()->GetModel();

  Selection* selection = model->GetSelection();
  std::vector<Selection::Item> current = selection->GetItems();
  selection->SetItems(_previous);
  _previous = current;
  const SdfPathVector affected =
    selection->ComputeAffectedPaths(model->GetStage());
  EngineRegistry::UpdateAllEnginesSelection(affected);
  SelectionChangedNotice().Send();
}

//==================================================================================
// Show Hide
//==================================================================================
ShowHideCommand::ShowHideCommand(SdfPathVector& paths, Mode mode)
  : Command(true)
{
  Application* app = Application::Get();
  UsdStageRefPtr stage = app->GetModel()->GetStage();
  switch (mode) {
  case SHOW:
    for (auto& path : paths) {
      UsdPrim prim = stage->GetPrimAtPath(path);
      if (prim.IsValid() && prim.IsA<UsdGeomImageable>()) {
        UsdGeomImageable imageable(prim);
        imageable.MakeVisible();
      }
    }
    break;

  case HIDE:
    for (auto& path : paths) {
      UsdPrim prim = stage->GetPrimAtPath(path);
      if (prim.IsValid() && prim.IsA<UsdGeomImageable>()) {
        UsdGeomImageable imageable(prim);
        imageable.MakeInvisible();
      }
    }
    break;

  case TOGGLE:
    for (auto& path : paths) {
      UsdPrim prim = stage->GetPrimAtPath(path);
      if (prim.IsValid() && prim.IsA<UsdGeomImageable>()) {
        UsdGeomImageable imageable(prim);
        TfToken visibility;
        imageable.GetVisibilityAttr().Get<TfToken>(&visibility);
        if (visibility == UsdGeomTokens->inherited) {
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
// Prim Activate/Deactivate
//==================================================================================
ActivatePrimCommand::ActivatePrimCommand(SdfPathVector& paths, Mode mode)
  : Command(true)
{
  Application* app = Application::Get();
  UsdStageRefPtr stage = app->GetModel()->GetStage();

  switch (mode) {
  case ACTIVATE:
    for (auto& path : paths) {
      UsdPrim prim = stage->GetPrimAtPath(path);
      if(prim.IsValid()) prim.SetActive(true);
    }
    break;

  case DEACTIVATE:
    for (auto& path : paths) {
      UsdPrim prim = stage->GetPrimAtPath(path);
      if(prim.IsValid()) prim.SetActive(false);
    }
    break;

  case TOGGLE:
    for (auto& path : paths) {
      UsdPrim prim = stage->GetPrimAtPath(path);
      if (prim.IsValid()) prim.SetActive(!prim.IsActive());
    }
    break;
  }
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

void ActivatePrimCommand::Do() {
  _inverse.Invert();
  SceneChangedNotice().Send();
}

//==================================================================================
// Translate
//==================================================================================
TranslateCommand::TranslateCommand(UsdStageRefPtr stage, 
  const ManipTargetDescList& targets, const UsdTimeCode& timeCode)
  : Command(true)
{
  for (auto& target : targets) {
    UsdPrim prim = stage->GetPrimAtPath(target.path);
    UsdGeomXformCommonAPI xformApi(prim);
   
    xformApi.SetTranslate(target.previous.translation, timeCode);
  }
  UndoRouter::Get().TransferEdits(&_inverse);

  for (auto& target : targets) {
    UsdGeomXformCommonAPI xformApi(stage->GetPrimAtPath(target.path));
    xformApi.SetTranslate(target.current.translation, timeCode);
  }
  UndoRouter::Get().TransferEdits(&_inverse);
  AttributeChangedNotice().Send();
}

void TranslateCommand::Do() 
{
  _inverse.Invert();
  AttributeChangedNotice().Send();
  SelectionChangedNotice().Send();
}

//==================================================================================
// Rotate
//==================================================================================
RotateCommand::RotateCommand(UsdStageRefPtr stage, 
  const ManipTargetDescList& targets, const UsdTimeCode& timeCode)
  : Command(true)
{
  for (auto& target: targets) {
    UsdPrim prim = stage->GetPrimAtPath(target.path);
    UsdGeomXformCommonAPI xformApi(stage->GetPrimAtPath(target.path));
    xformApi.SetRotate(target.previous.rotation, target.previous.rotOrder, timeCode);
  }
  UndoRouter::Get().TransferEdits(&_inverse);

  for (auto& target : targets) {
    UsdGeomXformCommonAPI xformApi(stage->GetPrimAtPath(target.path));
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
ScaleCommand::ScaleCommand(UsdStageRefPtr stage, 
  const ManipTargetDescList& targets, const UsdTimeCode& timeCode)
  : Command(true)
{
  UsdGeomXformCache xformCache(timeCode);
  for (auto& target : targets) {
    UsdPrim prim = stage->GetPrimAtPath(target.path);
  
    UsdGeomXformCommonAPI xformApi(prim);
    xformApi.SetScale(target.previous.scale, timeCode);
  }
  UndoRouter::Get().TransferEdits(&_inverse);

  for (auto& target : targets) {
    UsdGeomXformCommonAPI xformApi(stage->GetPrimAtPath(target.path));
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
PivotCommand::PivotCommand(UsdStageRefPtr stage, 
  const ManipTargetDescList& targets, const UsdTimeCode& timeCode)
  : Command(true)
{
  UsdGeomXformCache xformCache(timeCode);
  for (auto& target : targets) {
    UsdGeomXformCommonAPI xformApi(stage->GetPrimAtPath(target.path));
    xformApi.SetPivot(target.previous.pivot, timeCode);
  }
  UndoRouter::Get().TransferEdits(&_inverse);

  for (auto& target : targets) {
    UsdGeomXformCommonAPI xformApi(stage->GetPrimAtPath(target.path));
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
SetAttributeCommand::SetAttributeCommand(UsdAttributeVector& attributes,
  const VtValue& value, const VtValue& previous, const UsdTimeCode& timeCode)
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
CreateNodeCommand::CreateNodeCommand(
  UsdStageRefPtr stage, const std::string& name, const SdfPath& path)
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
  UsdStageRefPtr stage, const SdfPathVector& nodes, const GfVec2f& offset)
  : Command(false)
  , _nodes(nodes)
  , _offset(offset)
{
  for (auto& node : nodes) {
    UsdPrim prim = stage->GetPrimAtPath(node);
    if (!prim.IsValid()) {
      continue;
    }

    UsdUINodeGraphNodeAPI api(prim);
    GfVec2f pos;
   
    UsdAttribute posAttr = api.GetPosAttr();
    if (posAttr.IsValid()) {
      posAttr.Get(&pos);
      pos += offset;
      posAttr.Set(pos);
    } else {
      posAttr = api.CreatePosAttr(VtValue(offset));
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
// Size Node
//==================================================================================
SizeNodeCommand::SizeNodeCommand(
  UsdStageRefPtr stage, const SdfPathVector& nodes, const GfVec2f& offset)
  : Command(false)
  , _nodes(nodes)
  , _offset(offset)
{
  for (auto& node : nodes) {
    UsdPrim prim = stage->GetPrimAtPath(node);
    if (!prim.IsValid()) {
      continue;
    }

    UsdUINodeGraphNodeAPI api(prim);
    GfVec2f size;
   
    UsdAttribute sizeAttr = api.GetSizeAttr();
    if (sizeAttr.IsValid()) {
      sizeAttr.Get(&size);
      size += offset;
      sizeAttr.Set(size);
    } else {
      sizeAttr = api.CreateSizeAttr(VtValue(offset));
    }
  }
  UndoRouter::Get().TransferEdits(&_inverse);
  AttributeChangedNotice().Send();
}

void SizeNodeCommand::Do()
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
  UsdStageRefPtr stage, const SdfPathVector& nodes, const TfToken& state)
  : Command(false)
  , _nodes(nodes)
{
  for (auto& node : nodes) {
    UsdUINodeGraphNodeAPI api(stage->GetPrimAtPath(node));
    UsdAttribute expandAttr = api.CreateExpansionStateAttr();
    if (!expandAttr.IsValid()) {
     expandAttr = api.CreatePosAttr(VtValue(UsdUITokens->closed));
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
ConnectNodeCommand::ConnectNodeCommand(
  UsdStageRefPtr stage, const SdfPath& source, const SdfPath& destination)
  : Command(true)
  , _source(source)
  , _destination(destination)
{
  UsdPrim lhsPrim = stage->GetPrimAtPath(source.GetPrimPath());
  UsdPrim rhsPrim = stage->GetPrimAtPath(destination.GetPrimPath());
  if (!lhsPrim.IsValid() || !rhsPrim.IsValid()) {
    TF_WARN("[ConnectNodeCommand] Invalid attributes path provided!");
    return;
  }
  if (lhsPrim.IsA<UsdShadeShader>()) {
    UsdShadeShader lhs(lhsPrim);
    UsdShadeShader rhs(rhsPrim);

    UsdShadeOutput output = lhs.GetOutput(source.GetNameToken());
    UsdShadeInput input = rhs.GetInput(destination.GetNameToken());

    input.ConnectToSource(output);
  } /*else if (lhsPrim.IsA<UsdExecNode>()) {
    UsdExecNode lhs(lhsPrim);
    UsdExecNode rhs(rhsPrim);

    UsdExecOutput output = lhs.GetOutput(source.GetNameToken());
    UsdExecInput input = rhs.GetInput(destination.GetNameToken());

    input.ConnectToSource(output);
  }*/
  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

void ConnectNodeCommand::Do()
{
  _inverse.Invert();
  SceneChangedNotice().Send();
}

//==================================================================================
// UI Generic Command
//==================================================================================
UIGenericCommand::UIGenericCommand(CALLBACK_FN fn)
  : Command(false)
  , _fn(fn)
{
}

void UIGenericCommand::Do()
{
  _fn();
}

//==================================================================================
// Create Reference Command
//==================================================================================
CreateReferenceCommand::CreateReferenceCommand(
  UsdStageRefPtr stage, const SdfPath &path, const std::string &identifier)
  : Command(true)
  , _stage(stage)
  , _path(path)
  , _identifier(identifier)
  , _remove(false)
{
  Do();
}

void CreateReferenceCommand::Do()
{
  SdfLayerRefPtr reference = SdfLayer::FindOrOpen(_identifier);
  UsdPrim prim = _stage->GetPrimAtPath(_path);
  if(!prim.IsValid())return;
  if(_remove)
    prim.GetReferences().RemoveReference(reference->GetIdentifier());

  else
    prim.GetReferences().AddReference(reference->GetIdentifier());
  
  UndoRouter::Get().TransferEdits(&_inverse);
  _remove = !_remove;
  
  SceneChangedNotice().Send();
}


JVR_NAMESPACE_CLOSE_SCOPE