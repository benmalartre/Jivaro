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

JVR_NAMESPACE_OPEN_SCOPE

//==================================================================================
// Open Scene
//==================================================================================
OpenSceneCommand::OpenSceneCommand(const std::string& filename)
  : Command(false)
{
  Application* app = Application::Get();
  if (strlen(filename.c_str()) > 0) {
     std::vector<std::string> tokens = SplitString(GetFileName(filename), ".");
    std::string name = tokens.front();
    SdfPath path("/" + name);
    UndoBlock editBlock;
    UsdStageRefPtr stage = UsdStage::Open(filename);
    app->GetModel()->SetStage(stage);
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
  Application* app = Application::Get();
  SdfFileFormatConstPtr usdaFormat = SdfFileFormat::FindByExtension("usda");
  SdfLayerRefPtr layer = SdfLayer::New(usdaFormat, filename);
  if (layer) {
    UsdStageRefPtr stage = UsdStage::Open(layer);
    if (stage) {
      app->GetModel()->SetStage(stage);
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


static void _SetTypeNameFromType(SdfPrimSpecHandle& primSpec, short type)
{
  switch (type) {
  case Geometry::XFORM:
    primSpec->SetTypeName(TfToken("Xform"));
    break;

  case Geometry::CUBE:
    primSpec->SetTypeName(TfToken("Cube"));
    break;

  case Geometry::SPHERE:
    primSpec->SetTypeName(TfToken("Sphere"));
    break;

  case Geometry::CAPSULE:
    primSpec->SetTypeName(TfToken("Capsule"));
    break;

  case Geometry::CYLINDER:
    primSpec->SetTypeName(TfToken("Cylinder"));
    break;

  case Geometry::CONE:
    primSpec->SetTypeName(TfToken("Cone"));
    break;

  case Geometry::POINT:
    primSpec->SetTypeName(TfToken("Points"));
    break;

  case Geometry::MESH:
    primSpec->SetTypeName(TfToken("Mesh"));
    break;

  case Geometry::SOLVER:
    primSpec->SetTypeName(TfToken("PbdSolver"));
    break;

  }
}

static void _SetSpecsFromGeometry(SdfPrimSpecHandle& primSpec, Geometry* geometry )
{
  switch(geometry->GetType()) {
    case Geometry::CUBE:
      {
        //Cube* cube = Cube*(geometry);
        //primSpec->
      }
  }
}


//==================================================================================
// Create Prim
//==================================================================================
CreatePrimCommand::CreatePrimCommand(SdfLayerRefPtr layer, const SdfPath& name, short type, 
  bool asDefault, Geometry* geometry) 
  : Command(true)
{
  if (!layer) 
    return;

  SdfPrimSpecHandle primSpec =
    SdfPrimSpec::New(layer, name.GetString(), SdfSpecifier::SdfSpecifierDef);
  _SetTypeNameFromType(primSpec,  type);

  if(asDefault)
    layer->SetDefaultPrim(primSpec->GetNameToken());

  if(geometry)
    _SetSpecsFromGeometry(primSpec, geometry);

  UndoRouter::Get().TransferEdits(&_inverse);
  SceneChangedNotice().Send();
}

CreatePrimCommand::CreatePrimCommand(SdfPrimSpecHandle spec, const SdfPath& name, short type, 
  bool asDefault, Geometry* geometry)
  : Command(true)
{
  if (!spec) 
    return;

  UndoRouter::Get().TransferEdits(&_inverse);

  SdfPrimSpecHandle primSpec =
    SdfPrimSpec::New(spec, name.GetString(), SdfSpecifier::SdfSpecifierDef);
  _SetTypeNameFromType(primSpec,  type);

  if(asDefault)
    primSpec->GetLayer()->SetDefaultPrim(primSpec->GetNameToken());

  if(geometry)
    _SetSpecsFromGeometry(primSpec, geometry);

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
DuplicatePrimCommand::DuplicatePrimCommand(UsdStageRefPtr stage, const SdfPath& path)
  : Command(true)
{
  SdfPath destinationPath = SdfPath(path.GetString() + "_duplicate");
  UsdPrim sourcePrim = stage->GetPrimAtPath(path);
  SdfPrimSpecHandleVector stack = sourcePrim.GetPrimStack();

  UsdStagePopulationMask populationMask({ path });
  UsdStageRefPtr tmpStage =
    UsdStage::OpenMasked(stage->GetRootLayer(),
      populationMask, UsdStage::InitialLoadSet::LoadAll);

  SdfLayerRefPtr sourceLayer = tmpStage->Flatten();
  SdfLayerHandle destinationLayer = stage->GetEditTarget().GetLayer();
  SdfPrimSpecHandle destinationPrimSpec =
    SdfCreatePrimInLayer(destinationLayer, destinationPath);
  SdfCopySpec(SdfLayerHandle(sourceLayer),
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
DeletePrimCommand::DeletePrimCommand(UsdStageRefPtr stage, const SdfPathVector& paths)
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
  const SdfPathVector& paths, int mode)
  : Command(true)
{
  Selection* selection = Application::Get()->GetModel()->GetSelection();
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
  SelectionChangedNotice().Send();
}

void SelectCommand::Do()
{
  Selection* selection = Application::Get()->GetModel()->GetSelection();
  std::vector<Selection::Item> current = selection->GetItems();
  selection->SetItems(_previous);
  _previous = current;
  SelectionChangedNotice().Send();
}

//==================================================================================
// Show Hide
//==================================================================================
ShowHideCommand::ShowHideCommand(SdfPathVector& paths, Mode mode)
  : Command(true)
{
  Application* app = Application::Get();
  UsdStageRefPtr stage = app->GetModel()->GetWorkStage();
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
// Activate Deactivate
//==================================================================================
ActivateCommand::ActivateCommand(SdfPathVector& paths, Mode mode)
  : Command(true)
{
  Application* app = Application::Get();
  UsdStageRefPtr stage = app->GetModel()->GetStage();
  switch (mode) {
  case ACTIVATE:
    for (auto& path : paths) {
      UsdPrim prim = stage->GetPrimAtPath(path);
      prim.SetActive(true);
    }
    break;

  case DEACTIVATE:
    for (auto& path : paths) {
      UsdPrim prim = stage->GetPrimAtPath(path);
      prim.SetActive(false);
    }
    break;

  case TOGGLE:
    for (auto& path : paths) {
      UsdPrim prim = stage->GetPrimAtPath(path);
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
// Translate
//==================================================================================
TranslateCommand::TranslateCommand(UsdStageRefPtr stage, 
  const ManipTargetDescList& targets, const UsdTimeCode& timeCode)
  : Command(true)
{
  for (auto& target : targets) {
    UsdPrim prim = stage->GetPrimAtPath(target.path);
    UsdGeomXformCommonAPI xformApi(prim);
    if (!xformApi) {
      _EnsureXformCommonAPI(prim, timeCode);
    }
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
    if (!xformApi) {
      _EnsureXformCommonAPI(prim, timeCode);
    }
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
    if (!xformApi) {
      _EnsureXformCommonAPI(prim, timeCode);
    }
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
    UsdPrim prim = stage->GetPrimAtPath(target.path);
    UsdGeomXformCommonAPI xformApi(prim);
    if (!xformApi) {
      _EnsureXformCommonAPI(prim, timeCode);
    }
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
CreateNodeCommand::CreateNodeCommand(const std::string& name, const SdfPath& path)
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
  const SdfPathVector& nodes, const GfVec2f& offset)
  : Command(true)
  , _nodes(nodes)
  , _offset(offset)
{
  Application* app = Application::Get();
  UsdStageRefPtr stage = app->GetModel()->GetStage();
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
// Expend Node
// The current expansionState of the node in the ui. 
// 'open' = fully expanded
// 'closed' = fully collapsed
// 'minimized' = should take the least space possible
//==================================================================================
ExpendNodeCommand::ExpendNodeCommand(
  const SdfPathVector& nodes, const TfToken& state)
  : Command(true)
  , _nodes(nodes)
{
  Application* app = Application::Get();
  UsdStageRefPtr stage = app->GetModel()->GetStage();
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
ConnectNodeCommand::ConnectNodeCommand(const SdfPath& source, const SdfPath& destination)
  : Command(true)
  , _source(source)
  , _destination(destination)
{
  UsdStageRefPtr stage = Application::Get()->GetModel()->GetStage();
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



JVR_NAMESPACE_CLOSE_SCOPE