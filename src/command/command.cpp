#include "../command/command.h"
#include "../app/application.h"
#include "../app/notice.h"
#include <pxr/usd/usdGeom/mesh.h>

PXR_NAMESPACE_OPEN_SCOPE

//==================================================================================
// Open Scene
//==================================================================================
OpenSceneCommand::OpenSceneCommand(const std::string& filename)
  : Command(false)
  , _filename(filename)
{
}

void OpenSceneCommand::Execute()
{
  GetApplication()->OpenScene(_filename);
  NewSceneNotice().Send();
}


//==================================================================================
// Create Prim
//==================================================================================
CreatePrimCommand::CreatePrimCommand(pxr::UsdStageRefPtr stage, const std::string& name) 
  : Command(true)
  , _parent()
  , _stage(stage)
  , _name(name)
{
}

CreatePrimCommand::CreatePrimCommand(pxr::UsdPrim parent, const std::string& name)
  : Command(true)
  , _parent(parent)
  , _stage()
  , _name(name)
{
}

void CreatePrimCommand::Execute() 
{
  Redo();
}

void CreatePrimCommand::Undo()
{
  if (!_prim) return;
  if (_stage) {
    _stage->RemovePrim(_prim.GetPath());
  } else {
    _prim.GetStage()->RemovePrim(_prim.GetPath());
  }
  SceneChangedNotice().Send();
}

void CreatePrimCommand::Redo() {
  if (!_stage && !_parent) return;
  if (_stage) {
    _prim = _stage->DefinePrim(pxr::SdfPath::AbsoluteRootPath().AppendChild(_name));
  }
  else if(_parent) {
    _prim = _parent.GetStage()->DefinePrim(_parent.GetPath().AppendChild(_name));
  }
  SceneChangedNotice().Send();
}

//==================================================================================
// Selection
//==================================================================================
SelectCommand::SelectCommand(Selection::Type type, 
  const pxr::SdfPathVector& paths, int mode)
  : Command(true)
  , _paths(paths)
  , _type(type)
  , _mode(mode)
{
}

void SelectCommand::Execute()
{
  Selection* selection = GetApplication()->GetSelection();
  _previous = selection->GetItems();
  Redo();
}

void SelectCommand::Undo()
{
  Selection* selection = GetApplication()->GetSelection();
  selection->SetItems(_previous);
  SelectionChangedNotice().Send();
}

void SelectCommand::Redo() {
  Selection* selection = GetApplication()->GetSelection();
  switch (_mode) {
  case 0:
    selection->Clear();
    for (auto& path : _paths) selection->AddItem(path);
    break;
  case 1:
    for (auto& path : _paths) selection->AddItem(path);
    break;
  case 2:
    for (auto& path : _paths) selection->RemoveItem(path);
    break;
  case 3:
    for (auto& path : _paths) selection->ToggleItem(path);
    break;
  }
  SelectionChangedNotice().Send();
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
  std::cout << "TRANSLATE COMMAND : " << targets.size() << std::endl;
  _time = timeCode;
  for (const auto& target: targets) {
    std::cout << target.path << std::endl;
    _origin.push_back(target.previous.translation);
    _translate.push_back(target.current.translation);
    _prims.push_back(stage->GetPrimAtPath(target.path));
  }
}

void TranslateCommand::Execute()
{
  Redo();
}

void TranslateCommand::Undo()
{
  for (size_t i = 0; i < _prims.size(); ++i) {
    pxr::UsdGeomXformCommonAPI api(_prims[i]);
    api.SetTranslate(_origin[i], _time);
  }
  SceneChangedNotice().Send();
}

void TranslateCommand::Redo() {
  for (size_t i = 0; i < _prims.size(); ++i) {
    pxr::UsdGeomXformCommonAPI api(_prims[i]);
    if (!api) {
      _EnsureXformCommonAPI(_prims[i], _time);
    }
    api.SetTranslate(_translate[i], _time);
  }
  SceneChangedNotice().Send();
}

//==================================================================================
// Rotate
//==================================================================================
RotateCommand::RotateCommand(pxr::UsdStageRefPtr stage, 
  const HandleTargetDescList& targets, pxr::UsdTimeCode& timeCode)
  : Command(true)
{
  _time = timeCode;
  for (const auto& target: targets) {
    pxr::UsdGeomXformable xformable(stage->GetPrimAtPath(target.path));

    _origin.push_back(target.previous.rotation);
    _rotation.push_back(target.current.rotation);
    _rotOrder.push_back(target.previous.rotOrder);
    _prims.push_back(xformable.GetPrim()); 
  }
}

void RotateCommand::Execute()
{
  Redo();
}


void RotateCommand::Undo()
{
  for (size_t i = 0; i < _prims.size(); ++i) {
    pxr::UsdGeomXformCommonAPI api(_prims[i]);
    api.SetRotate(_origin[i], _rotOrder[i], _time);
  }
  SceneChangedNotice().Send();
}

void RotateCommand::Redo() {
  for (size_t i = 0; i < _prims.size(); ++i) {
    pxr::UsdGeomXformCommonAPI api(_prims[i]);
    if (!api) {
      _EnsureXformCommonAPI(_prims[i], _time);
    }
    api.SetRotate(_rotation[i],_rotOrder[i], _time);
  }
  SceneChangedNotice().Send();
}

//==================================================================================
// Scale
//==================================================================================
ScaleCommand::ScaleCommand(pxr::UsdStageRefPtr stage, 
  const HandleTargetDescList& targets, pxr::UsdTimeCode& timeCode)
  : Command(true)
{
  _time = timeCode;
  //pxr::UsdGeomXformCache xformCache(timeCode);
  for (auto& target : targets) {
    /*
    pxr::UsdGeomXformable xformable(stage->GetPrimAtPath(target.path));
    pxr::GfMatrix4f invParentMatrix(
      xformCache.GetParentToWorldTransform(xformable.GetPrim()).GetInverse());
    pxr::GfMatrix4d xformMatrix((target.offset * matrix) * invParentMatrix);
    pxr::GfVec3f(xformMatrix[0][0], xformMatrix[1][1], xformMatrix[2][2])
    */
    _origin.push_back(target.previous.scale);
    _scale.push_back(target.current.scale);
    _prims.push_back(stage->GetPrimAtPath(target.path));
  }
}

void ScaleCommand::Execute()
{
  Redo();
}

void ScaleCommand::Undo()
{
  for (size_t i = 0; i < _prims.size(); ++i) {
    pxr::UsdGeomXformCommonAPI api(_prims[i]);
    api.SetScale(_origin[i], _time);
  }
  SceneChangedNotice().Send();
}

void ScaleCommand::Redo() {
  for (size_t i = 0; i < _prims.size(); ++i) {
    pxr::UsdGeomXformCommonAPI api(_prims[i]);
    if (!api) {
      _EnsureXformCommonAPI(_prims[i], _time);
    }
    api.SetScale(_scale[i], _time);
  }
  SceneChangedNotice().Send();
}

//==================================================================================
// Pivot
//==================================================================================
PivotCommand::PivotCommand(pxr::UsdStageRefPtr stage, 
  const HandleTargetDescList& targets, pxr::UsdTimeCode& timeCode)
  : Command(true)
{
  _time = timeCode;
  pxr::UsdGeomXformCache xformCache(timeCode);
  for (auto& target : targets) {
    /*
    pxr::UsdGeomXformable xformable(stage->GetPrimAtPath(target.path));
    pxr::GfMatrix4f invParentMatrix(
      xformCache.GetParentToWorldTransform(xformable.GetPrim()).GetInverse());
    pxr::GfMatrix4d xformMatrix((target.offset * matrix) * invParentMatrix);
    pxr::GfVec3f(xformMatrix.GetRow3(3))
    */
    _origin.push_back(target.previous.pivot);
    _pivot.push_back(target.current.pivot);
    _prims.push_back(stage->GetPrimAtPath(target.path));
  }
}

void PivotCommand::Execute()
{
  Redo();
}

void PivotCommand::Undo()
{
  for (size_t i = 0; i < _prims.size(); ++i) {
    pxr::UsdGeomXformCommonAPI api(_prims[i]);
    api.SetPivot(_origin[i], _time);
  }
  SceneChangedNotice().Send();
}

void PivotCommand::Redo() {
  for (size_t i = 0; i < _prims.size(); ++i) {
    pxr::UsdGeomXformCommonAPI api(_prims[i]);
    if (!api) {
      _EnsureXformCommonAPI(_prims[i], _time);
    }
    api.SetPivot(_pivot[i], _time);
  }
  SceneChangedNotice().Send();
}

PXR_NAMESPACE_CLOSE_SCOPE